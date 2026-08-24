#pragma once

#include <QString>
#include <QStringList>
#include <QSettings>
#include <QSize>
#include <QPoint>

struct AppConfig {
    QString theme = "crimson_flame";
    bool showHidden = false;
    QString defaultViewMode = "details"; // "details", "icons", "compact"
    int iconSize = 64;
    bool confirmDelete = true;
    bool restoreTabs = true;

    // Layout
    bool showSidebar = true;
    bool showPreview = false;
    bool showSplit = false;
    int sidebarWidth = 220;
    int previewWidth = 300;
    QSize windowSize{1100, 700};
    QPoint windowPos{100, 100};

    // Bookmarks
    QStringList customBookmarks;
    QStringList lastOpenedTabs;
};

class ConfigManager {
public:
    static ConfigManager& instance();

    void load();
    void save();

    AppConfig& config() { return m_config; }
    const AppConfig& config() const { return m_config; }

    QString configPath() const;

    void addBookmark(const QString& path);
    void removeBookmark(const QString& path);

private:
    ConfigManager();
    AppConfig m_config;
};
