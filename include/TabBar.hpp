#pragma once

#include <QWidget>
#include <QTabBar>
#include <QHBoxLayout>
#include <QToolButton>

class TabBar : public QWidget {
    Q_OBJECT
public:
    explicit TabBar(QWidget* parent = nullptr);

    int addTab(const QString& title, const QString& path);
    void setTabText(int index, const QString& text);
    void setTabPath(int index, const QString& path);
    QString tabPath(int index) const;

    int currentIndex() const;
    void setCurrentIndex(int index);
    int count() const;
    void removeTab(int index);

signals:
    void currentChanged(int index);
    void tabCloseRequested(int index);
    void newTabRequested();

private slots:
    void onTabClose(int index);
    void onTabCurrentChanged(int index);

private:
    QTabBar* m_tabBar;
    QToolButton* m_addTabButton;
};
