#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QTreeView>
#include <QListView>
#include <QFileSystemModel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QFileIconProvider>
#include <QSortFilterProxyModel>

#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

class EvaIconProvider : public QFileIconProvider {
public:
    EvaIconProvider();
    QIcon icon(const QFileInfo& info) const override;
    QIcon icon(IconType type) const override;
};

class EvaItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit EvaItemDelegate(QObject* parent = nullptr);

protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
};

class FileView;

class EvaTreeView : public QTreeView {
    Q_OBJECT
public:
    explicit EvaTreeView(FileView* fileView, QWidget* parent = nullptr);

protected:
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    FileView* m_fileView;
};

class EvaListView : public QListView {
    Q_OBJECT
public:
    explicit EvaListView(FileView* fileView, QWidget* parent = nullptr);

protected:
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    FileView* m_fileView;
};

class FileView : public QWidget {
    Q_OBJECT
public:
    enum class ViewMode { Details, Icons, Compact };

    explicit FileView(QWidget* parent = nullptr);

    QString currentPath() const { return m_currentPath; }
    void setPath(const QString& path);
    void setViewMode(ViewMode mode);
    ViewMode viewMode() const { return m_viewMode; }

    void setShowHidden(bool show);
    bool showHidden() const { return m_showHidden; }

    QStringList selectedPaths() const;
    void showSearch(bool show);

    void promptCreateFolder();
    void promptCreateFile(const QString& defaultName = "untitled.txt", const QString& templateContent = QString());

    QFileSystemModel* model() const { return m_model; }

signals:
    void pathChanged(const QString& path);
    void fileSelected(const QString& path);
    void fileActivated(const QString& path);
    void statusMessageChanged(const QString& msg);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onItemDoubleClicked(const QModelIndex& index);
    void onSelectionChanged();
    void onSearchTextChanged(const QString& text);

private:
    void setupModels();
    void setupViews();
    void updateStatus();

    QString m_currentPath;
    ViewMode m_viewMode = ViewMode::Details;
    bool m_showHidden = false;

    QFileSystemModel* m_model;
    EvaIconProvider* m_iconProvider;
    EvaTreeView* m_treeView;
    EvaListView* m_listView;
    QStackedWidget* m_viewStack;
    QLineEdit* m_searchEdit;

    // Clipboard for Copy / Cut
    static QStringList s_clipboardPaths;
    static bool s_isCutOperation;
};
