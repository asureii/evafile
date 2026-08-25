#include "FileOperations.hpp"
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QTextStream>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QProcess>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

FileOperationWorker::FileOperationWorker(OpType type, const QStringList& sources, const QString& destination)
    : m_type(type), m_sources(sources), m_destination(destination)
{
}

QString FileOperationWorker::generateAutoRename(const QString& targetPath) {
    return FileOperations::autoRenamePath(targetPath);
}

bool FileOperationWorker::copyRecursive(const QString& src, const QString& dst) {
    if (m_isCancelled) return false;

    QFileInfo srcInfo(src);
    if (!srcInfo.exists()) return false;

    if (srcInfo.isDir()) {
        QDir().mkpath(dst);
        QDir dir(src);
        const auto entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden);
        for (const auto& entry : entries) {
            if (m_isCancelled) return false;
            QString subSrc = entry.absoluteFilePath();
            QString subDst = dst + "/" + entry.fileName();
            if (!copyRecursive(subSrc, subDst)) return false;
        }
        return true;
    } else {
        QString actualDst = dst;
        if (QFile::exists(actualDst)) {
            if (m_globalConflictAction == ConflictAction::AutoRename) {
                actualDst = generateAutoRename(actualDst);
            } else if (m_globalConflictAction == ConflictAction::Skip) {
                return true;
            } else if (m_globalConflictAction == ConflictAction::Ask) {
                actualDst = generateAutoRename(actualDst); // Default safe auto-rename
            }
        }

        m_progress.currentFile = srcInfo.fileName();
        emit progress(m_progress);

        QFile::remove(actualDst);
        bool ok = QFile::copy(src, actualDst);
        if (ok) {
            m_progress.bytesDone += srcInfo.size();
            m_progress.filesDone++;
            if (m_progress.filesTotal > 0) {
                m_progress.percent = static_cast<int>((m_progress.filesDone * 100) / m_progress.filesTotal);
            }
            emit progress(m_progress);
        }
        return ok;
    }
}

bool FileOperationWorker::moveRecursive(const QString& src, const QString& dst) {
    if (m_isCancelled) return false;

    QFileInfo srcInfo(src);
    if (!srcInfo.exists()) return false;

    QString actualDst = dst;
    if (QFile::exists(actualDst)) {
        if (m_globalConflictAction == ConflictAction::AutoRename || m_globalConflictAction == ConflictAction::Ask) {
            actualDst = generateAutoRename(actualDst);
        } else if (m_globalConflictAction == ConflictAction::Skip) {
            return true;
        }
    }

    std::error_code ec;
    fs::rename(src.toStdString(), actualDst.toStdString(), ec);
    if (!ec) {
        m_progress.filesDone++;
        emit progress(m_progress);
        return true;
    }

    // If cross-device move fails (EXDEV), fallback to copy + delete
    if (copyRecursive(src, actualDst)) {
        deleteRecursive(src);
        return true;
    }

    return false;
}

bool FileOperationWorker::trashFile(const QString& path) {
    QFileInfo fi(path);
    if (!fi.exists()) return false;

    QString trashDir = QDir::homePath() + "/.local/share/Trash";
    QString filesDir = trashDir + "/files";
    QString infoDir = trashDir + "/info";

    QDir().mkpath(filesDir);
    QDir().mkpath(infoDir);

    QString baseName = fi.fileName();
    QString targetFile = filesDir + "/" + baseName;
    QString targetInfo = infoDir + "/" + baseName + ".trashinfo";

    if (QFile::exists(targetFile)) {
        targetFile = generateAutoRename(targetFile);
        QFileInfo targetFi(targetFile);
        targetInfo = infoDir + "/" + targetFi.fileName() + ".trashinfo";
    }

    // Write .trashinfo
    QFile infoFile(targetInfo);
    if (infoFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&infoFile);
        out << "[Trash Info]\n";
        out << "Path=" << fi.absoluteFilePath() << "\n";
        out << "DeletionDate=" << QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss") << "\n";
        infoFile.close();
    }

    std::error_code ec;
    fs::rename(path.toStdString(), targetFile.toStdString(), ec);
    if (ec) {
        // Fallback
        if (copyRecursive(path, targetFile)) {
            deleteRecursive(path);
            return true;
        }
        return false;
    }
    return true;
}

bool FileOperationWorker::deleteRecursive(const QString& path) {
    QFileInfo fi(path);
    if (!fi.exists()) return true;

    if (fi.isDir()) {
        QDir dir(path);
        return dir.removeRecursively();
    } else {
        return QFile::remove(path);
    }
}

