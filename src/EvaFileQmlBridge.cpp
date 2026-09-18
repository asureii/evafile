#include "EvaFileQmlBridge.hpp"
#include "Config.hpp"
#include "EvaSuiteBridge.hpp"
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <QGuiApplication>
#include <QClipboard>
#include <QCryptographicHash>
#include <QStorageInfo>
#include <QStandardPaths>

// =============================================================================
// EvaFileFolderModel Implementation
// =============================================================================

EvaFileFolderModel::EvaFileFolderModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_watcher(new QFileSystemWatcher(this))
{
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &EvaFileFolderModel::reload);
}

int EvaFileFolderModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant EvaFileFolderModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return QVariant();
    }

    const FileItemData& item = m_items.at(index.row());
    switch (role) {
        case NameRole: return item.name;
        case PathRole: return item.path;
        case IsDirRole: return item.isDir;
        case IsHiddenRole: return item.isHidden;
        case SizeRole: return item.sizeFormatted;
        case SizeBytesRole: return item.sizeBytes;
        case TypeRole: return item.type;
        case ModifiedRole: return item.modified;
        case IconNameRole: return item.iconName;
        case ExtensionRole: return item.extension;
        default: return QVariant();
    }
}

QHash<int, QByteArray> EvaFileFolderModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PathRole] = "path";
    roles[IsDirRole] = "isDir";
    roles[IsHiddenRole] = "isHidden";
    roles[SizeRole] = "sizeFormatted";
    roles[SizeBytesRole] = "sizeBytes";
    roles[TypeRole] = "type";
    roles[ModifiedRole] = "modified";
    roles[IconNameRole] = "iconName";
    roles[ExtensionRole] = "extension";
    return roles;
}

void EvaFileFolderModel::setPath(const QString& path) {
    QString clean = QDir::cleanPath(path);
    if (m_path == clean && !m_allItems.isEmpty()) return;

    if (!m_path.isEmpty() && !m_watcher->directories().isEmpty()) {
        m_watcher->removePaths(m_watcher->directories());
    }

    m_path = clean;
    if (QDir(m_path).exists()) {
        m_watcher->addPath(m_path);
    }

    updateItems();
    emit pathChanged(m_path);
}

void EvaFileFolderModel::reload() {
    updateItems();
}

QVariantMap EvaFileFolderModel::get(int index) const {
    if (index < 0 || index >= m_items.size()) return QVariantMap();
    const auto& item = m_items.at(index);
    QVariantMap map;
    map["name"] = item.name;
    map["path"] = item.path;
    map["isDir"] = item.isDir;
    map["isHidden"] = item.isHidden;
    map["sizeFormatted"] = item.sizeFormatted;
    map["sizeBytes"] = item.sizeBytes;
    map["type"] = item.type;
    map["modified"] = item.modified;
    map["iconName"] = item.iconName;
    map["extension"] = item.extension;
    return map;
}

void EvaFileFolderModel::setFilterText(const QString& text) {
    if (m_filterText == text) return;
    m_filterText = text;

    beginResetModel();
    m_items.clear();
    for (const auto& item : m_allItems) {
        if (m_filterText.isEmpty() || item.name.contains(m_filterText, Qt::CaseInsensitive)) {
            m_items.append(item);
        }
    }
    endResetModel();

    emit filterTextChanged();
    emit countChanged();
}

void EvaFileFolderModel::setShowHidden(bool show) {
    if (m_showHidden == show) return;
    m_showHidden = show;
    updateItems();
    emit showHiddenChanged();
}

