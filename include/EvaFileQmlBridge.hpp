#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QFileSystemWatcher>
#include <QDir>
#include <QFileInfo>
#include "EvalinkManager.hpp"
#include "FileOperations.hpp"

struct FileItemData {
    QString name;
    QString path;
    bool isDir = false;
    bool isHidden = false;
    QString sizeFormatted;
    qint64 sizeBytes = 0;
    QString type;
    QString modified;
    QString iconName;
    QString extension;
};

class EvaFileFolderModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(bool showHidden READ showHidden WRITE setShowHidden NOTIFY showHiddenChanged)

public:
    enum FileRoles {
        NameRole = Qt::UserRole + 1,
        PathRole,
        IsDirRole,
        IsHiddenRole,
        SizeRole,
        SizeBytesRole,
        TypeRole,
        ModifiedRole,
        IconNameRole,
        ExtensionRole
    };
    Q_ENUM(FileRoles)

    explicit EvaFileFolderModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString path() const { return m_path; }
    void setPath(const QString& path);

    int count() const { return m_items.size(); }
    QString filterText() const { return m_filterText; }
    void setFilterText(const QString& text);

    bool showHidden() const { return m_showHidden; }
    void setShowHidden(bool show);

public slots:
    void reload();
    Q_INVOKABLE QVariantMap get(int index) const;

    static QString formatFileSize(qint64 bytes);

signals:
    void pathChanged(const QString& path);
    void countChanged();
    void filterTextChanged();
    void showHiddenChanged();

private:
    void updateItems();
    static QString determineIconName(const QFileInfo& fi);

    QString m_path;
    QString m_filterText;
    bool m_showHidden = false;
    QList<FileItemData> m_allItems;
    QList<FileItemData> m_items;
    QFileSystemWatcher* m_watcher;
};

class EvaFileController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentPath READ currentPath WRITE setCurrentPath NOTIFY currentPathChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY historyChanged)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY historyChanged)
    Q_PROPERTY(bool canGoUp READ canGoUp NOTIFY currentPathChanged)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(bool showHidden READ showHidden WRITE setShowHidden NOTIFY showHiddenChanged)
    Q_PROPERTY(int viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(bool isSplit READ isSplit WRITE setIsSplit NOTIFY splitChanged)
    Q_PROPERTY(QString splitPath READ splitPath WRITE setSplitPath NOTIFY splitPathChanged)
    Q_PROPERTY(bool previewVisible READ previewVisible WRITE setPreviewVisible NOTIFY previewVisibleChanged)
    Q_PROPERTY(QString previewPath READ previewPath NOTIFY previewChanged)
    Q_PROPERTY(QString previewTitle READ previewTitle NOTIFY previewChanged)
    Q_PROPERTY(QString previewContent READ previewContent NOTIFY previewChanged)
    Q_PROPERTY(bool previewIsImage READ previewIsImage NOTIFY previewChanged)
    Q_PROPERTY(bool previewIsMedia READ previewIsMedia NOTIFY previewChanged)
    Q_PROPERTY(QString previewChecksum READ previewChecksum NOTIFY previewChanged)
    Q_PROPERTY(QString previewSize READ previewSize NOTIFY previewChanged)
    Q_PROPERTY(bool evalinkDrawerVisible READ evalinkDrawerVisible WRITE setEvalinkDrawerVisible NOTIFY evalinkDrawerVisibleChanged)
    Q_PROPERTY(int evalinkActiveCount READ evalinkActiveCount NOTIFY evalinkUpdated)
    Q_PROPERTY(QString evalinkGlobalSpeed READ evalinkGlobalSpeed NOTIFY evalinkUpdated)
    Q_PROPERTY(QVariantList evalinkTasks READ evalinkTasks NOTIFY evalinkUpdated)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString freeSpaceText READ freeSpaceText NOTIFY currentPathChanged)
    Q_PROPERTY(QVariantList places READ places NOTIFY placesChanged)
    Q_PROPERTY(QVariantList drives READ drives NOTIFY drivesChanged)
    Q_PROPERTY(QStringList bookmarks READ bookmarks NOTIFY bookmarksChanged)
    Q_PROPERTY(bool opRunning READ opRunning NOTIFY opProgressChanged)
    Q_PROPERTY(QString opText READ opText NOTIFY opProgressChanged)
    Q_PROPERTY(int opPercent READ opPercent NOTIFY opProgressChanged)

