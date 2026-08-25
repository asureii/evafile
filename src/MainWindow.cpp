#include "MainWindow.hpp"
#include "Config.hpp"
#include "EvaSuiteBridge.hpp"
#include <QDir>
#include <QStatusBar>
#include <QShortcut>
#include <QStorageInfo>
#include <QCloseEvent>
#include <QApplication>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("EvaFile");
    setWindowIcon(QIcon(":/icons/evafile.svg"));

    setupUi();
    setupToolBar();
    setupStatusBar();
    setupShortcuts();

    // Connect File Operations
    connect(&FileOperations::instance(), &FileOperations::operationStarted, this, &MainWindow::onOperationStarted);
    connect(&FileOperations::instance(), &FileOperations::operationProgress, this, &MainWindow::onOperationProgress);
    connect(&FileOperations::instance(), &FileOperations::operationFinished, this, &MainWindow::onOperationFinished);

    // Restore tabs or open home
    const auto& conf = ConfigManager::instance().config();
    if (conf.restoreTabs && !conf.lastOpenedTabs.isEmpty()) {
        for (const QString& path : conf.lastOpenedTabs) {
            onNewTab(path);
        }
    } else {
        onNewTab(QDir::homePath());
    }

    // Set geometry and minimum constraints for tiling WMs
    setMinimumSize(320, 200);
    resize(conf.windowSize);
    move(conf.windowPos);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
    QWidget* centralContainer = new QWidget(this);
    QVBoxLayout* centralLayout = new QVBoxLayout(centralContainer);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    // Tab Bar on top
    m_tabBar = new TabBar(this);
    connect(m_tabBar, &TabBar::currentChanged, this, &MainWindow::onTabChanged);
    connect(m_tabBar, &TabBar::tabCloseRequested, this, &MainWindow::onCloseTab);
    connect(m_tabBar, &TabBar::newTabRequested, this, [this]() { onNewTab(); });
    centralLayout->addWidget(m_tabBar);

    // Main 3-way Splitter: [ Sidebar | TabStack | PreviewDrawer ]
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    m_sidebar = new Sidebar(this);
    connect(m_sidebar, &Sidebar::locationSelected, this, &MainWindow::onSidebarLocationSelected);
    m_mainSplitter->addWidget(m_sidebar);

    m_tabStack = new QStackedWidget(this);
    m_mainSplitter->addWidget(m_tabStack);

    m_previewDrawer = new PreviewDrawer(this);
    m_previewDrawer->setVisible(ConfigManager::instance().config().showPreview);
    m_mainSplitter->addWidget(m_previewDrawer);

    m_mainSplitter->setCollapsible(0, true);
    m_mainSplitter->setCollapsible(1, false);
    m_mainSplitter->setCollapsible(2, true);

    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);
    m_mainSplitter->setStretchFactor(2, 0);

    centralLayout->addWidget(m_mainSplitter);
    setCentralWidget(centralContainer);
}