void EvaFileFolderModel::updateItems() {
    beginResetModel();
    m_allItems.clear();
    m_items.clear();

    QDir dir(m_path);
    if (dir.exists()) {
        QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot;
        if (m_showHidden) filters |= QDir::Hidden;

        QFileInfoList list = dir.entryInfoList(filters, QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);
        for (const QFileInfo& fi : list) {
            FileItemData item;
            item.name = fi.fileName();
            item.path = fi.absoluteFilePath();
            item.isDir = fi.isDir();
            item.isHidden = fi.isHidden() || fi.fileName().startsWith('.');
            item.sizeBytes = fi.isDir() ? 0 : fi.size();
            item.sizeFormatted = fi.isDir() ? "Folder" : formatFileSize(fi.size());
            item.extension = fi.suffix().toLower();
            item.modified = fi.lastModified().toString("yyyy-MM-dd HH:mm");
            item.iconName = determineIconName(fi);

            if (fi.isDir()) item.type = "Directory";
            else if (!fi.suffix().isEmpty()) item.type = fi.suffix().toUpper() + " File";
            else item.type = "File";

            m_allItems.append(item);
            if (m_filterText.isEmpty() || item.name.contains(m_filterText, Qt::CaseInsensitive)) {
                m_items.append(item);
            }
        }
    }
    endResetModel();
    emit countChanged();
}

QString EvaFileFolderModel::determineIconName(const QFileInfo& fi) {
    if (fi.isDir()) {
        QString name = fi.fileName().toLower();
        if (name == "documents") return "documents";
        if (name == "downloads") return "downloads";
        if (name == "music") return "music";
        if (name == "pictures") return "pictures";
        if (name == "videos") return "videos";
        if (name == "desktop") return "desktop";
        return "folder";
    }

    QString ext = fi.suffix().toLower();
    if (ext == "zip" || ext == "tar" || ext == "gz" || ext == "xz" || ext == "bz2" || ext == "7z" || ext == "rar") return "file-archive";
    if (ext == "cpp" || ext == "c" || ext == "h" || ext == "hpp" || ext == "py" || ext == "js" || ext == "ts" || ext == "qml" || ext == "rs" || ext == "sh" || ext == "html" || ext == "css") return "file-code";
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "svg" || ext == "webp" || ext == "gif") return "file-image";
    if (ext == "mp4" || ext == "mkv" || ext == "webm" || ext == "avi") return "file-video";
    if (ext == "mp3" || ext == "flac" || ext == "ogg" || ext == "wav") return "file-audio";
    if (ext == "pdf") return "file-pdf";
    return "file";
}

QString EvaFileFolderModel::formatFileSize(qint64 bytes) {
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return QString("%1 KB").arg(QString::number(bytes / 1024.0, 'f', 1));
    if (bytes < 1024 * 1024 * 1024) return QString("%1 MB").arg(QString::number(bytes / (1024.0 * 1024.0), 'f', 1));
    return QString("%1 GB").arg(QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2));
}

// =============================================================================
// EvaFileController Implementation
// =============================================================================

EvaFileController::EvaFileController(QObject* parent)
    : QObject(parent)
{
    m_currentPath = QDir::homePath();
    m_splitPath = QDir::homePath() + "/Downloads";
    m_showHidden = ConfigManager::instance().config().showHidden;

    // Connect File Operations
    connect(&FileOperations::instance(), &FileOperations::operationStarted, this, &EvaFileController::onOperationStarted);
    connect(&FileOperations::instance(), &FileOperations::operationProgress, this, &EvaFileController::onOperationProgress);
    connect(&FileOperations::instance(), &FileOperations::operationFinished, this, &EvaFileController::onOperationFinished);

    // Connect Evalink
    connect(&EvalinkManager::instance(), &EvalinkManager::tasksUpdated, this, &EvaFileController::onEvalinkTasksUpdated);
    connect(&EvalinkManager::instance(), &EvalinkManager::downloadCompleted, this, &EvaFileController::onEvalinkDownloadCompleted);
}

void EvaFileController::setCurrentPath(const QString& path) {
    navigateTo(path);
}

bool EvaFileController::canGoUp() const {
    QDir d(m_currentPath);
    return d.cdUp();
}

void EvaFileController::navigateTo(const QString& path) {
    QString clean = QDir::cleanPath(path);
    if (clean.isEmpty() || !QDir(clean).exists() || clean == m_currentPath) return;

    m_backHistory.append(m_currentPath);
    m_forwardHistory.clear();
    m_currentPath = clean;

    emit currentPathChanged(m_currentPath);
    emit historyChanged();
    emit folderRefreshRequested();
}