public:
    explicit EvaFileController(QObject* parent = nullptr);

    QString currentPath() const { return m_currentPath; }
    void setCurrentPath(const QString& path);

    bool canGoBack() const { return !m_backHistory.isEmpty(); }
    bool canGoForward() const { return !m_forwardHistory.isEmpty(); }
    bool canGoUp() const;

    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString& query);

    bool showHidden() const { return m_showHidden; }
    void setShowHidden(bool show);

    int viewMode() const { return m_viewMode; }
    void setViewMode(int mode);

    bool isSplit() const { return m_isSplit; }
    void setIsSplit(bool split);

    QString splitPath() const { return m_splitPath; }
    void setSplitPath(const QString& path);

    bool previewVisible() const { return m_previewVisible; }
    void setPreviewVisible(bool visible);

    QString previewPath() const { return m_previewPath; }
    QString previewTitle() const { return m_previewTitle; }
    QString previewContent() const { return m_previewContent; }
    bool previewIsImage() const { return m_previewIsImage; }
    bool previewIsMedia() const { return m_previewIsMedia; }
    QString previewChecksum() const { return m_previewChecksum; }
    QString previewSize() const { return m_previewSize; }

    bool evalinkDrawerVisible() const { return m_evalinkDrawerVisible; }
    void setEvalinkDrawerVisible(bool visible);

    int evalinkActiveCount() const { return m_evalinkActiveCount; }
    QString evalinkGlobalSpeed() const { return m_evalinkGlobalSpeed; }
    QVariantList evalinkTasks() const { return m_evalinkTasks; }

    QString statusText() const { return m_statusText; }
    QString freeSpaceText() const;

    QVariantList places() const;
    QVariantList drives() const;
    QStringList bookmarks() const;

    bool opRunning() const { return m_opRunning; }
    QString opText() const { return m_opText; }
    int opPercent() const { return m_opPercent; }

public slots:
    // Navigation
    Q_INVOKABLE void navigateTo(const QString& path);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void goForward();
    Q_INVOKABLE void goUp();
    Q_INVOKABLE void goHome();

    // Interaction & Apps
    Q_INVOKABLE void openFile(const QString& path);
    Q_INVOKABLE void openInEvaTerm(const QString& path = QString());
    Q_INVOKABLE void sortWithEvaSort(const QString& path = QString());
    Q_INVOKABLE void copyLocation(const QString& path = QString());

    // File Operations
    Q_INVOKABLE void copyPaths(const QStringList& paths, const QString& targetDir);
    Q_INVOKABLE void movePaths(const QStringList& paths, const QString& targetDir);
    Q_INVOKABLE void trashPaths(const QStringList& paths);
    Q_INVOKABLE void deletePaths(const QStringList& paths);
    Q_INVOKABLE void renameItem(const QString& oldPath, const QString& newName);
    Q_INVOKABLE void createFolder(const QString& parentDir, const QString& name);
    Q_INVOKABLE void createFile(const QString& parentDir, const QString& name, const QString& content = QString());
    Q_INVOKABLE void compressZip(const QStringList& sources, const QString& destZip);
    Q_INVOKABLE void extractZip(const QString& zipFile, const QString& destDir);

    // Bookmarks
    Q_INVOKABLE void addBookmark(const QString& path);
    Q_INVOKABLE void removeBookmark(const QString& path);

    // Preview
    Q_INVOKABLE void previewFile(const QString& path);
    Q_INVOKABLE void togglePreview();

    // Evalink
    Q_INVOKABLE void startEvalinkDownload(const QString& url, const QString& dir = QString(), const QString& filename = QString());
    Q_INVOKABLE void pauseEvalink(const QString& gid);
    Q_INVOKABLE void resumeEvalink(const QString& gid);
    Q_INVOKABLE void removeEvalink(const QString& gid);
    Q_INVOKABLE void purgeEvalink();
    Q_INVOKABLE void toggleEvalinkDrawer();

    // Split
    Q_INVOKABLE void toggleSplit();

signals:
    void currentPathChanged(const QString& path);
    void historyChanged();
    void searchQueryChanged();
    void showHiddenChanged();
    void viewModeChanged();
    void splitChanged();
    void splitPathChanged();
    void previewVisibleChanged();
    void previewChanged();
    void evalinkDrawerVisibleChanged();
    void evalinkUpdated();
    void statusTextChanged();
    void placesChanged();
    void drivesChanged();
    void bookmarksChanged();
    void opProgressChanged();
    void folderRefreshRequested();

private slots:
    void onEvalinkTasksUpdated(const QList<EvalinkTask>& tasks, qint64 globalSpeed, int activeCount);
    void onEvalinkDownloadCompleted(const QString& name, const QString& dir);
    void onOperationStarted(const QString& desc);
    void onOperationProgress(const OperationProgress& p);
    void onOperationFinished(bool success, const QString& message);

private:
    void loadPreview(const QString& path);

    QString m_currentPath;
    QStringList m_backHistory;
    QStringList m_forwardHistory;
    QString m_searchQuery;
    bool m_showHidden = false;
    int m_viewMode = 0; // 0: Details, 1: Icons
    bool m_isSplit = false;
    QString m_splitPath;

    bool m_previewVisible = false;
    QString m_previewPath;
    QString m_previewTitle;
    QString m_previewContent;
    bool m_previewIsImage = false;
    bool m_previewIsMedia = false;
    QString m_previewChecksum;
    QString m_previewSize;

    bool m_evalinkDrawerVisible = false;
    int m_evalinkActiveCount = 0;
    QString m_evalinkGlobalSpeed = "0 B/s";
    QVariantList m_evalinkTasks;

    QString m_statusText = "Ready";
    bool m_opRunning = false;
    QString m_opText;
    int m_opPercent = 0;
};
