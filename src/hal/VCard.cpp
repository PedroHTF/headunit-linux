#include "hal/VCard.h"

#include <QByteArrayList>
#include <QTimeZone>
#include <algorithm>

namespace {

struct Property
{
    QByteArray name;         // em maiúsculas, sem grupo ("item1.TEL" -> "TEL")
    QByteArrayList params;   // em maiúsculas ("TYPE=CELL", "ENCODING=QUOTED-PRINTABLE", "MISSED")
    QByteArray rawValue;
};

/** Desfaz a dobra de linhas (RFC 2425: CRLF + espaço/tab) e a "dobra suave" do QP (=CRLF). */
QList<QByteArray> unfold(const QByteArray &data)
{
    QByteArray text = data;
    text.replace("\r\n", "\n");
    text.replace('\r', '\n');
    QList<QByteArray> out;
    bool qpContinues = false;
    for (const QByteArray &line : text.split('\n')) {
        if (qpContinues && !out.isEmpty()) {
            out.last().chop(1); // tira o '=' final
            out.last() += line;
        } else if ((line.startsWith(' ') || line.startsWith('\t')) && !out.isEmpty()) {
            out.last() += line.mid(1);
        } else {
            out << line;
        }
        const QByteArray &cur = out.last();
        qpContinues = cur.endsWith('=') && cur.toUpper().contains("QUOTED-PRINTABLE");
    }
    return out;
}

bool parseLine(const QByteArray &line, Property &p)
{
    const int colon = line.indexOf(':');
    if (colon <= 0) return false;
    QByteArrayList head = line.left(colon).split(';');
    QByteArray name = head.takeFirst().trimmed().toUpper();
    const int dot = name.lastIndexOf('.');
    if (dot >= 0) name = name.mid(dot + 1);
    p.name = name;
    p.params.clear();
    for (const QByteArray &h : head) {
        // vCard 3.0 permite TYPE=CELL,VOICE: separa em vários.
        const QByteArray up = h.trimmed().toUpper();
        const int eq = up.indexOf('=');
        if (eq > 0 && up.contains(',')) {
            for (const QByteArray &v : up.mid(eq + 1).split(','))
                p.params << up.left(eq + 1) + v;
        } else {
            p.params << up;
        }
    }
    p.rawValue = line.mid(colon + 1);
    return true;
}

bool hasParam(const Property &p, const char *value)
{
    for (const QByteArray &x : p.params)
        if (x == value || x.endsWith(QByteArray("=") + value)) return true;
    return false;
}

QString decodeValue(const Property &p)
{
    QByteArray bytes = p.rawValue;
    if (hasParam(p, "QUOTED-PRINTABLE")) {
        QByteArray out;
        for (int i = 0; i < bytes.size(); ++i) {
            if (bytes[i] == '=' && i + 2 < bytes.size()) {
                bool ok = false;
                const int v = bytes.mid(i + 1, 2).toInt(&ok, 16);
                if (ok) { out += char(v); i += 2; continue; }
            }
            out += bytes[i];
        }
        bytes = out;
    }
    QByteArray charset = "UTF-8";
    for (const QByteArray &x : p.params)
        if (x.startsWith("CHARSET=")) charset = x.mid(8);
    if (charset == "ISO-8859-1" || charset == "LATIN1")
        return QString::fromLatin1(bytes);
    return QString::fromUtf8(bytes);
}

/** Separa por ';' respeitando "\;" e tira os escapes do vCard 3.0. */
QStringList splitComponents(const QString &value)
{
    QStringList parts;
    QString cur;
    for (int i = 0; i < value.size(); ++i) {
        const QChar c = value[i];
        if (c == u'\\' && i + 1 < value.size()) {
            const QChar n = value[++i];
            cur += (n == u'n' || n == u'N') ? QChar(u' ') : n;
        } else if (c == u';') {
            parts << cur;
            cur.clear();
        } else {
            cur += c;
        }
    }
    parts << cur;
    return parts;
}

QString unescape(const QString &value) { return splitComponents(value).join(u';'); }

/** N = Sobrenome;Nome;Meio;Prefixo;Sufixo  ->  "Prefixo Nome Meio Sobrenome Sufixo". */
QString nameFromN(const QString &value)
{
    const QStringList c = splitComponents(value);
    auto at = [&c](int i) { return i < c.size() ? c[i].trimmed() : QString(); };
    QStringList order{at(3), at(1), at(2), at(0), at(4)};
    order.removeAll(QString());
    return order.join(u' ');
}

struct Card
{
    QString fn, n;
    QStringList tels;
    QString callKind;   // "in", "out", "missed" ou vazio
    QDateTime when;
};

QVector<Card> parseCards(const QByteArray &data)
{
    QVector<Card> cards;
    Card cur;
    bool inside = false;
    Property p;
    for (const QByteArray &line : unfold(data)) {
        if (!parseLine(line, p)) continue;
        if (p.name == "BEGIN" && p.rawValue.trimmed().toUpper() == "VCARD") {
            cur = Card();
            inside = true;
        } else if (p.name == "END" && p.rawValue.trimmed().toUpper() == "VCARD") {
            if (inside) cards << cur;
            inside = false;
        } else if (!inside) {
            continue;
        } else if (p.name == "FN") {
            cur.fn = unescape(decodeValue(p)).trimmed();
        } else if (p.name == "N") {
            cur.n = nameFromN(decodeValue(p));
        } else if (p.name == "TEL") {
            QString tel = decodeValue(p).trimmed();
            if (tel.startsWith(u"tel:", Qt::CaseInsensitive)) tel = tel.mid(4);
            if (!tel.isEmpty() && !cur.tels.contains(tel)) cur.tels << tel;
        } else if (p.name == "X-IRMC-CALL-DATETIME") {
            if (hasParam(p, "MISSED")) cur.callKind = QStringLiteral("missed");
            else if (hasParam(p, "RECEIVED")) cur.callKind = QStringLiteral("in");
            else if (hasParam(p, "DIALED")) cur.callKind = QStringLiteral("out");
            const QString v = decodeValue(p).trimmed();
            // 20261126T101500 (hora local) ou 20261126T101500Z (UTC)
            QDateTime dt = QDateTime::fromString(v.left(15), QStringLiteral("yyyyMMdd'T'HHmmss"));
            if (dt.isValid() && v.endsWith(u'Z')) {
                dt = QDateTime(dt.date(), dt.time(), QTimeZone::utc()).toLocalTime();
            }
            cur.when = dt;
        }
    }
    return cards;
}

QString displayName(const Card &c)
{
    if (!c.fn.isEmpty()) return c.fn;
    if (!c.n.isEmpty()) return c.n;
    return QString();
}

} // namespace