void MainWindow::setupToolBar() {
    m_toolBar = addToolBar("Navigation");
    m_toolBar->setMovable(false);
    m_toolBar->setIconSize(QSize(20, 20));

    m_actBack = m_toolBar->addAction(QIcon(":/icons/arrow-left.svg"), "Back (Alt+Left)", this, &MainWindow::onBack);
    m_actForward = m_toolBar->addAction(QIcon(":/icons/arrow-right.svg"), "Forward (Alt+Right)", this, &MainWindow::onForward);
    m_actUp = m_toolBar->addAction(QIcon(":/icons/arrow-up.svg"), "Up (Alt+Up)", this, &MainWindow::onUp);
    m_actHome = m_toolBar->addAction(QIcon(":/icons/home.svg"), "Home", this, &MainWindow::onHome);
    m_actReload = m_toolBar->addAction(QIcon(":/icons/refresh.svg"), "Reload (F5)", this, &MainWindow::onReload);

    m_toolBar->addSeparator();

    // Breadcrumb Bar
    m_breadcrumbBar = new BreadcrumbBar(this);
    connect(m_breadcrumbBar, &BreadcrumbBar::pathChanged, this, &MainWindow::onBreadcrumbPathChanged);
    m_toolBar->addWidget(m_breadcrumbBar);

    m_toolBar->addSeparator();

    // View Switchers
    m_actDetails = m_toolBar->addAction(QIcon(":/icons/view-details.svg"), "Details View (Ctrl+1)", this, &MainWindow::setDetailsView);
    m_actIcons = m_toolBar->addAction(QIcon(":/icons/view-icons.svg"), "Icons View (Ctrl+2)", this, &MainWindow::setIconsView);

    m_toolBar->addSeparator();

    // Dolphin Productivity Tools
    m_actSidebar = m_toolBar->addAction(QIcon(":/icons/sidebar.svg"), "Toggle Sidebar (Ctrl+B)", this, &MainWindow::onToggleSidebar);
    m_actSidebar->setCheckable(true);
    m_actSidebar->setChecked(ConfigManager::instance().config().showSidebar);

    m_actSplit = m_toolBar->addAction(QIcon(":/icons/split.svg"), "Toggle Split View (F3)", this, &MainWindow::onToggleSplit);
    m_actSplit->setCheckable(true);

    m_actPreview = m_toolBar->addAction(QIcon(":/icons/preview.svg"), "Toggle Preview (F11)", this, &MainWindow::onTogglePreview);
    m_actPreview->setCheckable(true);
    m_actPreview->setChecked(ConfigManager::instance().config().showPreview);

    m_actHidden = m_toolBar->addAction(QIcon(":/icons/eye.svg"), "Show Hidden Files (Ctrl+H)", this, &MainWindow::onToggleHidden);
    m_actHidden->setCheckable(true);
    m_actHidden->setChecked(ConfigManager::instance().config().showHidden);

    m_actSearch = m_toolBar->addAction(QIcon(":/icons/search.svg"), "Search / Filter (Ctrl+F)", this, &MainWindow::onToggleSearch);

    m_toolBar->addSeparator();

    // Eva Suite Actions
    m_actTerminal = m_toolBar->addAction(QIcon(":/icons/terminal.svg"), "Open EvaTerm (F4)", this, &MainWindow::onOpenTerminal);
    m_actSort = m_toolBar->addAction(QIcon(":/icons/sort.svg"), "Sort with EvaSort", this, &MainWindow::onSortWithEvaSort);
}

void MainWindow::setupStatusBar() {
    QStatusBar* bar = statusBar();

    m_statusLabel = new QLabel("Ready", this);
    m_spaceLabel = new QLabel(this);

    m_opLabel = new QLabel(this);
    m_opLabel->setVisible(false);

    m_opProgressBar = new QProgressBar(this);
    m_opProgressBar->setFixedWidth(160);
    m_opProgressBar->setVisible(false);

    bar->addWidget(m_statusLabel, 1);
    bar->addPermanentWidget(m_opLabel);
    bar->addPermanentWidget(m_opProgressBar);
    bar->addPermanentWidget(m_spaceLabel);
}

