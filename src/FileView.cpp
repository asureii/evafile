#include "FileView.hpp"
#include "FileOperations.hpp"
#include "EvaSuiteBridge.hpp"
#include "EvalinkDialog.hpp"
#include "EvalinkManager.hpp"
#include "Config.hpp"
#include <QHeaderView>
#include <QDir>
#include <QFileInfo>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>

QStringList FileView::s_clipboardPaths;
bool FileView::s_isCutOperation = false;

EvaIconProvider::EvaIconProvider() {}

QIcon EvaIconProvider::icon(const QFileInfo& info) const {
    if (info.isDir()) {
        if (info.fileName().startsWith(".")) {
            return QIcon(":/icons/folder-hidden.svg");
        }
        return QIcon(":/icons/folder.svg");
    }

    QString ext = info.suffix().toLower();
    if (ext == "cpp" || ext == "hpp" || ext == "c" || ext == "h" || ext == "rs" || ext == "py" || ext == "js" || ext == "ts" || ext == "json" || ext == "html" || ext == "css" || ext == "sh" || ext == "cmake" || ext == "toml" || ext == "yaml" || ext == "yml") {
        return QIcon(":/icons/file-code.svg");
    }
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "gif" || ext == "webp" || ext == "svg" || ext == "bmp" || ext == "ico" || ext == "avif") {
        return QIcon(":/icons/file-image.svg");
    }
    if (ext == "pdf") {
        return QIcon(":/icons/file-pdf.svg");
    }
    if (ext == "mp3" || ext == "flac" || ext == "wav" || ext == "ogg" || ext == "m4a" || ext == "opus" || ext == "aac") {
        return QIcon(":/icons/file-audio.svg");
    }
    if (ext == "mp4" || ext == "mkv" || ext == "avi" || ext == "mov" || ext == "webm" || ext == "flv") {
        return QIcon(":/icons/file-video.svg");
    }
    if (ext == "zip" || ext == "tar" || ext == "gz" || ext == "xz" || ext == "bz2" || ext == "7z" || ext == "rar" || ext == "zst") {
        return QIcon(":/icons/file-archive.svg");
    }

    return QIcon(":/icons/file.svg");
}

QIcon EvaIconProvider::icon(IconType type) const {
    if (type == Folder) return QIcon(":/icons/folder.svg");
    if (type == Trashcan) return QIcon(":/icons/trash.svg");
    if (type == Drive) return QIcon(":/icons/disk.svg");
    return QIcon(":/icons/file.svg");
}

EvaItemDelegate::EvaItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void EvaItemDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
    QStyledItemDelegate::initStyleOption(option, index);

    const QFileSystemModel* fsModel = qobject_cast<const QFileSystemModel*>(index.model());
    if (!fsModel) return;

    QString fileName = fsModel->fileName(index);
    bool isDir = fsModel->isDir(index);

    if (option->state & QStyle::State_Selected) {
        option->palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
        return;
    }

    if (isDir) {
        if (fileName.startsWith(".")) {
            // Hidden directory: Warm yellow / amber
            option->palette.setColor(QPalette::Text, QColor("#ffc85a"));
        } else {
            // Normal directory: Bright red / crimson
            option->palette.setColor(QPalette::Text, QColor("#ff7373"));
        }
    } else if (fileName.startsWith(".")) {
        // Hidden file: Muted gray-pink
        option->palette.setColor(QPalette::Text, QColor("#b58487"));
    }
}

FileView::FileView(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    // Search filter input
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Filter files in current folder... (Press Esc to close)");
    m_searchEdit->setVisible(false);
    layout->addWidget(m_searchEdit);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &FileView::onSearchTextChanged);

    m_viewStack = new QStackedWidget(this);
    layout->addWidget(m_viewStack);

    setupModels();
    setupViews();

    setPath(QDir::homePath());
}

