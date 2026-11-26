#include "hal/VCard.h"

#include <QtTest>

class TestVCard : public QObject
{
    Q_OBJECT
private slots:
    void vcard30WithSeveralNumbers()
    {
        const QByteArray data =
            "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:\r\nN:;;;;\r\nEND:VCARD\r\n" // vCard 0 do PBAP (o próprio celular), vazio
            "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Bruno Lima\r\nN:Lima;Bruno;;;\r\n"
            "TEL;TYPE=CELL,VOICE:+55 11 98765-4321\r\nTEL;TYPE=WORK:+55 11 2345-6789\r\nEND:VCARD\r\n"
            "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Ana Souza\r\nTEL:+55 11 91234-5678\r\nEND:VCARD\r\n";
        const QVector<Contact> c = VCard::parseContacts(data);
        QCOMPARE(c.size(), 3);
        QCOMPARE(c[0].name, QStringLiteral("Ana Souza")); // em ordem alfabética
        QCOMPARE(c[1].name, QStringLiteral("Bruno Lima"));
        QCOMPARE(c[1].number, QStringLiteral("+55 11 98765-4321"));
        QCOMPARE(c[2].number, QStringLiteral("+55 11 2345-6789"));
    }

    void vcard21QuotedPrintableAndFolding()
    {
        // Android em vCard 2.1: acentos em QUOTED-PRINTABLE (UTF-8), com quebra suave "=".
        const QByteArray data =
            "BEGIN:VCARD\nVERSION:2.1\n"
            "N;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:Concei=C3=A7=C3=A3o;Jo=C3=A3=\n"
            "o;;;\n"
            "TEL;CELL:11999990000\nEND:VCARD\n"
            "BEGIN:VCARD\nVERSION:3.0\nFN:Oficina do\n  Zé\nTEL:4002-8922\nEND:VCARD\n";
        const QVector<Contact> c = VCard::parseContacts(data);
        QCOMPARE(c.size(), 2);
        QCOMPARE(c[0].name, QStringLiteral("João Conceição")); // só N: "Nome Sobrenome"
        QCOMPARE(c[0].number, QStringLiteral("11999990000"));
        QCOMPARE(c[1].name, QStringLiteral("Oficina do Zé"));
    }

    void escapedCommaAndGroupedProperty()
    {
        const QByteArray data = "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Silva\\, Pedro\r\n"
                                "item1.TEL:tel:+5511955550000\r\nEND:VCARD\r\n";
        const QVector<Contact> c = VCard::parseContacts(data);
        QCOMPARE(c.size(), 1);
        QCOMPARE(c[0].name, QStringLiteral("Silva, Pedro"));
        QCOMPARE(c[0].number, QStringLiteral("+5511955550000"));
    }

    void callHistory()
    {
        const QByteArray data =
            "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Ana Souza\r\nTEL:+55 11 91234-5678\r\n"
            "X-IRMC-CALL-DATETIME;TYPE=RECEIVED:20261125T081500\r\nEND:VCARD\r\n"
            "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:\r\nTEL:+55 11 95555-0000\r\n"
            "X-IRMC-CALL-DATETIME;MISSED:20261126T101500\r\nEND:VCARD\r\n"
            "BEGIN:VCARD\r\nVERSION:2.1\r\nN:\r\nTEL:\r\n"
            "X-IRMC-CALL-DATETIME;DIALED:20261124T200000\r\nEND:VCARD\r\n"; // número privado sem nada: some
        const QVector<RecentCall> h = VCard::parseHistory(data);
        QCOMPARE(h.size(), 2);
        QCOMPARE(h[0].kind, QStringLiteral("missed")); // mais recente primeiro
        QCOMPARE(h[0].name, QStringLiteral("+55 11 95555-0000")); // sem nome: mostra o número
        QCOMPARE(h[0].when, QDateTime(QDate(2026, 11, 26), QTime(10, 15)));
        QCOMPARE(h[1].kind, QStringLiteral("in"));
        QCOMPARE(h[1].name, QStringLiteral("Ana Souza"));
    }

    void historyDefaultKind()
    {
        const QByteArray data = "BEGIN:VCARD\r\nVERSION:3.0\r\nFN:Casa\r\nTEL:3456-7890\r\nEND:VCARD\r\n";
        QCOMPARE(VCard::parseHistory(data).size(), 0);
        const QVector<RecentCall> h = VCard::parseHistory(data, QStringLiteral("out"));
        QCOMPARE(h.size(), 1);
        QCOMPARE(h[0].kind, QStringLiteral("out"));
    }

    void garbageIsIgnored()
    {
        QCOMPARE(VCard::parseContacts("isto não é um vcard\n:::\nBEGIN:VCARD\nFN:Sem fim").size(), 0);
        QCOMPARE(VCard::parseContacts(QByteArray()).size(), 0);
    }
};

QTEST_GUILESS_MAIN(TestVCard)
#include "tst_vcard.moc"
