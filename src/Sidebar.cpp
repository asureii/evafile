#include "Sidebar.hpp"
#include "Config.hpp"
#include <QDir>
#include <QStandardPaths>
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>
#include <QIcon>
#include <QStorageInfo>

Sidebar::Sidebar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("SidebarContainer");
    setMinimumWidth(180);
    setMaximumWidth(320);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_tree = new QTreeWidget(this);
    m_tree->setHeaderHidden(true);
    m_tree->setRootIsDecorated(false);
    m_tree->setIndentation(10);
    m_tree->setAnimated(true);
    m_tree->setUniformRowHeights(true);
    m_tree->setFocusPolicy(Qt::NoFocus);

    layout->addWidget(m_tree);

    connect(m_tree, &QTreeWidget::itemClicked, this, &Sidebar::onItemClicked);

    refresh();
}

void Sidebar::refresh() {
    m_tree->clear();

    setupPlaces();
    setupDrives();
    setupBookmarks();

    m_tree->expandAll();
}

QTreeWidgetItem* Sidebar::addCategory(const QString& title) {
    QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
    item->setText(0, title);
    item->setFlags(Qt::ItemIsEnabled);
    item->setForeground(0, QBrush(QColor("#e32a10")));
    QFont font = item->font(0);
    font.setBold(true);
    font.setPointSize(9);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 1.0);
    item->setFont(0, font);
    return item;
}

void Sidebar::addPlaceItem(QTreeWidgetItem* category, const QString& name, const QString& path, const QString& iconPath) {
    if (!QDir(path).exists() && path != (QDir::homePath() + "/.local/share/Trash/files")) {
        // Create standard directory if missing
        QDir().mkpath(path);
    }

    QTreeWidgetItem* item = new QTreeWidgetItem(category);
    item->setText(0, name);
    item->setIcon(0, QIcon(iconPath));
    item->setData(0, Qt::UserRole, path);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

void Sidebar::setupPlaces() {
    m_placesCategory = addCategory("PLACES");

    addPlaceItem(m_placesCategory, "Home", QDir::homePath(), ":/icons/home.svg");
    addPlaceItem(m_placesCategory, "Desktop", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), ":/icons/desktop.svg");
    addPlaceItem(m_placesCategory, "Downloads", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), ":/icons/downloads.svg");
    addPlaceItem(m_placesCategory, "Documents", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), ":/icons/documents.svg");
    addPlaceItem(m_placesCategory, "Pictures", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation), ":/icons/pictures.svg");
    addPlaceItem(m_placesCategory, "Music", QStandardPaths::writableLocation(QStandardPaths::MusicLocation), ":/icons/music.svg");
    addPlaceItem(m_placesCategory, "Videos", QStandardPaths::writableLocation(QStandardPaths::MoviesLocation), ":/icons/videos.svg");

    QString trashPath = QDir::homePath() + "/.local/share/Trash/files";
    QDir().mkpath(trashPath);
    addPlaceItem(m_placesCategory, "Trash", trashPath, ":/icons/trash.svg");
}

void Sidebar::setupDrives() {
    m_drivesCategory = addCategory("DRIVES");

    QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo& vol : volumes) {
        if (vol.isValid() && vol.isReady() && !vol.isReadOnly() && vol.bytesTotal() > 0) {
            QString name = vol.displayName();
            if (name.isEmpty()) name = vol.rootPath();
            if (vol.rootPath() == "/") name = "Root (/)";

            double freeGB = vol.bytesAvailable() / (1024.0 * 1024.0 * 1024.0);
            double totalGB = vol.bytesTotal() / (1024.0 * 1024.0 * 1024.0);
            QString label = QString("%1 (%2G free)").arg(name).arg(QString::number(freeGB, 'f', 1));

            addPlaceItem(m_drivesCategory, label, vol.rootPath(), ":/icons/disk.svg");
        }
    }
}

void Sidebar::setupBookmarks() {
    m_bookmarksCategory = addCategory("BOOKMARKS");

    const QStringList& bms = ConfigManager::instance().config().customBookmarks;
    for (const QString& bm : bms) {
        QFileInfo fi(bm);
        QString name = fi.fileName();
        if (name.isEmpty()) name = bm;
        addPlaceItem(m_bookmarksCategory, name, bm, ":/icons/bookmark.svg");
    }
}

void Sidebar::onItemClicked(QTreeWidgetItem* item, int /*column*/) {
    if (!item) return;
    QString path = item->data(0, Qt::UserRole).toString();
    if (!path.isEmpty() && QDir(path).exists()) {
        emit locationSelected(path);
    }
}

void Sidebar::selectPath(const QString& path) {
    for (int c = 0; c < m_tree->topLevelItemCount(); ++c) {
        QTreeWidgetItem* cat = m_tree->topLevelItem(c);
        for (int i = 0; i < cat->childCount(); ++i) {
            QTreeWidgetItem* child = cat->child(i);
            if (child->data(0, Qt::UserRole).toString() == path) {
                m_tree->setCurrentItem(child);
                return;
            }
        }
    }
    m_tree->clearSelection();
}

void Sidebar::contextMenuEvent(QContextMenuEvent* event) {
    QTreeWidgetItem* item = m_tree->itemAt(event->pos());
    if (!item) return;

    QString path = item->data(0, Qt::UserRole).toString();
    if (path.isEmpty()) return;

    QMenu menu(this);
    QAction* openAct = menu.addAction(QIcon(":/icons/folder-open.svg"), "Open Location");

    bool isCustomBookmark = ConfigManager::instance().config().customBookmarks.contains(path);
    QAction* removeBmAct = nullptr;
    if (isCustomBookmark) {
        removeBmAct = menu.addAction(QIcon(":/icons/trash.svg"), "Remove Bookmark");
    }

    QAction* chosen = menu.exec(event->globalPos());
    if (chosen == openAct) {
        emit locationSelected(path);
    } else if (removeBmAct && chosen == removeBmAct) {
        ConfigManager::instance().removeBookmark(path);
        refresh();
    }
}