void FileView::setupModels() {
    m_model = new QFileSystemModel(this);
    m_iconProvider = new EvaIconProvider();
    m_model->setIconProvider(m_iconProvider);
    m_model->setRootPath("/");
    m_model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
}

void FileView::setupViews() {
    // 1. Details Tree View
    m_treeView = new QTreeView(this);
    m_treeView->setModel(m_model);
    m_treeView->setItemDelegate(new EvaItemDelegate(m_treeView));
    m_treeView->setSortingEnabled(true);
    m_treeView->sortByColumn(0, Qt::AscendingOrder);
    m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_treeView->setAllColumnsShowFocus(true);
    m_treeView->setAnimated(false);
    m_treeView->setIndentation(16);
    m_treeView->setUniformRowHeights(true);
    m_treeView->header()->setStretchLastSection(false);
    m_treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_treeView->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    // 2. Icons Grid View
    m_listView = new QListView(this);
    m_listView->setModel(m_model);
    m_listView->setItemDelegate(new EvaItemDelegate(m_listView));
    m_listView->setViewMode(QListView::IconMode);
    m_listView->setIconSize(QSize(64, 64));
    m_listView->setGridSize(QSize(100, 95));
    m_listView->setSpacing(8);
    m_listView->setResizeMode(QListView::Adjust);
    m_listView->setMovement(QListView::Static);
    m_listView->setWordWrap(true);
    m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_listView->setUniformItemSizes(true);

    m_viewStack->addWidget(m_treeView);
    m_viewStack->addWidget(m_listView);

    connect(m_treeView, &QTreeView::doubleClicked, this, &FileView::onItemDoubleClicked);
    connect(m_listView, &QListView::doubleClicked, this, &FileView::onItemDoubleClicked);

    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileView::onSelectionChanged);
    connect(m_listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileView::onSelectionChanged);
}

void FileView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    int w = width();
    if (w < 420) {
        m_treeView->setColumnHidden(2, true);  // Hide Type
        m_treeView->setColumnHidden(3, true);  // Hide Date Modified
    } else if (w < 580) {
        m_treeView->setColumnHidden(2, true);  // Hide Type
        m_treeView->setColumnHidden(3, false); // Show Date Modified
    } else {
        m_treeView->setColumnHidden(2, false); // Show Type
        m_treeView->setColumnHidden(3, false); // Show Date Modified
    }
}

void FileView::setPath(const QString& path) {
    QString clean = QDir::cleanPath(path);
    if (!QDir(clean).exists()) return;

    m_currentPath = clean;
    QModelIndex rootIdx = m_model->setRootPath(clean);
    m_treeView->setRootIndex(rootIdx);
    m_listView->setRootIndex(rootIdx);

    updateStatus();
    emit pathChanged(clean);
}

void FileView::setViewMode(ViewMode mode) {
    m_viewMode = mode;
    if (mode == ViewMode::Details) {
        m_viewStack->setCurrentWidget(m_treeView);
    } else {
        m_viewStack->setCurrentWidget(m_listView);
    }
}

void FileView::setShowHidden(bool show) {
    m_showHidden = show;
    QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot;
    if (show) filters |= QDir::Hidden;
    m_model->setFilter(filters);
}

void FileView::showSearch(bool show) {
    m_searchEdit->setVisible(show);
    if (show) {
        m_searchEdit->clear();
        m_searchEdit->setFocus();
    } else {
        m_model->setNameFilters(QStringList());
        m_model->setNameFilterDisables(false);
    }
}

void FileView::onSearchTextChanged(const QString& text) {
    if (text.trimmed().isEmpty()) {
        m_model->setNameFilters(QStringList());
        m_model->setNameFilterDisables(false);
    } else {
        QStringList filters;
        filters << QString("*%1*").arg(text.trimmed());
        m_model->setNameFilters(filters);
        m_model->setNameFilterDisables(false);
    }
}

