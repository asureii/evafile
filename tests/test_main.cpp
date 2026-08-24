#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include "../include/FileOperations.hpp"
#include "../include/Config.hpp"
#include "../include/ThemeEngine.hpp"

class TestEvaFile : public QObject {
    Q_OBJECT

private slots:
    void testThemeColors() {
        const Theme& theme = ThemeEngine::instance().currentTheme();
        QCOMPARE(theme.accent_primary.name(), QString("#cf2824"));
        QCOMPARE(theme.accent_bright.name(), QString("#e32a10"));
        QCOMPARE(theme.bg_base.name(), QString("#25090a"));
        QCOMPARE(theme.bg_surface.name(), QString("#360e10"));

        QString qss = ThemeEngine::instance().generateStyleSheet();
        QVERIFY(qss.contains("#cf2824"));
        QVERIFY(qss.contains("#e32a10"));
        QVERIFY(qss.contains("#25090a"));
    }

    void testAutoRenamePath() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString file1 = tempDir.path() + "/document.txt";
        QFile f1(file1);
        QVERIFY(f1.open(QIODevice::WriteOnly));
        f1.close();

        // When document.txt exists, auto rename should produce document (1).txt
        QString next1 = FileOperations::autoRenamePath(file1);
        QCOMPARE(next1, tempDir.path() + "/document (1).txt");

        // Create document (1).txt
        QFile f2(next1);
        QVERIFY(f2.open(QIODevice::WriteOnly));
        f2.close();

        // Next should produce document (2).txt
        QString next2 = FileOperations::autoRenamePath(file1);
        QCOMPARE(next2, tempDir.path() + "/document (2).txt");
    }

    void testCompoundExtensionAutoRename() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString archive = tempDir.path() + "/backup.tar.gz";
        QFile f1(archive);
        QVERIFY(f1.open(QIODevice::WriteOnly));
        f1.close();

        QString next = FileOperations::autoRenamePath(archive);
        QCOMPARE(next, tempDir.path() + "/backup (1).tar.gz");
    }

    void testCreateFileAndDir() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        bool dirOk = FileOperations::instance().createDirectory(tempDir.path(), "test_folder");
        QVERIFY(dirOk);
        QVERIFY(QDir(tempDir.path() + "/test_folder").exists());

        bool fileOk = FileOperations::instance().createFile(tempDir.path() + "/test_folder", "notes.md");
        QVERIFY(fileOk);
        QVERIFY(QFile::exists(tempDir.path() + "/test_folder/notes.md"));
    }

    void testConfigBookmarks() {
        auto& conf = ConfigManager::instance();
        conf.addBookmark("/tmp/test_bookmark_123");
        QVERIFY(conf.config().customBookmarks.contains("/tmp/test_bookmark_123"));

        conf.removeBookmark("/tmp/test_bookmark_123");
        QVERIFY(!conf.config().customBookmarks.contains("/tmp/test_bookmark_123"));
    }
};

QTEST_MAIN(TestEvaFile)
#include "test_main.moc"
