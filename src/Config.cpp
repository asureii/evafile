#include "Config.hpp"
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>

ConfigManager& ConfigManager::instance() {
    static ConfigManager s_instance;
    return s_instance;
}

ConfigManager::ConfigManager() {
    load();
}

QString ConfigManager::configPath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/evafile";
    QDir().mkpath(configDir);
    return configDir + "/evafile.conf";
}

void ConfigManager::load() {
    QSettings settings(configPath(), QSettings::IniFormat);

    settings.beginGroup("General");
    m_config.theme = settings.value("theme", "crimson_flame").toString();
    m_config.showHidden = settings.value("show_hidden", false).toBool();
    m_config.defaultViewMode = settings.value("default_view_mode", "details").toString();
    m_config.iconSize = settings.value("icon_size", 64).toInt();
    m_config.confirmDelete = settings.value("confirm_delete", true).toBool();
    m_config.restoreTabs = settings.value("restore_tabs", true).toBool();
    settings.endGroup();

    settings.beginGroup("Layout");
    m_config.showSidebar = settings.value("show_sidebar", true).toBool();
    m_config.showPreview = settings.value("show_preview", false).toBool();
    m_config.showSplit = settings.value("show_split", false).toBool();
    m_config.sidebarWidth = settings.value("sidebar_width", 220).toInt();
    m_config.previewWidth = settings.value("preview_width", 300).toInt();
    m_config.windowSize = settings.value("window_size", QSize(1100, 700)).toSize();
    m_config.windowPos = settings.value("window_pos", QPoint(100, 100)).toPoint();
    settings.endGroup();

    settings.beginGroup("Bookmarks");
    m_config.customBookmarks = settings.value("custom_bookmarks").toStringList();
    settings.endGroup();

    settings.beginGroup("Session");
    m_config.lastOpenedTabs = settings.value("last_tabs").toStringList();
    settings.endGroup();

    if (m_config.lastOpenedTabs.isEmpty()) {
        m_config.lastOpenedTabs.append(QDir::homePath());
    }
}

void ConfigManager::save() {
    QSettings settings(configPath(), QSettings::IniFormat);

    settings.beginGroup("General");
    settings.setValue("theme", m_config.theme);
    settings.setValue("show_hidden", m_config.showHidden);
    settings.setValue("default_view_mode", m_config.defaultViewMode);
    settings.setValue("icon_size", m_config.iconSize);
    settings.setValue("confirm_delete", m_config.confirmDelete);
    settings.setValue("restore_tabs", m_config.restoreTabs);
    settings.endGroup();

    settings.beginGroup("Layout");
    settings.setValue("show_sidebar", m_config.showSidebar);
    settings.setValue("show_preview", m_config.showPreview);
    settings.setValue("show_split", m_config.showSplit);
    settings.setValue("sidebar_width", m_config.sidebarWidth);
    settings.setValue("preview_width", m_config.previewWidth);
    settings.setValue("window_size", m_config.windowSize);
    settings.setValue("window_pos", m_config.windowPos);
    settings.endGroup();

    settings.beginGroup("Bookmarks");
    settings.setValue("custom_bookmarks", m_config.customBookmarks);
    settings.endGroup();

    settings.beginGroup("Session");
    settings.setValue("last_tabs", m_config.lastOpenedTabs);
    settings.endGroup();

    settings.sync();
}

void ConfigManager::addBookmark(const QString& path) {
    if (!path.isEmpty() && !m_config.customBookmarks.contains(path)) {
        m_config.customBookmarks.append(path);
        save();
    }
}

void ConfigManager::removeBookmark(const QString& path) {
    if (m_config.customBookmarks.removeAll(path) > 0) {
        save();
    }
}
