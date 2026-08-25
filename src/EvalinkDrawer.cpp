#include "EvalinkDrawer.hpp"
#include "EvaSuiteBridge.hpp"
#include <QScrollArea>
#include <QHBoxLayout>
#include <QToolButton>
#include <QFileInfo>
#include <QDir>

EvalinkDrawer::EvalinkDrawer(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("EvalinkDrawer");
    setMinimumWidth(280);
    setMaximumWidth(420);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // Header card
    QWidget* headerWidget = new QWidget(this);
    headerWidget->setStyleSheet("background-color: #360e10; border: 1px solid #5a1e22; border-radius: 6px; padding: 4px;");
    QVBoxLayout* headerVBox = new QVBoxLayout(headerWidget);
    headerVBox->setContentsMargins(8, 8, 8, 8);
    headerVBox->setSpacing(6);

    QHBoxLayout* topHeader = new QHBoxLayout();
    topHeader->setContentsMargins(0, 0, 0, 0);

    QLabel* iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/icons/downloads.svg").pixmap(20, 20));
    m_titleLabel = new QLabel("Evalink Downloads", this);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #ebdada;");

    m_speedBadge = new QLabel("0 B/s", this);
    m_speedBadge->setStyleSheet("background-color: #25090a; color: #ff7373; font-weight: bold; font-size: 11px; padding: 2px 6px; border-radius: 4px; border: 1px solid #5a1e22;");

    m_closeBtn = new QPushButton("✕", this);
    m_closeBtn->setFixedSize(22, 22);
    m_closeBtn->setStyleSheet("background: transparent; color: #b58487; font-weight: bold; border: none;");
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() { setVisible(false); });

    topHeader->addWidget(iconLabel);
    topHeader->addWidget(m_titleLabel);
    topHeader->addStretch(1);
    topHeader->addWidget(m_speedBadge);
    topHeader->addWidget(m_closeBtn);
    headerVBox->addLayout(topHeader);

    // Toolbar inside header
    QHBoxLayout* actionHeader = new QHBoxLayout();
    actionHeader->setContentsMargins(0, 0, 0, 0);
    actionHeader->setSpacing(4);

    m_addBtn = new QPushButton("+ Add", this);
    m_addBtn->setStyleSheet("background-color: #cf2824; color: #ffffff; font-weight: bold; border-radius: 4px; padding: 4px 10px; font-size: 11px;");
    connect(m_addBtn, &QPushButton::clicked, this, &EvalinkDrawer::onNewDownloadClicked);

    m_purgeBtn = new QPushButton("Purge Done", this);
    m_purgeBtn->setStyleSheet("background-color: #25090a; color: #b58487; border: 1px solid #5a1e22; border-radius: 4px; padding: 4px 8px; font-size: 11px;");
    connect(m_purgeBtn, &QPushButton::clicked, this, &EvalinkDrawer::onPurgeClicked);

    m_watchBtn = new QPushButton("Watch TUI", this);
    m_watchBtn->setStyleSheet("background-color: #25090a; color: #ff7373; border: 1px solid #5a1e22; border-radius: 4px; padding: 4px 8px; font-size: 11px;");
    connect(m_watchBtn, &QPushButton::clicked, this, &EvalinkDrawer::onOpenTerminalWatchClicked);

    actionHeader->addWidget(m_addBtn);
    actionHeader->addWidget(m_purgeBtn);
    actionHeader->addWidget(m_watchBtn);
    actionHeader->addStretch(1);
    headerVBox->addLayout(actionHeader);

    mainLayout->addWidget(headerWidget);

    // Scroll Area for items
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("background: transparent;");

    QWidget* scrollContent = new QWidget(scrollArea);
    scrollContent->setStyleSheet("background: transparent;");
    m_itemsLayout = new QVBoxLayout(scrollContent);
    m_itemsLayout->setContentsMargins(0, 0, 0, 0);
    m_itemsLayout->setSpacing(8);
    m_itemsLayout->setAlignment(Qt::AlignTop);

    m_emptyLabel = new QLabel("No active or recent downloads.\nClick '+ Add' to start downloading with Evalink.", scrollContent);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: #8a585c; font-size: 12px; padding: 20px 10px;");
    m_itemsLayout->addWidget(m_emptyLabel);

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);

    // Connect to EvalinkManager
    connect(&EvalinkManager::instance(), &EvalinkManager::tasksUpdated, this, &EvalinkDrawer::updateTasks);
}

void EvalinkDrawer::onNewDownloadClicked() {
    emit newDownloadRequested();
}

void EvalinkDrawer::onPurgeClicked() {
    EvalinkManager::instance().purgeCompleted();
}

void EvalinkDrawer::onOpenTerminalWatchClicked() {
    EvaSuiteBridge::openInEvaTerm(QDir::homePath());
}

