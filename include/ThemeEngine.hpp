#pragma once

#include <QString>
#include <QColor>
#include <QApplication>
#include <QPalette>

struct Theme {
    QString name = "Crimson Flame";
    QColor bg_base{"#25090a"};       // Main file view / central background
    QColor bg_dark{"#1a0506"};       // Tab bar, status bar, darkest background
    QColor bg_surface{"#360e10"};    // Sidebar, cards, breadcrumb pills
    QColor bg_input{"#2c0a0c"};      // Input fields, search box
    QColor fg_base{"#ebdada"};       // Standard text
    QColor fg_muted{"#b58487"};      // Metadata, secondary text
    QColor accent_primary{"#cf2824"};// Selection, primary active buttons
    QColor accent_bright{"#e32a10"}; // Hover glow, active pane borders, highlights
    QColor border_dark{"#6e282c"};   // Dividers, panel borders
    QColor border_subtle{"#451316"}; // Subtle outlines
    QColor selection_bg{"#cf2824"};  // Selection background
    QColor selection_fg{"#ffffff"};  // Selection text
    QColor warning{"#e6ab3c"};
    QColor success{"#e0682b"};
};

class ThemeEngine {
public:
    static ThemeEngine& instance();

    const Theme& currentTheme() const { return m_theme; }
    void setTheme(const Theme& theme);

    QString generateStyleSheet() const;
    void applyToApplication(QApplication* app);

private:
    ThemeEngine();
    Theme m_theme;
};
