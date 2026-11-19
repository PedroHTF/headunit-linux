#include "media/MediaLibrary.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

static void touch(const QString &path)
{
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
}

class TestMediaLibrary : public QObject
{
    Q_OBJECT
private slots:
    void scansAudioAndVideo()
    {
        QTemporaryDir dir;
        QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("Banda X")));
        touch(dir.filePath(QStringLiteral("Banda X/b musica.mp3")));
        touch(dir.filePath(QStringLiteral("a musica.FLAC")));
        touch(dir.filePath(QStringLiteral("filme.mp4")));
        touch(dir.filePath(QStringLiteral("notas.txt")));

        MediaLibrary lib;
        lib.setRootPath(dir.path());
        QCOMPARE(lib.allItems().size(), 3);
        QCOMPARE(lib.count(), 2); // aba Músicas
        QCOMPARE(lib.visibleItems().at(0).title, QStringLiteral("a musica"));
        QCOMPARE(lib.visibleItems().at(1).subtitle, QStringLiteral("Banda X"));

        lib.setShowVideos(true);
        QCOMPARE(lib.count(), 1);
        QVERIFY(lib.visibleItems().at(0).isVideo);
    }

    void unmountHidesEverything()
    {
        QTemporaryDir dir;
        touch(dir.filePath(QStringLiteral("x.mp3")));
        MediaLibrary lib;
        lib.setRootPath(dir.path());
        QCOMPARE(lib.count(), 1);
        lib.setMounted(false);
        QCOMPARE(lib.count(), 0);
        lib.setMounted(true);
        QCOMPARE(lib.count(), 1);
    }

    void detectsNewFiles()
    {
        QTemporaryDir dir;
        MediaLibrary lib;
        lib.setRootPath(dir.path());
        QCOMPARE(lib.count(), 0);
        touch(dir.filePath(QStringLiteral("nova.mp3")));
        QTRY_COMPARE(lib.count(), 1);
    }
};

QTEST_GUILESS_MAIN(TestMediaLibrary)
#include "tst_media_library.moc"