void EvaFileController::refresh() {
    emit folderRefreshRequested();
}

void EvaFileController::goBack() {
    if (m_backHistory.isEmpty()) return;
    m_forwardHistory.append(m_currentPath);
    m_currentPath = m_backHistory.takeLast();

    emit currentPathChanged(m_currentPath);
    emit historyChanged();
    emit folderRefreshRequested();
}

void EvaFileController::goForward() {
    if (m_forwardHistory.isEmpty()) return;
    m_backHistory.append(m_currentPath);
    m_currentPath = m_forwardHistory.takeLast();

    emit currentPathChanged(m_currentPath);
    emit historyChanged();
    emit folderRefreshRequested();
}

void EvaFileController::goUp() {
    QDir d(m_currentPath);
    if (d.cdUp()) {
        navigateTo(d.absolutePath());
    }
}

void EvaFileController::goHome() {
    navigateTo(QDir::homePath());
}

void EvaFileController::setSearchQuery(const QString& query) {
    if (m_searchQuery == query) return;
    m_searchQuery = query;
    emit searchQueryChanged();
}

void EvaFileController::setShowHidden(bool show) {
    if (m_showHidden == show) return;
    m_showHidden = show;
    ConfigManager::instance().config().showHidden = show;
    ConfigManager::instance().save();
    emit showHiddenChanged();
}

void EvaFileController::setViewMode(int mode) {
    if (m_viewMode == mode) return;
    m_viewMode = mode;
    emit viewModeChanged();
}

void EvaFileController::setIsSplit(bool split) {
    if (m_isSplit == split) return;
    m_isSplit = split;
    emit splitChanged();
}

void EvaFileController::setSplitPath(const QString& path) {
    QString clean = QDir::cleanPath(path);
    if (m_splitPath == clean || !QDir(clean).exists()) return;
    m_splitPath = clean;
    emit splitPathChanged();
}

void EvaFileController::setPreviewVisible(bool visible) {
    if (m_previewVisible == visible) return;
    m_previewVisible = visible;
    emit previewVisibleChanged();
}

void EvaFileController::setEvalinkDrawerVisible(bool visible) {
    if (m_evalinkDrawerVisible == visible) return;
    m_evalinkDrawerVisible = visible;
    emit evalinkDrawerVisibleChanged();
}

void EvaFileController::openFile(const QString& path) {
    QFileInfo fi(path);
    if (fi.isDir()) {
        navigateTo(fi.absoluteFilePath());
    } else {
        EvaSuiteBridge::openWithDefaultApp(fi.absoluteFilePath());
    }
}

void EvaFileController::openInEvaTerm(const QString& path) {
    QString target = path.isEmpty() ? m_currentPath : path;
    EvaSuiteBridge::openInEvaTerm(target);
}

void EvaFileController::sortWithEvaSort(const QString& path) {
    QString target = path.isEmpty() ? m_currentPath : path;
    EvaSuiteBridge::sortWithEvaSort(target, nullptr);
    emit folderRefreshRequested();
}

void EvaFileController::copyLocation(const QString& path) {
    QString target = path.isEmpty() ? m_currentPath : path;
    QGuiApplication::clipboard()->setText(target);
    m_statusText = "Location copied to clipboard: " + target;
    emit statusTextChanged();
}

void EvaFileController::copyPaths(const QStringList& paths, const QString& targetDir) {
    QString dest = targetDir.isEmpty() ? m_currentPath : targetDir;
    FileOperations::instance().copy(paths, dest);
}

void EvaFileController::movePaths(const QStringList& paths, const QString& targetDir) {
    QString dest = targetDir.isEmpty() ? m_currentPath : targetDir;
    FileOperations::instance().move(paths, dest);
}

void EvaFileController::trashPaths(const QStringList& paths) {
    FileOperations::instance().trash(paths);
}

void EvaFileController::deletePaths(const QStringList& paths) {
    FileOperations::instance().permanentDelete(paths);
}

