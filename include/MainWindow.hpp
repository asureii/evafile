#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QStackedWidget>
#include <QProgressBar>
#include <QLabel>
#include <QToolBar>
#include <QAction>
#include "TabBar.hpp"
#include "BreadcrumbBar.hpp"
#include "Sidebar.hpp"
#include "SplitManager.hpp"
#include "PreviewDrawer.hpp"
#include "FileOperations.hpp"
#include "EvalinkDrawer.hpp"
#include "EvalinkDialog.hpp"
#include "EvalinkManager.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void navigateTo(const QString& path);

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onNewTab(const QString& path = QString());
    void onCloseTab(int index);
    void onTabChanged(int index);
    void onBreadcrumbPathChanged(const QString& path);
    void onSidebarLocationSelected(const QString& path);
    void onActivePathChanged(const QString& path);
    void onFileSelected(const QString& path);

    void onBack();
    void onForward();
    void onUp();
    void onHome();
    void onReload();

    void onToggleSidebar();
    void onToggleSplit();
    void onTogglePreview();
    void onToggleHidden();
    void onToggleSearch();
    void onOpenTerminal();
    void onSortWithEvaSort();

    void onNewEvalinkDownload();
    void onToggleEvalinkDrawer();
    void onEvalinkTasksUpdated(const QList<EvalinkTask>& tasks, qint64 globalSpeed, int activeCount);
    void onEvalinkDownloadCompleted(const QString& name, const QString& dir);

    void setDetailsView();
    void setIconsView();

    void onOperationStarted(const QString& desc);
    void onOperationProgress(const OperationProgress& p);
    void onOperationFinished(bool success, const QString& message);

private:
    void setupUi();
    void setupToolBar();
    void setupStatusBar();
    void setupShortcuts();
    void updateNavigationButtons();
    SplitManager* currentSplitManager() const;

    // Navigation history
    struct TabSession {
        SplitManager* splitManager = nullptr;
        QStringList backHistory;
        QStringList forwardHistory;
    };

    QList<TabSession> m_tabSessions;

    // Widgets
    TabBar* m_tabBar;
    QToolBar* m_toolBar;
    BreadcrumbBar* m_breadcrumbBar;
    Sidebar* m_sidebar;
    PreviewDrawer* m_previewDrawer;
    EvalinkDrawer* m_evalinkDrawer;
    QStackedWidget* m_tabStack;
    QSplitter* m_mainSplitter;

    // Actions
    QAction* m_actBack;
    QAction* m_actForward;
    QAction* m_actUp;
    QAction* m_actHome;
    QAction* m_actReload;
    QAction* m_actSidebar;
    QAction* m_actSplit;
    QAction* m_actPreview;
    QAction* m_actHidden;
    QAction* m_actSearch;
    QAction* m_actDetails;
    QAction* m_actIcons;
    QAction* m_actTerminal;
    QAction* m_actSort;
    QAction* m_actEvalink;

    // Status bar
    QLabel* m_statusLabel;
    QLabel* m_spaceLabel;
    QProgressBar* m_opProgressBar;
    QLabel* m_opLabel;
    QPushButton* m_evalinkStatusBtn;
};
