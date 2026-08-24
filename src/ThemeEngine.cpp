#include "ThemeEngine.hpp"

ThemeEngine& ThemeEngine::instance() {
    static ThemeEngine s_instance;
    return s_instance;
}

ThemeEngine::ThemeEngine() {
    // Default theme is Crimson Flame
}

void ThemeEngine::setTheme(const Theme& theme) {
    m_theme = theme;
}

QString ThemeEngine::generateStyleSheet() const {
    const QString bg_base = m_theme.bg_base.name();
    const QString bg_dark = m_theme.bg_dark.name();
    const QString bg_surface = m_theme.bg_surface.name();
    const QString bg_input = m_theme.bg_input.name();
    const QString fg_base = m_theme.fg_base.name();
    const QString fg_muted = m_theme.fg_muted.name();
    const QString accent_pri = m_theme.accent_primary.name();
    const QString accent_brt = m_theme.accent_bright.name();
    const QString border_dk = m_theme.border_dark.name();
    const QString border_sub = m_theme.border_subtle.name();
    const QString sel_bg = m_theme.selection_bg.name();
    const QString sel_fg = m_theme.selection_fg.name();

    QString qss = R"(
        /* Global Window and Base */
        QMainWindow, QDialog {
            background-color: @BG_BASE@;
            color: @FG_BASE@;
            font-family: "Inter", "Segoe UI", "Cantarell", "Noto Sans", sans-serif;
            font-size: 13px;
        }

        QWidget {
            color: @FG_BASE@;
            outline: none;
        }

        /* ToolBar */
        QToolBar {
            background-color: @BG_DARK@;
            border-bottom: 1px solid @BORDER_DK@;
            padding: 2px 4px;
            spacing: 3px;
        }

        QToolButton {
            background-color: transparent;
            color: @FG_BASE@;
            border: 1px solid transparent;
            border-radius: 5px;
            padding: 4px 6px;
            font-weight: 500;
        }

        QToolButton:hover {
            background-color: @BG_SURFACE@;
            border: 1px solid @ACCENT_BRT@;
            color: #ffffff;
        }

        QToolButton:pressed, QToolButton:checked {
            background-color: @ACCENT_PRI@;
            color: #ffffff;
            border: 1px solid @ACCENT_BRT@;
        }

        /* Tab Bar (Dolphin / EvaTerm Style) */
        QTabBar {
            background-color: @BG_DARK@;
            qproperty-drawBase: 0;
            border-bottom: 1px solid @BORDER_DK@;
        }

        QTabBar::tab {
            background-color: @BG_SURFACE@;
            color: @FG_MUTED@;
            border: 1px solid @BORDER_DK@;
            border-bottom: none;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            padding: 6px 12px;
            margin-right: 2px;
            font-weight: 500;
            min-width: 48px;
            max-width: 240px;
        }

        QTabBar::tab:hover {
            background-color: #441316;
            color: #ffffff;
            border-color: @ACCENT_BRT@;
        }

        QTabBar::tab:selected {
            background-color: @ACCENT_PRI@;
            color: #ffffff;
            border: 1px solid @ACCENT_BRT@;
            border-bottom: none;
        }

        QTabBar::close-button {
            image: url(:/icons/close.svg);
            subcontrol-position: right;
            padding: 2px;
            border-radius: 3px;
        }

        QTabBar::close-button:hover {
            background-color: #8c1a17;
        }

        /* Inputs and Search */
        QLineEdit {
            background-color: @BG_INPUT@;
            color: @FG_BASE@;
            border: 1px solid @BORDER_DK@;
            border-radius: 5px;
            padding: 5px 10px;
            selection-background-color: @SEL_BG@;
            selection-color: @SEL_FG@;
        }

        QLineEdit:focus {
            border: 1px solid @ACCENT_BRT@;
            background-color: #310b0d;
        }

        /* Views (Details Tree & Grid List) */
        QTreeView, QListView, QTableView {
            background-color: @BG_BASE@;
            alternate-background-color: #2a0a0c;
            border: none;
            color: @FG_BASE@;
            selection-background-color: @SEL_BG@;
            selection-color: @SEL_FG@;
            show-decoration-selected: 1;
            padding: 4px;
        }

        QTreeView::item, QListView::item {
            padding: 4px 6px;
            border-radius: 4px;
            border: 1px solid transparent;
        }

        QTreeView::item:hover, QListView::item:hover {
            background-color: @BG_SURFACE@;
            color: #ffffff;
            border: 1px solid @BORDER_SUB@;
        }

        QTreeView::item:selected, QListView::item:selected {
            background-color: @SEL_BG@;
            color: @SEL_FG@;
        }

        QHeaderView::section {
            background-color: @BG_DARK@;
            color: @FG_MUTED@;
            padding: 5px 8px;
            border: none;
            border-right: 1px solid @BORDER_DK@;
            border-bottom: 1px solid @BORDER_DK@;
            font-weight: 600;
        }

        QHeaderView::section:hover {
            background-color: @BG_SURFACE@;
            color: @FG_BASE@;
        }

        /* Splitter */
        QSplitter::handle {
            background-color: @BORDER_DK@;
        }

        QSplitter::handle:horizontal {
            width: 2px;
        }

        QSplitter::handle:vertical {
            height: 2px;
        }

        QSplitter::handle:hover {
            background-color: @ACCENT_BRT@;
        }

        /* ScrollBars (Sleek Modern) */
        QScrollBar:vertical {
            background-color: @BG_BASE@;
            width: 8px;
            margin: 0px;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical {
            background-color: @BORDER_DK@;
            min-height: 24px;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: @ACCENT_PRI@;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
            height: 0px;
        }

        QScrollBar:horizontal {
            background-color: @BG_BASE@;
            height: 8px;
            margin: 0px;
            border-radius: 4px;
        }

        QScrollBar::handle:horizontal {
            background-color: @BORDER_DK@;
            min-width: 24px;
            border-radius: 4px;
        }

        QScrollBar::handle:horizontal:hover {
            background-color: @ACCENT_PRI@;
        }

        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
            background: none;
            width: 0px;
        }

        /* Sidebar & Places */
        #SidebarContainer {
            background-color: @BG_SURFACE@;
            border-right: 1px solid @BORDER_DK@;
        }

        #SidebarTitle {
            color: @ACCENT_BRT@;
            font-size: 11px;
            font-weight: 700;
            text-transform: uppercase;
            padding: 8px 12px 4px 12px;
            letter-spacing: 0.5px;
        }

        /* Breadcrumb Bar */
        #BreadcrumbContainer {
            background-color: @BG_INPUT@;
            border: 1px solid @BORDER_DK@;
            border-radius: 6px;
            padding: 2px;
        }

        QPushButton.breadcrumb-pill {
            background-color: @BG_SURFACE@;
            color: @FG_BASE@;
            border: 1px solid transparent;
            border-radius: 4px;
            padding: 3px 8px;
            font-weight: 500;
        }

        QPushButton.breadcrumb-pill:hover {
            background-color: @ACCENT_PRI@;
            color: #ffffff;
            border: 1px solid @ACCENT_BRT@;
        }

        /* Preview Panel */
        #PreviewContainer {
            background-color: @BG_SURFACE@;
            border-left: 1px solid @BORDER_DK@;
        }

        /* Context Menu & Popup */
        QMenu {
            background-color: @BG_DARK@;
            border: 1px solid @BORDER_DK@;
            border-radius: 6px;
            padding: 4px;
        }

        QMenu::item {
            color: @FG_BASE@;
            padding: 6px 24px 6px 12px;
            border-radius: 4px;
        }

        QMenu::item:selected {
            background-color: @ACCENT_PRI@;
            color: #ffffff;
        }

        QMenu::separator {
            height: 1px;
            background-color: @BORDER_DK@;
            margin: 4px 6px;
        }

        /* Status Bar */
        QStatusBar {
            background-color: @BG_DARK@;
            border-top: 1px solid @BORDER_DK@;
            color: @FG_MUTED@;
            font-size: 12px;
            padding: 2px 8px;
        }

        /* Progress Bar */
        QProgressBar {
            background-color: @BG_INPUT@;
            border: 1px solid @BORDER_DK@;
            border-radius: 4px;
            text-align: center;
            color: #ffffff;
            font-size: 11px;
            font-weight: 600;
            height: 14px;
        }

        QProgressBar::chunk {
            background-color: @ACCENT_PRI@;
            border-radius: 3px;
        }

        /* Buttons & Dialogs */
        QPushButton {
            background-color: @BG_SURFACE@;
            color: @FG_BASE@;
            border: 1px solid @BORDER_DK@;
            border-radius: 5px;
            padding: 6px 14px;
            font-weight: 500;
        }

        QPushButton:hover {
            background-color: @ACCENT_PRI@;
            color: #ffffff;
            border: 1px solid @ACCENT_BRT@;
        }

        QPushButton:pressed {
            background-color: #a31c19;
        }

        QPushButton:default {
            background-color: @ACCENT_PRI@;
            color: #ffffff;
            border: 1px solid @ACCENT_BRT@;
        }
    )";

    qss.replace("@BG_BASE@", bg_base);
    qss.replace("@BG_DARK@", bg_dark);
    qss.replace("@BG_SURFACE@", bg_surface);
    qss.replace("@BG_INPUT@", bg_input);
    qss.replace("@FG_BASE@", fg_base);
    qss.replace("@FG_MUTED@", fg_muted);
    qss.replace("@ACCENT_PRI@", accent_pri);
    qss.replace("@ACCENT_BRT@", accent_brt);
    qss.replace("@BORDER_DK@", border_dk);
    qss.replace("@BORDER_SUB@", border_sub);
    qss.replace("@SEL_BG@", sel_bg);
    qss.replace("@SEL_FG@", sel_fg);

    return qss;
}

void ThemeEngine::applyToApplication(QApplication* app) {
    if (!app) return;

    QPalette palette;
    palette.setColor(QPalette::Window, m_theme.bg_base);
    palette.setColor(QPalette::WindowText, m_theme.fg_base);
    palette.setColor(QPalette::Base, m_theme.bg_base);
    palette.setColor(QPalette::AlternateBase, QColor("#2a0a0c"));
    palette.setColor(QPalette::ToolTipBase, m_theme.bg_dark);
    palette.setColor(QPalette::ToolTipText, m_theme.fg_base);
    palette.setColor(QPalette::Text, m_theme.fg_base);
    palette.setColor(QPalette::Button, m_theme.bg_surface);
    palette.setColor(QPalette::ButtonText, m_theme.fg_base);
    palette.setColor(QPalette::BrightText, m_theme.accent_bright);
    palette.setColor(QPalette::Link, m_theme.accent_bright);
    palette.setColor(QPalette::Highlight, m_theme.selection_bg);
    palette.setColor(QPalette::HighlightedText, m_theme.selection_fg);

    app->setPalette(palette);
    app->setStyleSheet(generateStyleSheet());
}
