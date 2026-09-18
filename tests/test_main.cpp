#include <QTest>
#include <QTemporaryDir>
#include <QFile>
#include <QMimeData>
#include "../include/FileOperations.hpp"
#include "../include/Config.hpp"
#include "../include/ThemeEngine.hpp"
#include "../include/EvalinkManager.hpp"
#include "../include/DragDropHelper.hpp"

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

    void testZipCompressionAndExtraction() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString sample = tempDir.path() + "/hello.txt";
        QFile f(sample);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("Hello EvaFile ZIP compatibility!");
        f.close();

        QString targetZip = tempDir.path() + "/test.zip";
        FileOperationWorker compressWorker(FileOperationWorker::OpType::CompressZip, {sample}, targetZip);
        compressWorker.run();

        QVERIFY(QFile::exists(targetZip));
        QVERIFY(QFileInfo(targetZip).size() > 0);

        QString extractDir = tempDir.path() + "/extracted";
        FileOperationWorker extractWorker(FileOperationWorker::OpType::ExtractZip, {targetZip}, extractDir);
        extractWorker.run();

        QVERIFY(QDir(extractDir).exists());
        QVERIFY(QFile::exists(extractDir + "/hello.txt"));

        QFile extractedFile(extractDir + "/hello.txt");
        QVERIFY(extractedFile.open(QIODevice::ReadOnly));
        QCOMPARE(QString::fromUtf8(extractedFile.readAll()), QString("Hello EvaFile ZIP compatibility!"));
        extractedFile.close();
    }

    void testEvalinkManagerFormatting() {
        QCOMPARE(EvalinkManager::formatBytes(0), QString("0 B"));
        QCOMPARE(EvalinkManager::formatBytes(512), QString("512 B"));
        QCOMPARE(EvalinkManager::formatBytes(1024 * 1024), QString("1.0 MB"));
        QCOMPARE(EvalinkManager::formatBytes(1024ULL * 1024 * 1024 * 2), QString("2.00 GB"));

        QCOMPARE(EvalinkManager::formatSpeed(0), QString("0 B/s"));
        QCOMPARE(EvalinkManager::formatSpeed(1024 * 1024 * 15), QString("15.0 MB/s"));
    }

    void testDragDropMimeData() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString file1 = tempDir.path() + "/sample.txt";
        QFile f(file1);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("drag drop test");
        f.close();

        QMimeData mime;
        QList<QUrl> urls;
        urls.append(QUrl::fromLocalFile(file1));
        mime.setUrls(urls);

        QVERIFY(mime.hasUrls());
        QCOMPARE(mime.urls().size(), 1);
        QCOMPARE(mime.urls().first().toLocalFile(), file1);
    }

    void testMoveAndCopyOperations() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString subDir = tempDir.path() + "/target_folder";
        QDir().mkpath(subDir);

        QString srcFile = tempDir.path() + "/test_move.txt";
        QFile f(srcFile);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("hello move");
        f.close();

        // Test FileOperationWorker directly
        FileOperationWorker moveWorker(FileOperationWorker::OpType::Move, {srcFile}, subDir);
        moveWorker.run();

        QVERIFY(!QFile::exists(srcFile));
        QVERIFY(QFile::exists(subDir + "/test_move.txt"));
    }
};

QTEST_MAIN(TestEvaFile)
#include "test_main.moc"
