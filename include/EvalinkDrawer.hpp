#pragma once

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QProgressBar>
#include "EvalinkManager.hpp"

class EvalinkDrawer : public QWidget {
    Q_OBJECT
public:
    explicit EvalinkDrawer(QWidget* parent = nullptr);

signals:
    void openFolderRequested(const QString& folderPath);
    void newDownloadRequested();

public slots:
    void updateTasks(const QList<EvalinkTask>& tasks, qint64 globalSpeed, int activeCount);

private slots:
    void onNewDownloadClicked();
    void onPurgeClicked();
    void onOpenTerminalWatchClicked();

private:
    QWidget* createItemWidget(const EvalinkTask& task);

    QLabel* m_titleLabel;
    QLabel* m_speedBadge;
    QPushButton* m_addBtn;
    QPushButton* m_purgeBtn;
    QPushButton* m_watchBtn;
    QPushButton* m_closeBtn;
    QVBoxLayout* m_itemsLayout;
    QLabel* m_emptyLabel;
};