void EvaFileController::renameItem(const QString& oldPath, const QString& newName) {
    FileOperations::instance().rename(oldPath, newName);
    emit folderRefreshRequested();
}

void EvaFileController::createFolder(const QString& parentDir, const QString& name) {
    QString dir = parentDir.isEmpty() ? m_currentPath : parentDir;
    FileOperations::instance().createDirectory(dir, name);
    emit folderRefreshRequested();
}

void EvaFileController::createFile(const QString& parentDir, const QString& name, const QString& content) {
    QString dir = parentDir.isEmpty() ? m_currentPath : parentDir;
    FileOperations::instance().createFileWithContent(dir, name, content);
    emit folderRefreshRequested();
}

void EvaFileController::compressZip(const QStringList& sources, const QString& destZip) {
    FileOperations::instance().compressToZip(sources, destZip);
}

void EvaFileController::extractZip(const QString& zipFile, const QString& destDir) {
    QString dest = destDir.isEmpty() ? m_currentPath : destDir;
    FileOperations::instance().extractZip(zipFile, dest);
}

void EvaFileController::addBookmark(const QString& path) {
    ConfigManager::instance().addBookmark(path);
    emit bookmarksChanged();
}

void EvaFileController::removeBookmark(const QString& path) {
    ConfigManager::instance().removeBookmark(path);
    emit bookmarksChanged();
}

void EvaFileController::previewFile(const QString& path) {
    loadPreview(path);
    setPreviewVisible(true);
}

void EvaFileController::togglePreview() {
    setPreviewVisible(!m_previewVisible);
}

void EvaFileController::startEvalinkDownload(const QString& url, const QString& dir, const QString& filename) {
    QString dest = dir.isEmpty() ? m_currentPath : dir;
    EvalinkManager::instance().addDownload(url, dest, filename);
    setEvalinkDrawerVisible(true);
}

void EvaFileController::pauseEvalink(const QString& gid) {
    EvalinkManager::instance().pauseDownload(gid);
}

void EvaFileController::resumeEvalink(const QString& gid) {
    EvalinkManager::instance().resumeDownload(gid);
}

void EvaFileController::removeEvalink(const QString& gid) {
    EvalinkManager::instance().cancelDownload(gid);
}

void EvaFileController::purgeEvalink() {
    EvalinkManager::instance().purgeCompleted();
}

void EvaFileController::toggleEvalinkDrawer() {
    setEvalinkDrawerVisible(!m_evalinkDrawerVisible);
}

void EvaFileController::toggleSplit() {
    setIsSplit(!m_isSplit);
}

void EvaFileController::loadPreview(const QString& path) {
    QFileInfo fi(path);
    if (!fi.exists()) return;

    m_previewPath = fi.absoluteFilePath();
    m_previewTitle = fi.fileName();
    m_previewSize = EvaFileFolderModel::formatFileSize(fi.size());

    QString ext = fi.suffix().toLower();
    m_previewIsImage = (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "svg" || ext == "webp" || ext == "gif");
    m_previewIsMedia = (ext == "mp4" || ext == "mkv" || ext == "webm" || ext == "mp3" || ext == "flac" || ext == "wav");

    // Text preview
    if (!fi.isDir() && !m_previewIsImage && !m_previewIsMedia && fi.size() < 1024 * 512) {
        QFile file(m_previewPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_previewContent = QString::fromUtf8(file.read(8192));
            file.close();
        }
    } else {
        m_previewContent.clear();
    }

    // Checksum for files < 15MB
    if (!fi.isDir() && fi.size() < 15 * 1024 * 1024) {
        QFile file(m_previewPath);
        if (file.open(QIODevice::ReadOnly)) {
            QCryptographicHash hash(QCryptographicHash::Sha256);
            hash.addData(&file);
            m_previewChecksum = QString::fromUtf8(hash.result().toHex()).left(16) + "...";
            file.close();
        }
    } else {
        m_previewChecksum = "N/A";
    }

    emit previewChanged();
}