void MainWindow::setupShortcuts() {
    connect(new QShortcut(QKeySequence("Ctrl+T"), this), &QShortcut::activated, this, [this]() { onNewTab(); });
    connect(new QShortcut(QKeySequence("Ctrl+W"), this), &QShortcut::activated, this, [this]() { onCloseTab(m_tabBar->currentIndex()); });
    connect(new QShortcut(QKeySequence("Ctrl+B"), this), &QShortcut::activated, this, &MainWindow::onToggleSidebar);
    connect(new QShortcut(QKeySequence("Ctrl+Tab"), this), &QShortcut::activated, this, [this]() {
        int next = (m_tabBar->currentIndex() + 1) % m_tabBar->count();
        m_tabBar->setCurrentIndex(next);
    });
    connect(new QShortcut(QKeySequence("Ctrl+Shift+Tab"), this), &QShortcut::activated, this, [this]() {
        int prev = (m_tabBar->currentIndex() - 1 + m_tabBar->count()) % m_tabBar->count();
        m_tabBar->setCurrentIndex(prev);
    });
    connect(new QShortcut(QKeySequence("F3"), this), &QShortcut::activated, this, &MainWindow::onToggleSplit);
    connect(new QShortcut(QKeySequence("F4"), this), &QShortcut::activated, this, &MainWindow::onOpenTerminal);
    connect(new QShortcut(QKeySequence("F5"), this), &QShortcut::activated, this, &MainWindow::onReload);
    connect(new QShortcut(QKeySequence("F11"), this), &QShortcut::activated, this, &MainWindow::onTogglePreview);
    connect(new QShortcut(QKeySequence("Space"), this), &QShortcut::activated, this, &MainWindow::onTogglePreview);
    connect(new QShortcut(QKeySequence("Ctrl+H"), this), &QShortcut::activated, this, &MainWindow::onToggleHidden);
    connect(new QShortcut(QKeySequence("Ctrl+L"), this), &QShortcut::activated, this, [this]() { m_breadcrumbBar->focusAddressBar(); });
    connect(new QShortcut(QKeySequence("Ctrl+F"), this), &QShortcut::activated, this, &MainWindow::onToggleSearch);
    connect(new QShortcut(QKeySequence("Ctrl+1"), this), &QShortcut::activated, this, &MainWindow::setDetailsView);
    connect(new QShortcut(QKeySequence("Ctrl+2"), this), &QShortcut::activated, this, &MainWindow::setIconsView);
    connect(new QShortcut(QKeySequence("Ctrl+Shift+N"), this), &QShortcut::activated, this, [this]() {
        SplitManager* sm = currentSplitManager();
        if (sm && sm->activeView()) sm->activeView()->promptCreateFolder();
    });
    connect(new QShortcut(QKeySequence("Ctrl+Alt+N"), this), &QShortcut::activated, this, [this]() {
        SplitManager* sm = currentSplitManager();
        if (sm && sm->activeView()) sm->activeView()->promptCreateFile();
    });
    connect(new QShortcut(QKeySequence("Alt+Left"), this), &QShortcut::activated, this, &MainWindow::onBack);
    connect(new QShortcut(QKeySequence("Alt+Right"), this), &QShortcut::activated, this, &MainWindow::onForward);
    connect(new QShortcut(QKeySequence("Alt+Up"), this), &QShortcut::activated, this, &MainWindow::onUp);
}

SplitManager* MainWindow::currentSplitManager() const {
    int idx = m_tabBar->currentIndex();
    if (idx >= 0 && idx < m_tabSessions.size()) {
        return m_tabSessions[idx].splitManager;
    }
    return nullptr;
}

void MainWindow::onNewTab(const QString& path) {
    QString target = path.isEmpty() ? QDir::homePath() : path;
    if (!QDir(target).exists()) target = QDir::homePath();

    SplitManager* sm = new SplitManager(this);
    sm->setPath(target);
    sm->setShowHidden(ConfigManager::instance().config().showHidden);

    connect(sm, &SplitManager::activePathChanged, this, &MainWindow::onActivePathChanged);
    connect(sm, &SplitManager::activeFileSelected, this, &MainWindow::onFileSelected);
    connect(sm, &SplitManager::statusMessageChanged, this, [this](const QString& msg) {
        m_statusLabel->setText(msg);
    });

    TabSession session;
    session.splitManager = sm;
    m_tabSessions.append(session);

    m_tabStack->addWidget(sm);

    QFileInfo fi(target);
    QString tabTitle = (target == QDir::homePath()) ? "Home" : (target == "/" ? "/" : fi.fileName());
    int tabIdx = m_tabBar->addTab(tabTitle, target);
    m_tabBar->setCurrentIndex(tabIdx);
}

void MainWindow::onCloseTab(int index) {
    if (m_tabBar->count() <= 1) {
        // Don't close last tab, or close application
        return;
    }

    if (index >= 0 && index < m_tabSessions.size()) {
        SplitManager* sm = m_tabSessions[index].splitManager;
        m_tabStack->removeWidget(sm);
        delete sm;
        m_tabSessions.removeAt(index);
        m_tabBar->removeTab(index);
    }
}

