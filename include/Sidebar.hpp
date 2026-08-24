#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QStorageInfo>

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget* parent = nullptr);

    void refresh();
    void selectPath(const QString& path);

signals:
    void locationSelected(const QString& path);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);

private:
    void setupPlaces();
    void setupDrives();
    void setupBookmarks();
    QTreeWidgetItem* addCategory(const QString& title);
    void addPlaceItem(QTreeWidgetItem* category, const QString& name, const QString& path, const QString& iconPath);

    QTreeWidget* m_tree;
    QTreeWidgetItem* m_placesCategory;
    QTreeWidgetItem* m_drivesCategory;
    QTreeWidgetItem* m_bookmarksCategory;
};