QStringList FileView::selectedPaths() const {
    QStringList paths;
    QItemSelectionModel* selModel = (m_viewMode == ViewMode::Details) ? m_treeView->selectionModel() : m_listView->selectionModel();
    const QModelIndexList indexes = selModel->selectedRows(0);
    for (const QModelIndex& idx : indexes) {
        paths.append(m_model->filePath(idx));
    }
    return paths;
}

void FileView::onItemDoubleClicked(const QModelIndex& index) {
    QString path = m_model->filePath(index);
    if (m_model->isDir(index)) {
        setPath(path);
    } else {
        emit fileActivated(path);
        EvaSuiteBridge::openWithDefaultApp(path);
    }
}

void FileView::onSelectionChanged() {
    QStringList paths = selectedPaths();
    if (paths.size() == 1) {
        emit fileSelected(paths.first());
    } else if (paths.isEmpty()) {
        emit fileSelected(QString());
    }
    updateStatus();
}

void FileView::updateStatus() {
    QDir dir(m_currentPath);
    QFileInfoList list = dir.entryInfoList(m_showHidden ? (QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden) : (QDir::AllEntries | QDir::NoDotAndDotDot));

    QStringList sel = selectedPaths();
    if (sel.isEmpty()) {
        emit statusMessageChanged(QString("%1 items in %2").arg(list.size()).arg(dir.dirName().isEmpty() ? "/" : dir.dirName()));
    } else {
        emit statusMessageChanged(QString("%1 of %2 items selected").arg(sel.size()).arg(list.size()));
    }
}

void FileView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QStringList sel = selectedPaths();
        if (!sel.isEmpty()) {
            QFileInfo fi(sel.first());
            if (fi.isDir()) setPath(fi.absoluteFilePath());
            else EvaSuiteBridge::openWithDefaultApp(fi.absoluteFilePath());
        }
    } else if (event->key() == Qt::Key_Backspace || (event->modifiers() == Qt::AltModifier && event->key() == Qt::Key_Up)) {
        QDir dir(m_currentPath);
        if (dir.cdUp()) {
            setPath(dir.absolutePath());
        }
    } else if (event->key() == Qt::Key_Delete) {
        QStringList sel = selectedPaths();
        if (!sel.isEmpty()) {
            if (event->modifiers() & Qt::ShiftModifier) {
                if (QMessageBox::question(this, "Permanent Delete",
                    QString("Permanently delete %1 selected item(s)?").arg(sel.size())) == QMessageBox::Yes) {
                    FileOperations::instance().permanentDelete(sel);
                }
            } else {
                FileOperations::instance().trash(sel);
            }
        }
    } else if (event->key() == Qt::Key_F2) {
        QStringList sel = selectedPaths();
        if (sel.size() == 1) {
            QFileInfo fi(sel.first());
            bool ok = false;
            QString newName = QInputDialog::getText(this, "Rename Item", "New name:", QLineEdit::Normal, fi.fileName(), &ok);
            if (ok && !newName.isEmpty() && newName != fi.fileName()) {
                FileOperations::instance().rename(fi.absoluteFilePath(), newName);
            }
        }
    } else if (event->matches(QKeySequence::Copy)) {
        s_clipboardPaths = selectedPaths();
        s_isCutOperation = false;
    } else if (event->matches(QKeySequence::Cut)) {
        s_clipboardPaths = selectedPaths();
        s_isCutOperation = true;
    } else if (event->matches(QKeySequence::Paste)) {
        if (!s_clipboardPaths.isEmpty()) {
            if (s_isCutOperation) {
                FileOperations::instance().move(s_clipboardPaths, m_currentPath);
                s_clipboardPaths.clear();
            } else {
                FileOperations::instance().copy(s_clipboardPaths, m_currentPath);
            }
        }
    } else if (event->key() == Qt::Key_C && (event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier)) {
        QStringList sel = selectedPaths();
        QString text = sel.isEmpty() ? m_currentPath : sel.join("\n");
        QGuiApplication::clipboard()->setText(text);
        emit statusMessageChanged(QString("Location copied: %1").arg(text));
    } else if (event->key() == Qt::Key_N && (event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier)) {
        promptCreateFolder();
    } else if (event->key() == Qt::Key_N && (event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::AltModifier)) {
        promptCreateFile();
    } else if (event->key() == Qt::Key_Escape && m_searchEdit->isVisible()) {
        showSearch(false);
    } else {
        QWidget::keyPressEvent(event);
    }
}

