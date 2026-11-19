#include "logo/LogoManager.h"
#include "media/FolderTree.h"
#include "media/MediaLibrary.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

static void touch(const QString &path)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("x");
}

class TestLogoFolders : public QObject
{
    Q_OBJECT
private slots:
    void folderTree()
    {
        const QStringList folders{"Musicas/Rock", "Musicas/Rock/Anos 80", "Musicas/MPB", "Videos", ""};
        QCOMPARE(FolderTree::subfolders(folders, ""), (QStringList{"Musicas", "Videos"}));
        QCOMPARE(FolderTree::subfolders(folders, "/Musicas/"), (QStringList{"MPB", "Rock"}));
        QCOMPARE(FolderTree::parent("Musicas/Rock"), QStringLiteral("Musicas"));
        QVERIFY(!FolderTree::hasParent(""));
        QCOMPARE(FolderTree::countUnder(folders, "Musicas"), 3);
    }

    void libraryFoldersAndSystemFolderHidden()
    {
        QTemporaryDir dir;
        touch(dir.filePath("Musicas/Rock/a.mp3"));
        touch(dir.filePath("Musicas/b.mp3"));
        touch(dir.filePath("raiz.mp4"));
        touch(dir.filePath("HeadUnit/logo.mp4"));
        MediaLibrary lib;
        lib.setRootPath(dir.path());
        QCOMPARE(lib.allItems().size(), 3); // logo.mp4 fica de fora

        const QVariantList root = lib.folderEntries("");
        QCOMPARE(root.size(), 2); // pasta Musicas + raiz.mp4
        QCOMPARE(root[0].toMap()["kind"].toString(), QStringLiteral("folder"));
        QCOMPARE(root[0].toMap()["count"].toInt(), 2);
        QCOMPARE(lib.filesIn("Musicas/Rock").size(), 1);
    }

    void logoImportAndDefault()
    {
        QTemporaryDir usb, data;
        touch(usb.filePath("HeadUnit/logo.png"));
        touch(usb.filePath("Outra/logo.png")); // fora da pasta HeadUnit: ignorado
        QSettings storage(data.filePath("s.ini"), QSettings::IniFormat);
        LogoManager logo(data.filePath("app"), &storage);
        QCOMPARE(logo.kind(), QStringLiteral("default"));

        const QVariantList found = logo.findCandidates(usb.path());
        QCOMPARE(found.size(), 1);
        QVERIFY(logo.importFile(found[0].toMap()["path"].toString()));
        QCOMPARE(logo.kind(), QStringLiteral("image"));
        QVERIFY(QFile::exists(logo.fileUrl().toLocalFile()));

        LogoManager reloaded(data.filePath("app"), &storage);
        QCOMPARE(reloaded.kind(), QStringLiteral("image"));

        logo.useDefault();
        QCOMPARE(logo.kind(), QStringLiteral("default"));
    }
};

QTEST_GUILESS_MAIN(TestLogoFolders)
#include "tst_logo_folders.moc"