QWidget* EvalinkDrawer::createItemWidget(const EvalinkTask& task) {
    QWidget* card = new QWidget(this);
    card->setStyleSheet("background-color: #360e10; border: 1px solid #5a1e22; border-radius: 6px; padding: 6px;");

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(4);

    // Title line
    QHBoxLayout* titleLine = new QHBoxLayout();
    titleLine->setContentsMargins(0, 0, 0, 0);

    QLabel* nameLabel = new QLabel(task.name, card);
    nameLabel->setStyleSheet("font-weight: bold; font-size: 12px; color: #ebdada;");
    nameLabel->setToolTip(task.name + "\nDirectory: " + task.dir);

    QString statusColor = "#b58487";
    if (task.status == "active") statusColor = "#e32a10";
    else if (task.status == "complete") statusColor = "#55b870";
    else if (task.status == "paused") statusColor = "#ffc85a";
    else if (task.status == "error") statusColor = "#ff4444";

    QLabel* statusBadge = new QLabel(task.status.toUpper(), card);
    statusBadge->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 10px;").arg(statusColor));

    titleLine->addWidget(nameLabel, 1);
    titleLine->addWidget(statusBadge);
    layout->addLayout(titleLine);

    // Progress bar
    QProgressBar* pbar = new QProgressBar(card);
    pbar->setRange(0, 100);
    pbar->setValue(task.percent);
    pbar->setFixedHeight(6);
    pbar->setTextVisible(false);
    pbar->setStyleSheet(
        "QProgressBar { background-color: #25090a; border-radius: 3px; border: none; }"
        "QProgressBar::chunk { background-color: #cf2824; border-radius: 3px; }"
    );
    layout->addWidget(pbar);

    // Details Line
    QHBoxLayout* detailsLine = new QHBoxLayout();
    detailsLine->setContentsMargins(0, 0, 0, 0);

    QString doneStr = EvalinkManager::formatBytes(task.completedBytes);
    QString totalStr = (task.totalBytes > 0) ? EvalinkManager::formatBytes(task.totalBytes) : "?";
    QString speedStr = (task.status == "active") ? (" • " + EvalinkManager::formatSpeed(task.downloadSpeed)) : "";
    QString etaStr = (task.status == "active" && task.eta != "--") ? (" • ETA: " + task.eta) : "";

    QLabel* infoLabel = new QLabel(QString("%1 / %2 (%3%)%4%5").arg(doneStr).arg(totalStr).arg(task.percent).arg(speedStr).arg(etaStr), card);
    infoLabel->setStyleSheet("color: #b58487; font-size: 11px;");
    detailsLine->addWidget(infoLabel, 1);

    // Control buttons
    if (task.status == "active") {
        QPushButton* pauseBtn = new QPushButton("⏸", card);
        pauseBtn->setFixedSize(22, 22);
        pauseBtn->setStyleSheet("background-color: #25090a; color: #ebdada; border: 1px solid #5a1e22; border-radius: 3px; font-size: 10px;");
        QString gid = task.gid;
        connect(pauseBtn, &QPushButton::clicked, [gid]() { EvalinkManager::instance().pauseDownload(gid); });
        detailsLine->addWidget(pauseBtn);
    } else if (task.status == "paused") {
        QPushButton* resumeBtn = new QPushButton("▶", card);
        resumeBtn->setFixedSize(22, 22);
        resumeBtn->setStyleSheet("background-color: #25090a; color: #55b870; border: 1px solid #5a1e22; border-radius: 3px; font-size: 10px;");
        QString gid = task.gid;
        connect(resumeBtn, &QPushButton::clicked, [gid]() { EvalinkManager::instance().resumeDownload(gid); });
        detailsLine->addWidget(resumeBtn);
    }

    QPushButton* cancelBtn = new QPushButton("✕", card);
    cancelBtn->setFixedSize(22, 22);
    cancelBtn->setStyleSheet("background-color: #25090a; color: #ff7373; border: 1px solid #5a1e22; border-radius: 3px; font-size: 10px;");
    QString gid = task.gid;
    connect(cancelBtn, &QPushButton::clicked, [gid]() { EvalinkManager::instance().cancelDownload(gid); });
    detailsLine->addWidget(cancelBtn);

    if (!task.dir.isEmpty()) {
        QPushButton* folderBtn = new QPushButton(card);
        folderBtn->setIcon(QIcon(":/icons/folder.svg"));
        folderBtn->setFixedSize(22, 22);
        folderBtn->setStyleSheet("background-color: #25090a; border: 1px solid #5a1e22; border-radius: 3px;");
        QString dir = task.dir;
        connect(folderBtn, &QPushButton::clicked, [this, dir]() { emit openFolderRequested(dir); });
        detailsLine->addWidget(folderBtn);
    }

    layout->addLayout(detailsLine);
    return card;
}

void EvalinkDrawer::updateTasks(const QList<EvalinkTask>& tasks, qint64 globalSpeed, int activeCount) {
    m_speedBadge->setText(EvalinkManager::formatSpeed(globalSpeed));
    if (activeCount > 0) {
        m_speedBadge->setStyleSheet("background-color: #cf2824; color: #ffffff; font-weight: bold; font-size: 11px; padding: 2px 6px; border-radius: 4px;");
    } else {
        m_speedBadge->setStyleSheet("background-color: #25090a; color: #ff7373; font-weight: bold; font-size: 11px; padding: 2px 6px; border-radius: 4px; border: 1px solid #5a1e22;");
    }

    // Clear old items
    QLayoutItem* item;
    while ((item = m_itemsLayout->takeAt(0)) != nullptr) {
        if (item->widget() && item->widget() != m_emptyLabel) {
            delete item->widget();
        }
        delete item;
    }

    if (tasks.isEmpty()) {
        m_itemsLayout->addWidget(m_emptyLabel);
        m_emptyLabel->setVisible(true);
    } else {
        m_emptyLabel->setVisible(false);
        for (const auto& task : tasks) {
            m_itemsLayout->addWidget(createItemWidget(task));
        }
    }
}