void FileView::promptCreateFolder() {
    QString defaultName = "New Folder";
    if (QDir(m_currentPath + "/" + defaultName).exists()) {
        int i = 1;
        while (QDir(QString("%1/New Folder (%2)").arg(m_currentPath).arg(i)).exists()) {
            i++;
        }
        defaultName = QString("New Folder (%1)").arg(i);
    }

    bool ok = false;
    QString name = QInputDialog::getText(this, "Create Folder", "Enter folder name:", QLineEdit::Normal, defaultName, &ok);
    if (ok && !name.trimmed().isEmpty()) {
        FileOperations::instance().createDirectory(m_currentPath, name.trimmed());
    }
}

void FileView::promptCreateFile(const QString& defaultName, const QString& templateContent) {
    QString candidate = defaultName;
    if (QFile::exists(m_currentPath + "/" + candidate)) {
        QFileInfo fi(candidate);
        QString base = fi.completeBaseName();
        QString ext = fi.suffix();
        int i = 1;
        do {
            if (ext.isEmpty()) candidate = QString("%1 (%2)").arg(base).arg(i);
            else candidate = QString("%1 (%2).%3").arg(base).arg(i).arg(ext);
            i++;
        } while (QFile::exists(m_currentPath + "/" + candidate));
    }

    bool ok = false;
    QString name = QInputDialog::getText(this, "Create File", "Enter file name:", QLineEdit::Normal, candidate, &ok);
    if (ok && !name.trimmed().isEmpty()) {
        FileOperations::instance().createFileWithContent(m_currentPath, name.trimmed(), templateContent);
    }
}

