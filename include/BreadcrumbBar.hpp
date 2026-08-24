#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QCompleter>
#include <QFileSystemModel>

class BreadcrumbBar : public QWidget {
    Q_OBJECT
public:
    explicit BreadcrumbBar(QWidget* parent = nullptr);

    QString currentPath() const { return m_currentPath; }
    void setPath(const QString& path);

    void focusAddressBar();

signals:
    void pathChanged(const QString& path);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onSegmentClicked();
    void onEditFinished();

private:
    void updateBreadcrumbs();
    void switchToEditMode();
    void switchToPillMode();

    QString m_currentPath;
    QWidget* m_pillContainer;
    QHBoxLayout* m_pillLayout;
    QLineEdit* m_pathEdit;
    QCompleter* m_completer;
    QFileSystemModel* m_completerModel;
    bool m_isEditMode = false;
};