QString EvaFileController::freeSpaceText() const {
    QStorageInfo storage(m_currentPath);
    if (storage.isValid()) {
        return QString("%1 free of %2")
            .arg(EvaFileFolderModel::formatFileSize(storage.bytesAvailable()))
            .arg(EvaFileFolderModel::formatFileSize(storage.bytesTotal()));
    }
    return "";
}

QVariantList EvaFileController::places() const {
    QVariantList list;
    auto addPlace = [&](const QString& name, const QString& path, const QString& icon) {
        QVariantMap map;
        map["name"] = name;
        map["path"] = path;
        map["icon"] = icon;
        list.append(map);
    };

    addPlace("Home", QDir::homePath(), "home");
    addPlace("Root", "/", "disk");
    addPlace("Documents", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "documents");
    addPlace("Downloads", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), "downloads");
    addPlace("Music", QStandardPaths::writableLocation(QStandardPaths::MusicLocation), "music");
    addPlace("Pictures", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation), "pictures");
    addPlace("Videos", QStandardPaths::writableLocation(QStandardPaths::MoviesLocation), "videos");
    addPlace("Trash", QDir::homePath() + "/.local/share/Trash/files", "trash");
    return list;
}

QVariantList EvaFileController::drives() const {
    QVariantList list;
    const auto volumes = QStorageInfo::mountedVolumes();
    for (const auto& vol : volumes) {
        if (!vol.isValid() || !vol.isReady() || vol.isReadOnly() || vol.rootPath().startsWith("/var") || vol.rootPath().startsWith("/sys") || vol.rootPath().startsWith("/proc")) {
            continue;
        }
        QVariantMap map;
        map["name"] = vol.displayName().isEmpty() ? vol.rootPath() : vol.displayName();
        map["path"] = vol.rootPath();
        map["totalBytes"] = vol.bytesTotal();
        map["freeBytes"] = vol.bytesAvailable();
        map["usedPercent"] = vol.bytesTotal() > 0 ? static_cast<int>(((vol.bytesTotal() - vol.bytesAvailable()) * 100) / vol.bytesTotal()) : 0;
        list.append(map);
    }
    return list;
}

QStringList EvaFileController::bookmarks() const {
    return ConfigManager::instance().config().customBookmarks;
}

void EvaFileController::onEvalinkTasksUpdated(const QList<EvalinkTask>& tasks, qint64 globalSpeed, int activeCount) {
    m_evalinkActiveCount = activeCount;
    m_evalinkGlobalSpeed = EvalinkManager::formatSpeed(globalSpeed);

    m_evalinkTasks.clear();
    for (const auto& task : tasks) {
        QVariantMap map;
        map["gid"] = task.gid;
        map["filename"] = task.name;
        map["status"] = task.status;
        map["totalLength"] = EvalinkManager::formatBytes(task.totalBytes);
        map["completedLength"] = EvalinkManager::formatBytes(task.completedBytes);
        map["downloadSpeed"] = EvalinkManager::formatSpeed(task.downloadSpeed);
        map["progressPercent"] = task.percent;
        map["dir"] = task.dir;
        m_evalinkTasks.append(map);
    }
    emit evalinkUpdated();
}

void EvaFileController::onEvalinkDownloadCompleted(const QString& name, const QString& dir) {
    m_statusText = "Download finished: " + name;
    emit statusTextChanged();
    if (dir == m_currentPath) {
        emit folderRefreshRequested();
    }
}

void EvaFileController::onOperationStarted(const QString& desc) {
    m_opRunning = true;
    m_opText = desc;
    m_opPercent = 0;
    emit opProgressChanged();
}

void EvaFileController::onOperationProgress(const OperationProgress& p) {
    m_opPercent = p.percent;
    m_opText = QString("Processing %1 (%2%)").arg(p.currentFile).arg(p.percent);
    emit opProgressChanged();
}

void EvaFileController::onOperationFinished(bool success, const QString& message) {
    m_opRunning = false;
    m_opPercent = 0;
    m_statusText = message;
    emit opProgressChanged();
    emit statusTextChanged();
    emit folderRefreshRequested();
}
