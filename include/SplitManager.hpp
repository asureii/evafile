#pragma once

#include <QWidget>
#include <QSplitter>
#include <QHBoxLayout>
#include "FileView.hpp"

class SplitManager : public QWidget {
    Q_OBJECT
public:
    explicit SplitManager(QWidget* parent = nullptr);

    bool isSplit() const { return m_isSplit; }
    void setSplit(bool split);
    void toggleSplit();

    FileView* activeView() const { return m_activeView; }
    FileView* leftView() const { return m_leftView; }
    FileView* rightView() const { return m_rightView; }

    void setPath(const QString& path);
    void setViewMode(FileView::ViewMode mode);
    void setShowHidden(bool show);
    void showSearch(bool show);

signals:
    void activePathChanged(const QString& path);
    void activeFileSelected(const QString& path);
    void statusMessageChanged(const QString& msg);
    void splitStateChanged(bool isSplit);

private slots:
    void onLeftPathChanged(const QString& path);
    void onRightPathChanged(const QString& path);

private:
    void setActiveView(FileView* view);

    bool m_isSplit = false;
    QSplitter* m_splitter;
    FileView* m_leftView;
    FileView* m_rightView;
    FileView* m_activeView;
};