void MainWindow::onTabChanged(int index) {
    if (index >= 0 && index < m_tabSessions.size()) {
        m_tabStack->setCurrentIndex(index);
        SplitManager* sm = m_tabSessions[index].splitManager;
        if (sm) {
            QString path = sm->activeView()->currentPath();
            m_breadcrumbBar->setPath(path);
            m_sidebar->selectPath(path);
            m_actSplit->setChecked(sm->isSplit());
            updateNavigationButtons();

            // Update free space
            QStorageInfo storage(path);
            if (storage.isValid() && storage.isReady()) {
                double freeGB = storage.bytesAvailable() / (1024.0 * 1024.0 * 1024.0);
                double totalGB = storage.bytesTotal() / (1024.0 * 1024.0 * 1024.0);
                m_spaceLabel->setText(QString("Free: %1 GB / %2 GB").arg(QString::number(freeGB, 'f', 1)).arg(QString::number(totalGB, 'f', 1)));
            }
        }
    }
}

void MainWindow::onBreadcrumbPathChanged(const QString& path) {
    navigateTo(path);
}

void MainWindow::onSidebarLocationSelected(const QString& path) {
    navigateTo(path);
}

void MainWindow::navigateTo(const QString& path) {
    int idx = m_tabBar->currentIndex();
    if (idx >= 0 && idx < m_tabSessions.size()) {
        SplitManager* sm = m_tabSessions[idx].splitManager;
        QString oldPath = sm->activeView()->currentPath();
        if (oldPath != path) {
            m_tabSessions[idx].backHistory.append(oldPath);
            m_tabSessions[idx].forwardHistory.clear();
        }
        sm->setPath(path);
        m_tabBar->setTabPath(idx, path);
        m_breadcrumbBar->setPath(path);
        m_sidebar->selectPath(path);
        updateNavigationButtons();
    }
}

void MainWindow::onActivePathChanged(const QString& path) {
    int idx = m_tabBar->currentIndex();
    if (idx >= 0) {
        m_tabBar->setTabPath(idx, path);
        m_breadcrumbBar->setPath(path);
        m_sidebar->selectPath(path);
        updateNavigationButtons();
    }
}

void MainWindow::onFileSelected(const QString& path) {
    if (m_previewDrawer->isVisible()) {
        m_previewDrawer->setFile(path);
    }
}

void MainWindow::onBack() {
    int idx = m_tabBar->currentIndex();
    if (idx >= 0 && idx < m_tabSessions.size()) {
        auto& session = m_tabSessions[idx];
        if (!session.backHistory.isEmpty()) {
            QString cur = session.splitManager->activeView()->currentPath();
            session.forwardHistory.append(cur);
            QString prev = session.backHistory.takeLast();
            session.splitManager->setPath(prev);
            m_tabBar->setTabPath(idx, prev);
            m_breadcrumbBar->setPath(prev);
            m_sidebar->selectPath(prev);
            updateNavigationButtons();
        }
    }
}

void MainWindow::onForward() {
    int idx = m_tabBar->currentIndex();
    if (idx >= 0 && idx < m_tabSessions.size()) {
        auto& session = m_tabSessions[idx];
        if (!session.forwardHistory.isEmpty()) {
            QString cur = session.splitManager->activeView()->currentPath();
            session.backHistory.append(cur);
            QString next = session.forwardHistory.takeLast();
            session.splitManager->setPath(next);
            m_tabBar->setTabPath(idx, next);
            m_breadcrumbBar->setPath(next);
            m_sidebar->selectPath(next);
            updateNavigationButtons();
        }
    }
}

void MainWindow::onUp() {
    SplitManager* sm = currentSplitManager();
    if (sm) {
        QDir dir(sm->activeView()->currentPath());
        if (dir.cdUp()) {
            navigateTo(dir.absolutePath());
        }
    }
}

void MainWindow::onHome() {
    navigateTo(QDir::homePath());
}

