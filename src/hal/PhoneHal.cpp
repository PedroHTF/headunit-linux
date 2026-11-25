#include "hal/PhoneHal.h"

#include <QVariantMap>

static QString digitsOnly(const QString &s)
{
    QString out;
    for (QChar c : s)
        if (c.isDigit()) out += c;
    return out;
}

QString PhoneHal::nameFor(const QString &number) const
{
    const QString n = digitsOnly(number);
    for (const Contact &c : m_contacts)
        if (!n.isEmpty() && digitsOnly(c.number).endsWith(n.right(8))) return c.name;
    return number;
}

QVariantList PhoneHal::contactsVariant() const
{
    QVariantList list;
    for (const Contact &c : m_contacts)
        list << QVariantMap{{"name", c.name}, {"number", c.number}};
    return list;
}

QVariantList PhoneHal::recentsVariant() const
{
    QVariantList list;
    for (const RecentCall &r : m_recents) {
        list << QVariantMap{{"name", r.name}, {"number", r.number}, {"kind", r.kind},
                            {"when", r.when.toString(QStringLiteral("dd/MM HH:mm"))}};
    }
    return list;
}
