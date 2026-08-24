#include "SplitManager.hpp"
#include <QDir>
#include <QFocusEvent>

SplitManager::SplitManager(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setChildrenCollapsible(false);

    m_leftView = new FileView(this);
    m_rightView = new FileView(this);
    m_rightView->setVisible(false);

    m_splitter->addWidget(m_leftView);
    m_splitter->addWidget(m_rightView);

    layout->addWidget(m_splitter);

    m_activeView = m_leftView;

    // Connect left view signals
    connect(m_leftView, &FileView::pathChanged, this, &SplitManager::onLeftPathChanged);
    connect(m_leftView, &FileView::fileSelected, this, [this](const QString& p) {
        if (m_activeView == m_leftView) emit activeFileSelected(p);
    });
    connect(m_leftView, &FileView::statusMessageChanged, this, [this](const QString& msg) {
        if (m_activeView == m_leftView) emit statusMessageChanged(msg);
    });

    // Connect right view signals
    connect(m_rightView, &FileView::pathChanged, this, &SplitManager::onRightPathChanged);
    connect(m_rightView, &FileView::fileSelected, this, [this](const QString& p) {
        if (m_activeView == m_rightView) emit activeFileSelected(p);
    });
    connect(m_rightView, &FileView::statusMessageChanged, this, [this](const QString& msg) {
        if (m_activeView == m_rightView) emit statusMessageChanged(msg);
    });

    // Handle view focus/selection to update active view
    connect(m_leftView, &FileView::pathChanged, this, [this]() { setActiveView(m_leftView); });
    connect(m_rightView, &FileView::pathChanged, this, [this]() { setActiveView(m_rightView); });
}

void SplitManager::setActiveView(FileView* view) {
    if (m_activeView != view) {
        m_activeView = view;
        emit activePathChanged(view->currentPath());
    }
}

void SplitManager::setSplit(bool split) {
    m_isSplit = split;
    m_rightView->setVisible(split);
    if (split) {
        if (m_rightView->currentPath() == m_leftView->currentPath()) {
            m_rightView->setPath(m_leftView->currentPath());
        }
        m_splitter->setSizes({width() / 2, width() / 2});
    } else {
        m_activeView = m_leftView;
        emit activePathChanged(m_leftView->currentPath());
    }
    emit splitStateChanged(split);
}

void SplitManager::toggleSplit() {
    setSplit(!m_isSplit);
}

void SplitManager::setPath(const QString& path) {
    if (m_activeView) {
        m_activeView->setPath(path);
    } else {
        m_leftView->setPath(path);
    }
}

void SplitManager::setViewMode(FileView::ViewMode mode) {
    m_leftView->setViewMode(mode);
    m_rightView->setViewMode(mode);
}

void SplitManager::setShowHidden(bool show) {
    m_leftView->setShowHidden(show);
    m_rightView->setShowHidden(show);
}

void SplitManager::showSearch(bool show) {
    if (m_activeView) {
        m_activeView->showSearch(show);
    }
}

void SplitManager::onLeftPathChanged(const QString& path) {
    if (m_activeView == m_leftView) {
        emit activePathChanged(path);
    }
}

void SplitManager::onRightPathChanged(const QString& path) {
    if (m_activeView == m_rightView) {
        emit activePathChanged(path);
    }
}