void MainWindow::onReload() {
    SplitManager* sm = currentSplitManager();
    if (sm) {
        sm->setPath(sm->activeView()->currentPath());
    }
}

void MainWindow::onToggleSplit() {
    SplitManager* sm = currentSplitManager();
    if (sm) {
        sm->toggleSplit();
        m_actSplit->setChecked(sm->isSplit());
    }
}

void MainWindow::onTogglePreview() {
    bool vis = !m_previewDrawer->isVisible();
    m_previewDrawer->setVisible(vis);
    m_actPreview->setChecked(vis);
    if (vis) {
        SplitManager* sm = currentSplitManager();
        if (sm) {
            QStringList sel = sm->activeView()->selectedPaths();
            if (!sel.isEmpty()) m_previewDrawer->setFile(sel.first());
            else m_previewDrawer->clearPreview();
        }
    }
}

void MainWindow::onToggleHidden() {
    bool next = !ConfigManager::instance().config().showHidden;
    ConfigManager::instance().config().showHidden = next;
    m_actHidden->setChecked(next);
    for (auto& session : m_tabSessions) {
        session.splitManager->setShowHidden(next);
    }
    m_statusLabel->setText(next ? "Hidden files: Visible" : "Hidden files: Hidden");
    ConfigManager::instance().save();
}

void MainWindow::onToggleSearch() {
    SplitManager* sm = currentSplitManager();
    if (sm) {
        sm->showSearch(true);
    }
}

void MainWindow::onOpenTerminal() {
    SplitManager* sm = currentSplitManager();
    if (sm) {
        EvaSuiteBridge::openInEvaTerm(sm->activeView()->currentPath());
    }
}

void MainWindow::onSortWithEvaSort() {
    SplitManager* sm = currentSplitManager();
    if (sm) {
        EvaSuiteBridge::sortWithEvaSort(sm->activeView()->currentPath(), this);
        onReload();
    }
}

void MainWindow::setDetailsView() {
    SplitManager* sm = currentSplitManager();
    if (sm) sm->setViewMode(FileView::ViewMode::Details);
}

void MainWindow::setIconsView() {
    SplitManager* sm = currentSplitManager();
    if (sm) sm->setViewMode(FileView::ViewMode::Icons);
}

void MainWindow::updateNavigationButtons() {
    int idx = m_tabBar->currentIndex();
    if (idx >= 0 && idx < m_tabSessions.size()) {
        m_actBack->setEnabled(!m_tabSessions[idx].backHistory.isEmpty());
        m_actForward->setEnabled(!m_tabSessions[idx].forwardHistory.isEmpty());
    }
}

void MainWindow::onOperationStarted(const QString& desc) {
    m_opLabel->setText(desc);
    m_opLabel->setVisible(true);
    m_opProgressBar->setValue(0);
    m_opProgressBar->setVisible(true);
}

void MainWindow::onOperationProgress(const OperationProgress& p) {
    m_opProgressBar->setValue(p.percent);
    if (!p.currentFile.isEmpty()) {
        m_opLabel->setText(QString("Processing %1 (%2%)").arg(p.currentFile).arg(p.percent));
    }
}

void MainWindow::onOperationFinished(bool success, const QString& message) {
    m_opProgressBar->setVisible(false);
    m_opLabel->setVisible(false);
    m_statusLabel->setText(message);
    onReload();
}

void MainWindow::onToggleSidebar() {
    bool vis = !m_sidebar->isVisible();
    m_sidebar->setVisible(vis);
    m_actSidebar->setChecked(vis);
    ConfigManager::instance().config().showSidebar = vis;
    ConfigManager::instance().save();
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    QMainWindow::keyPressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Save session
    QStringList openTabs;
    for (int i = 0; i < m_tabBar->count(); ++i) {
        openTabs.append(m_tabBar->tabPath(i));
    }
    auto& conf = ConfigManager::instance().config();
    conf.lastOpenedTabs = openTabs;
    conf.windowSize = size();
    conf.windowPos = pos();
    conf.showPreview = m_previewDrawer->isVisible();
    ConfigManager::instance().save();

    event->accept();
}