void FileView::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu(this);
    QStringList sel = selectedPaths();

    if (!sel.isEmpty()) {
        QFileInfo firstFi(sel.first());
        bool isSingleZip = (sel.size() == 1) && firstFi.isFile() && (firstFi.suffix().compare("zip", Qt::CaseInsensitive) == 0);

        // Selection specific actions
        menu.addAction(QIcon(":/icons/folder-open.svg"), "Open", [this, sel]() {
            if (QFileInfo(sel.first()).isDir()) setPath(sel.first());
            else EvaSuiteBridge::openWithDefaultApp(sel.first());
        });

        if (EvaSuiteBridge::isEvaTermAvailable() || true) {
            menu.addAction(QIcon(":/icons/terminal.svg"), "Open in EvaTerm", [sel, this]() {
                QString dir = QFileInfo(sel.first()).isDir() ? sel.first() : m_currentPath;
                EvaSuiteBridge::openInEvaTerm(dir);
            });
        }

        if (EvaSuiteBridge::isEvaSortAvailable()) {
            menu.addAction(QIcon(":/icons/sort.svg"), "Smart Sort with EvaSort", [sel, this]() {
                QString dir = QFileInfo(sel.first()).isDir() ? sel.first() : m_currentPath;
                EvaSuiteBridge::sortWithEvaSort(dir, this);
            });
        }

        // ZIP Extract / Compress Actions
        if (isSingleZip) {
            menu.addSeparator();
            menu.addAction(QIcon(":/icons/file-archive.svg"), "Extract Here", [this, sel]() {
                FileOperations::instance().extractZip(sel.first(), m_currentPath);
            });
            menu.addAction(QIcon(":/icons/folder.svg"), QString("Extract to \"%1/\"").arg(firstFi.completeBaseName()), [this, sel, firstFi]() {
                QString targetDir = m_currentPath + "/" + firstFi.completeBaseName();
                FileOperations::instance().extractZip(sel.first(), targetDir);
            });
            menu.addAction(QIcon(":/icons/file-archive.svg"), "Extract to...", [this, sel]() {
                QString targetDir = QFileDialog::getExistingDirectory(this, "Extract to Location", m_currentPath);
                if (!targetDir.isEmpty()) {
                    FileOperations::instance().extractZip(sel.first(), targetDir);
                }
            });
        } else {
            menu.addSeparator();
            QString zipName = (sel.size() == 1) ? (firstFi.completeBaseName() + ".zip") : "Archive.zip";
            menu.addAction(QIcon(":/icons/file-archive.svg"), QString("Compress to \"%1\"").arg(zipName), [this, sel, zipName]() {
                QString targetZip = m_currentPath + "/" + zipName;
                FileOperations::instance().compressToZip(sel, targetZip);
            });
        }

        menu.addSeparator();
        menu.addAction(QIcon(":/icons/cut.svg"), "Cut", [sel]() {
            s_clipboardPaths = sel;
            s_isCutOperation = true;
        });
        menu.addAction(QIcon(":/icons/copy.svg"), "Copy", [sel]() {
            s_clipboardPaths = sel;
            s_isCutOperation = false;
        });
        menu.addAction(QIcon(":/icons/copy.svg"), "Copy Location (Ctrl+Shift+C)", [sel, this]() {
            QString text = sel.join("\n");
            QGuiApplication::clipboard()->setText(text);
            emit statusMessageChanged(QString("Location copied: %1").arg(text));
        });

        if (sel.size() == 1) {
            menu.addAction(QIcon(":/icons/rename.svg"), "Rename (F2)", [this, sel]() {
                QFileInfo fi(sel.first());
                bool ok = false;
                QString newName = QInputDialog::getText(this, "Rename Item", "New name:", QLineEdit::Normal, fi.fileName(), &ok);
                if (ok && !newName.isEmpty() && newName != fi.fileName()) {
                    FileOperations::instance().rename(fi.absoluteFilePath(), newName);
                }
            });

            if (QFileInfo(sel.first()).isDir()) {
                menu.addAction(QIcon(":/icons/bookmark.svg"), "Add to Bookmarks", [sel]() {
                    ConfigManager::instance().addBookmark(sel.first());
                });
            }
        }

        menu.addSeparator();
        menu.addAction(QIcon(":/icons/trash.svg"), "Move to Trash (Del)", [sel]() {
            FileOperations::instance().trash(sel);
        });
        menu.addAction(QIcon(":/icons/delete.svg"), "Delete Permanently (Shift+Del)", [this, sel]() {
            if (QMessageBox::question(this, "Permanent Delete",
                QString("Permanently delete %1 selected item(s)?").arg(sel.size())) == QMessageBox::Yes) {
                FileOperations::instance().permanentDelete(sel);
            }
        });
    } else {
        // Empty space context actions
        menu.addAction(QIcon(":/icons/folder.svg"), "New Folder... (Ctrl+Shift+N)", [this]() {
            promptCreateFolder();
        });
        menu.addAction(QIcon(":/icons/file.svg"), "New Empty File... (Ctrl+Alt+N)", [this]() {
            promptCreateFile("untitled.txt");
        });

        QMenu* templateMenu = menu.addMenu(QIcon(":/icons/file-code.svg"), "Create from Format / Template");
        templateMenu->addAction(QIcon(":/icons/file.svg"), "Plain Text (.txt)", [this]() {
            promptCreateFile("document.txt");
        });
        templateMenu->addAction(QIcon(":/icons/file.svg"), "Markdown (.md)", [this]() {
            promptCreateFile("notes.md", "# Notes\n\n");
        });
        templateMenu->addAction(QIcon(":/icons/file-code.svg"), "C++ Source (.cpp)", [this]() {
            promptCreateFile("main.cpp", "#include <iostream>\n\nint main() {\n    std::cout << \"Hello, World!\" << std::endl;\n    return 0;\n}\n");
        });
        templateMenu->addAction(QIcon(":/icons/file-code.svg"), "C++ Header (.hpp)", [this]() {
            promptCreateFile("header.hpp", "#pragma once\n\n");
        });
        templateMenu->addAction(QIcon(":/icons/file-code.svg"), "Python Script (.py)", [this]() {
            promptCreateFile("script.py", "#!/usr/bin/env python3\n\ndef main():\n    print(\"Hello!\")\n\nif __name__ == \"__main__\":\n    main()\n");
        });
        templateMenu->addAction(QIcon(":/icons/file-code.svg"), "Shell Script (.sh)", [this]() {
            promptCreateFile("script.sh", "#!/usr/bin/env bash\nset -euo pipefail\n\n");
        });
        templateMenu->addAction(QIcon(":/icons/file-code.svg"), "JSON Document (.json)", [this]() {
            promptCreateFile("data.json", "{\n  \n}\n");
        });
        templateMenu->addAction(QIcon(":/icons/file-code.svg"), "TOML Config (.toml)", [this]() {
            promptCreateFile("config.toml", "# Configuration\n");
        });
        templateMenu->addAction(QIcon(":/icons/file-code.svg"), "HTML Document (.html)", [this]() {
            promptCreateFile("index.html", "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n  <meta charset=\"UTF-8\">\n  <title>Document</title>\n</head>\n<body>\n  \n</body>\n</html>\n");
        });

        menu.addSeparator();
        if (!s_clipboardPaths.isEmpty()) {
            menu.addAction(QIcon(":/icons/paste.svg"), QString("Paste %1 Items").arg(s_clipboardPaths.size()), [this]() {
                if (s_isCutOperation) {
                    FileOperations::instance().move(s_clipboardPaths, m_currentPath);
                    s_clipboardPaths.clear();
                } else {
                    FileOperations::instance().copy(s_clipboardPaths, m_currentPath);
                }
            });
            menu.addSeparator();
        }

        menu.addAction(QIcon(":/icons/copy.svg"), "Copy Location (Ctrl+Shift+C)", [this]() {
            QGuiApplication::clipboard()->setText(m_currentPath);
            emit statusMessageChanged(QString("Location copied: %1").arg(m_currentPath));
        });

        menu.addAction(QIcon(":/icons/downloads.svg"), "Download with Evalink... (Ctrl+D)", [this]() {
            EvalinkDialog dlg(m_currentPath, this);
            if (dlg.exec() == QDialog::Accepted) {
                EvalinkManager::instance().addDownload(dlg.url(), dlg.destinationDirectory(), dlg.customFilename());
            }
        });

        menu.addAction(QIcon(":/icons/terminal.svg"), "Open in EvaTerm (F4)", [this]() {
            EvaSuiteBridge::openInEvaTerm(m_currentPath);
        });

        if (EvaSuiteBridge::isEvaSortAvailable()) {
            menu.addAction(QIcon(":/icons/sort.svg"), "Smart Sort with EvaSort", [this]() {
                EvaSuiteBridge::sortWithEvaSort(m_currentPath, this);
            });
        }

        menu.addAction(QIcon(":/icons/refresh.svg"), "Reload (F5)", [this]() {
            setPath(m_currentPath);
        });

        menu.addSeparator();
        QAction* hiddenAct = menu.addAction(QIcon(":/icons/eye.svg"), "Show Hidden Files (Ctrl+H)", [this]() {
            setShowHidden(!m_showHidden);
            ConfigManager::instance().config().showHidden = m_showHidden;
            ConfigManager::instance().save();
        });
        hiddenAct->setCheckable(true);
        hiddenAct->setChecked(m_showHidden);
    }

    menu.exec(event->globalPos());
}
