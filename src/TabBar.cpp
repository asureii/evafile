#include "TabBar.hpp"
#include <QIcon>
#include <QFileInfo>
#include <QDir>

TabBar::TabBar(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(36);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 0, 4, 0);
    layout->setSpacing(2);

    m_tabBar = new QTabBar(this);
    m_tabBar->setTabsClosable(true);
    m_tabBar->setMovable(true);
    m_tabBar->setExpanding(false);
    m_tabBar->setDrawBase(false);
    m_tabBar->setElideMode(Qt::ElideRight);
    m_tabBar->setUsesScrollButtons(true);

    m_addTabButton = new QToolButton(this);
    m_addTabButton->setIcon(QIcon(":/icons/add-tab.svg"));
    m_addTabButton->setToolTip("New Tab (Ctrl+T)");
    m_addTabButton->setFixedSize(26, 26);

    layout->addWidget(m_tabBar);
    layout->addWidget(m_addTabButton);
    layout->addStretch(1);

    connect(m_tabBar, &QTabBar::currentChanged, this, &TabBar::onTabCurrentChanged);
    connect(m_tabBar, &QTabBar::tabCloseRequested, this, &TabBar::onTabClose);
    connect(m_addTabButton, &QToolButton::clicked, this, &TabBar::newTabRequested);
}

int TabBar::addTab(const QString& title, const QString& path) {
    int idx = m_tabBar->addTab(QIcon(":/icons/folder.svg"), title);
    m_tabBar->setTabData(idx, path);
    return idx;
}

void TabBar::setTabText(int index, const QString& text) {
    if (index >= 0 && index < m_tabBar->count()) {
        m_tabBar->setTabText(index, text);
    }
}

void TabBar::setTabPath(int index, const QString& path) {
    if (index >= 0 && index < m_tabBar->count()) {
        m_tabBar->setTabData(index, path);
        QFileInfo fi(path);
        QString name = fi.fileName();
        if (name.isEmpty() || path == "/") name = "/";
        else if (path == QDir::homePath()) name = "Home";
        m_tabBar->setTabText(index, name);
    }
}

QString TabBar::tabPath(int index) const {
    if (index >= 0 && index < m_tabBar->count()) {
        return m_tabBar->tabData(index).toString();
    }
    return QString();
}

int TabBar::currentIndex() const {
    return m_tabBar->currentIndex();
}

void TabBar::setCurrentIndex(int index) {
    m_tabBar->setCurrentIndex(index);
}

int TabBar::count() const {
    return m_tabBar->count();
}

void TabBar::removeTab(int index) {
    m_tabBar->removeTab(index);
}

void TabBar::onTabClose(int index) {
    emit tabCloseRequested(index);
}

void TabBar::onTabCurrentChanged(int index) {
    emit currentChanged(index);
}