bool FileOperationWorker::extractZipArchive(const QString& zipFile, const QString& destDir) {
    if (m_isCancelled) return false;
    QFileInfo fi(zipFile);
    if (!fi.exists()) return false;

    QDir().mkpath(destDir);

    m_progress.currentFile = fi.fileName();
    m_progress.percent = 25;
    emit progress(m_progress);

    // Try unzip
    QProcess proc;
    proc.start("unzip", {"-q", "-o", zipFile, "-d", destDir});
    if (proc.waitForStarted(2000) && proc.waitForFinished(120000) && proc.exitCode() == 0) {
        m_progress.percent = 100;
        m_progress.filesDone = 1;
        emit progress(m_progress);
        return true;
    }

    // Fallback 1: bsdtar
    QProcess bsdtarProc;
    bsdtarProc.start("bsdtar", {"-xf", zipFile, "-C", destDir});
    if (bsdtarProc.waitForStarted(2000) && bsdtarProc.waitForFinished(120000) && bsdtarProc.exitCode() == 0) {
        m_progress.percent = 100;
        m_progress.filesDone = 1;
        emit progress(m_progress);
        return true;
    }

    // Fallback 2: python zipfile module
    QProcess pyProc;
    pyProc.start("python3", {"-m", "zipfile", "-e", zipFile, destDir});
    if (pyProc.waitForStarted(2000) && pyProc.waitForFinished(120000) && pyProc.exitCode() == 0) {
        m_progress.percent = 100;
        m_progress.filesDone = 1;
        emit progress(m_progress);
        return true;
    }

    return false;
}

bool FileOperationWorker::compressZipArchive(const QStringList& sources, const QString& zipFile) {
    if (m_isCancelled || sources.isEmpty()) return false;

    m_progress.currentFile = QFileInfo(zipFile).fileName();
    m_progress.percent = 25;
    emit progress(m_progress);

    if (QFile::exists(zipFile)) {
        QFile::remove(zipFile);
    }

    QFileInfo firstFi(sources.first());
    QString workingDir = firstFi.absolutePath();

    // Try zip command
    QStringList zipArgs;
    zipArgs << "-r" << "-q" << zipFile;
    for (const QString& src : sources) {
        QFileInfo fi(src);
        if (fi.absolutePath() == workingDir) {
            zipArgs << fi.fileName();
        } else {
            zipArgs << src;
        }
    }

    QProcess proc;
    proc.setWorkingDirectory(workingDir);
    proc.start("zip", zipArgs);
    if (proc.waitForStarted(2000) && proc.waitForFinished(120000) && proc.exitCode() == 0) {
        m_progress.percent = 100;
        m_progress.filesDone = sources.size();
        emit progress(m_progress);
        return true;
    }

    // Fallback 1: bsdtar
    QProcess bsdtarProc;
    bsdtarProc.setWorkingDirectory(workingDir);
    QStringList tarArgs;
    tarArgs << "-a" << "-cf" << zipFile;
    for (const QString& src : sources) {
        tarArgs << QFileInfo(src).fileName();
    }
    bsdtarProc.start("bsdtar", tarArgs);
    if (bsdtarProc.waitForStarted(2000) && bsdtarProc.waitForFinished(120000) && bsdtarProc.exitCode() == 0) {
        m_progress.percent = 100;
        m_progress.filesDone = sources.size();
        emit progress(m_progress);
        return true;
    }

    // Fallback 2: python zipfile module
    QString pyScript =
        "import zipfile, os, sys\n"
        "zip_path = sys.argv[1]\n"
        "sources = sys.argv[2:]\n"
        "with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zf:\n"
        "    for src in sources:\n"
        "        if os.path.isdir(src):\n"
        "            for root, _, files in os.walk(src):\n"
        "                for f in files:\n"
        "                    p = os.path.join(root, f)\n"
        "                    zf.write(p, os.path.relpath(p, os.path.dirname(src)))\n"
        "        elif os.path.isfile(src):\n"
        "            zf.write(src, os.path.basename(src))\n";
    QProcess pyProc;
    QStringList pyArgs;
    pyArgs << "-c" << pyScript << zipFile;
    for (const QString& src : sources) pyArgs << src;
    pyProc.start("python3", pyArgs);
    if (pyProc.waitForStarted(2000) && pyProc.waitForFinished(120000) && pyProc.exitCode() == 0) {
        m_progress.percent = 100;
        m_progress.filesDone = sources.size();
        emit progress(m_progress);
        return true;
    }

    return false;
}

void FileOperationWorker::run() {
    m_progress.filesTotal = m_sources.size();
    m_progress.filesDone = 0;
    m_progress.percent = 0;

    bool allSuccess = true;

    if (m_type == OpType::ExtractZip) {
        if (!m_sources.isEmpty()) {
            allSuccess = extractZipArchive(m_sources.first(), m_destination);
        }
    } else if (m_type == OpType::CompressZip) {
        allSuccess = compressZipArchive(m_sources, m_destination);
    } else {
        for (const QString& src : m_sources) {
            if (m_isCancelled) break;

            QFileInfo fi(src);
            QString destPath = m_destination.isEmpty() ? QString() : (m_destination + "/" + fi.fileName());

            switch (m_type) {
                case OpType::Copy:
                    if (!copyRecursive(src, destPath)) allSuccess = false;
                    break;
                case OpType::Move:
                    if (!moveRecursive(src, destPath)) allSuccess = false;
                    break;
                case OpType::Trash:
                    if (!trashFile(src)) allSuccess = false;
                    break;
                case OpType::Delete:
                    if (!deleteRecursive(src)) allSuccess = false;
                    break;
                default:
                    break;
            }
        }
    }

    m_progress.isCompleted = true;
    emit progress(m_progress);
    emit finished(allSuccess, allSuccess ? "Operation completed successfully" : "Some items could not be processed");
}

