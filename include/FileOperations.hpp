#pragma once

#include <QObject>
#include <QStringList>
#include <QThread>
#include <functional>

enum class ConflictAction {
    Ask,
    Overwrite,
    AutoRename,
    Skip,
    Cancel
};

struct OperationProgress {
    QString currentFile;
    qint64 bytesDone = 0;
    qint64 bytesTotal = 0;
    int filesDone = 0;
    int filesTotal = 0;
    double speedMBs = 0.0;
    int percent = 0;
    bool isCompleted = false;
    bool hasError = false;
    QString errorMessage;
};

class FileOperationWorker : public QObject {
    Q_OBJECT
public:
    enum class OpType { Copy, Move, Trash, Delete, ExtractZip, CompressZip };

    FileOperationWorker(OpType type, const QStringList& sources, const QString& destination);

    void cancel() { m_isCancelled = true; }

signals:
    void progress(const OperationProgress& p);
    void conflictOccurred(const QString& source, const QString& dest, ConflictAction& resolution);
    void finished(bool success, const QString& message);

public slots:
    void run();

private:
    bool copyRecursive(const QString& src, const QString& dst);
    bool moveRecursive(const QString& src, const QString& dst);
    bool trashFile(const QString& path);
    bool deleteRecursive(const QString& path);
    bool extractZipArchive(const QString& zipFile, const QString& destDir);
    bool compressZipArchive(const QStringList& sources, const QString& zipFile);

    QString generateAutoRename(const QString& targetPath);

    OpType m_type;
    QStringList m_sources;
    QString m_destination;
    bool m_isCancelled = false;
    OperationProgress m_progress;
    ConflictAction m_globalConflictAction = ConflictAction::Ask;
};

class FileOperations : public QObject {
    Q_OBJECT
public:
    static FileOperations& instance();

    void copy(const QStringList& sources, const QString& destination);
    void move(const QStringList& sources, const QString& destination);
    void trash(const QStringList& paths);
    void permanentDelete(const QStringList& paths);
    void extractZip(const QString& zipPath, const QString& destination);
    void compressToZip(const QStringList& sources, const QString& destinationZip);
    bool createDirectory(const QString& parentPath, const QString& name);
    bool createFile(const QString& parentPath, const QString& name);
    bool rename(const QString& oldPath, const QString& newName);
    static QString autoRenamePath(const QString& targetPath);

signals:
    void operationStarted(const QString& desc);
    void operationProgress(const OperationProgress& p);
    void operationFinished(bool success, const QString& message);

private:
    FileOperations();
    void startWorker(FileOperationWorker* worker);
};