QVector<Contact> VCard::parseContacts(const QByteArray &data)
{
    QVector<Contact> out;
    for (const Card &c : parseCards(data)) {
        const QString name = displayName(c);
        if (name.isEmpty() && c.tels.isEmpty()) continue; // o vCard 0 do PBAP às vezes vem vazio
        for (const QString &tel : c.tels)
            out << Contact{name.isEmpty() ? tel : name, tel};
    }
    std::stable_sort(out.begin(), out.end(), [](const Contact &a, const Contact &b) {
        return QString::localeAwareCompare(a.name, b.name) < 0;
    });
    return out;
}

QVector<RecentCall> VCard::parseHistory(const QByteArray &data, const QString &defaultKind)
{
    QVector<RecentCall> out;
    for (const Card &c : parseCards(data)) {
        const QString number = c.tels.value(0);
        const QString kind = c.callKind.isEmpty() ? defaultKind : c.callKind;
        if (kind.isEmpty() || (number.isEmpty() && displayName(c).isEmpty())) continue;
        const QString name = displayName(c);
        out << RecentCall{name.isEmpty() ? (number.isEmpty() ? QStringLiteral("Número privado") : number) : name,
                          number, kind, c.when};
    }
    std::stable_sort(out.begin(), out.end(), [](const RecentCall &a, const RecentCall &b) { return a.when > b.when; });
    return out;
}