FileOperations& FileOperations::instance() {
    static FileOperations s_instance;
    return s_instance;
}

FileOperations::FileOperations() {}

QString FileOperations::autoRenamePath(const QString& targetPath) {
    QFileInfo fi(targetPath);
    QString dir = fi.absolutePath();
    QString fullName = fi.fileName();

    // Check for compound extension e.g. tar.gz
    QString baseName = fi.completeBaseName();
    QString suffix = fi.suffix();

    if (fullName.endsWith(".tar.gz") || fullName.endsWith(".tar.xz") || fullName.endsWith(".tar.bz2") || fullName.endsWith(".tar.zst")) {
        int dot1 = fullName.indexOf('.');
        baseName = fullName.left(dot1);
        suffix = fullName.mid(dot1 + 1);
    }

    int counter = 1;
    QString candidate;
    do {
        if (suffix.isEmpty()) {
            candidate = QString("%1/%2 (%3)").arg(dir).arg(baseName).arg(counter);
        } else {
            candidate = QString("%1/%2 (%3).%4").arg(dir).arg(baseName).arg(counter).arg(suffix);
        }
        counter++;
    } while (QFile::exists(candidate));

    return candidate;
}

void FileOperations::startWorker(FileOperationWorker* worker) {
    QThread* thread = new QThread();
    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &FileOperationWorker::run);
    connect(worker, &FileOperationWorker::progress, this, &FileOperations::operationProgress);
    connect(worker, &FileOperationWorker::finished, this, &FileOperations::operationFinished);
    connect(worker, &FileOperationWorker::finished, thread, &QThread::quit);
    connect(worker, &FileOperationWorker::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
}

void FileOperations::copy(const QStringList& sources, const QString& destination) {
    emit operationStarted(QString("Copying %1 items...").arg(sources.size()));
    FileOperationWorker* worker = new FileOperationWorker(FileOperationWorker::OpType::Copy, sources, destination);
    startWorker(worker);
}

void FileOperations::move(const QStringList& sources, const QString& destination) {
    emit operationStarted(QString("Moving %1 items...").arg(sources.size()));
    FileOperationWorker* worker = new FileOperationWorker(FileOperationWorker::OpType::Move, sources, destination);
    startWorker(worker);
}

void FileOperations::trash(const QStringList& paths) {
    emit operationStarted(QString("Moving %1 items to Trash...").arg(paths.size()));
    FileOperationWorker* worker = new FileOperationWorker(FileOperationWorker::OpType::Trash, paths, QString());
    startWorker(worker);
}

void FileOperations::permanentDelete(const QStringList& paths) {
    emit operationStarted(QString("Permanently deleting %1 items...").arg(paths.size()));
    FileOperationWorker* worker = new FileOperationWorker(FileOperationWorker::OpType::Delete, paths, QString());
    startWorker(worker);
}

void FileOperations::extractZip(const QString& zipPath, const QString& destination) {
    QFileInfo fi(zipPath);
    emit operationStarted(QString("Extracting %1...").arg(fi.fileName()));
    FileOperationWorker* worker = new FileOperationWorker(FileOperationWorker::OpType::ExtractZip, {zipPath}, destination);
    startWorker(worker);
}

void FileOperations::compressToZip(const QStringList& sources, const QString& destinationZip) {
    QFileInfo fi(destinationZip);
    emit operationStarted(QString("Compressing to %1...").arg(fi.fileName()));
    FileOperationWorker* worker = new FileOperationWorker(FileOperationWorker::OpType::CompressZip, sources, destinationZip);
    startWorker(worker);
}

bool FileOperations::createDirectory(const QString& parentPath, const QString& name) {
    QDir dir(parentPath);
    return dir.mkdir(name);
}

bool FileOperations::createFile(const QString& parentPath, const QString& name) {
    return createFileWithContent(parentPath, name, QString());
}

bool FileOperations::createFileWithContent(const QString& parentPath, const QString& name, const QString& content) {
    QString fullPath = parentPath + "/" + name;
    QFile file(fullPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (!content.isEmpty()) {
            QTextStream out(&file);
            out << content;
        }
        file.close();
        return true;
    }
    return false;
}

bool FileOperations::rename(const QString& oldPath, const QString& newName) {
    QFileInfo fi(oldPath);
    QString newPath = fi.absolutePath() + "/" + newName;
    return QFile::rename(oldPath, newPath);
}
