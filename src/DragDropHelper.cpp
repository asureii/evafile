#include "DragDropHelper.hpp"
#include "FileOperations.hpp"
#include <QApplication>
#include <QFontMetrics>
#include <QTimer>

void DragDropHelper::startDrag(QWidget* sourceWidget, const QStringList& paths, const QIcon& primaryIcon) {
    if (paths.isEmpty() || !sourceWidget) return;

    QList<QUrl> urls;
    for (const QString& p : paths) {
        urls.append(QUrl::fromLocalFile(p));
    }

    QDrag* drag = new QDrag(sourceWidget);
    QMimeData* mimeData = new QMimeData();
    mimeData->setUrls(urls);
    drag->setMimeData(mimeData);

    // Create a sleek drag thumbnail
    QPixmap pixmap(200, 36);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Rounded background pill in Crimson Flame aesthetic
    QPainterPath bgPath;
    bgPath.addRoundedRect(QRectF(1, 1, 198, 34), 6, 6);
    painter.fillPath(bgPath, QColor(0x36, 0x0e, 0x10, 235));
    painter.setPen(QPen(QColor(0xcf, 0x28, 0x24), 1.5));
    painter.drawPath(bgPath);

    // Icon
    QIcon icon = primaryIcon.isNull() ? QIcon(":/icons/file.svg") : primaryIcon;
    icon.paint(&painter, 8, 8, 20, 20);

    // Label text
    painter.setPen(QColor("#ebdada"));
    QFont font = painter.font();
    font.setPointSize(9);
    font.setBold(true);
    painter.setFont(font);

    QString text;
    if (paths.size() == 1) {
        text = QFileInfo(paths.first()).fileName();
    } else {
        text = QString("%1 items").arg(paths.size());
    }
    QFontMetrics fm(font);
    QString elided = fm.elidedText(text, Qt::ElideRight, 150);
    painter.drawText(36, 22, elided);
    painter.end();

    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(18, 18));

    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction);
}

bool DragDropHelper::handleDragEnter(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return true;
    }
    event->ignore();
    return false;
}

bool DragDropHelper::handleDragMove(QDragMoveEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return true;
    }
    event->ignore();
    return false;
}

bool DragDropHelper::executeDrop(QDropEvent* event, const QString& targetDir, QWidget* parentWidget) {
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return false;
    }

    QString cleanTarget = QDir::cleanPath(targetDir);
    if (cleanTarget.isEmpty() || !QDir(cleanTarget).exists()) {
        event->ignore();
        return false;
    }

    QStringList sourcePaths;
    for (const QUrl& url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            sourcePaths.append(QDir::cleanPath(url.toLocalFile()));
        }
    }

    if (sourcePaths.isEmpty()) {
        event->ignore();
        return false;
    }

    // Check for circular directory nesting or dropping onto self
    for (const QString& src : sourcePaths) {
        if (cleanTarget == src) {
            event->ignore();
            return false;
        }
        if (QFileInfo(src).isDir()) {
            if (cleanTarget == src || cleanTarget.startsWith(src + "/")) {
                if (parentWidget) {
                    QMessageBox::warning(parentWidget, "Invalid Destination",
                        QString("Cannot move or copy \"%1\" into a subdirectory of itself.").arg(QFileInfo(src).fileName()));
                }
                event->ignore();
                return false;
            }
        }
    }

    event->acceptProposedAction();

    bool sameDir = true;
    for (const QString& src : sourcePaths) {
        if (QFileInfo(src).absolutePath() != cleanTarget) {
            sameDir = false;
            break;
        }
    }

    Qt::KeyboardModifiers mods = event->modifiers();
    if (mods & Qt::ControlModifier) {
        FileOperations::instance().copy(sourcePaths, cleanTarget);
        return true;
    }
    if (mods & Qt::ShiftModifier) {
        if (!sameDir) {
            FileOperations::instance().move(sourcePaths, cleanTarget);
            return true;
        }
        return false;
    }

    // Defer the popup menu to the next event loop cycle so the DND grab is fully released by the window manager
    QPoint dropPos = QCursor::pos();
    QTimer::singleShot(0, [sourcePaths, cleanTarget, parentWidget, dropPos]() {
        bool sameDir = true;
        for (const QString& src : sourcePaths) {
            if (QFileInfo(src).absolutePath() != cleanTarget) {
                sameDir = false;
                break;
            }
        }

        QMenu* menu = new QMenu(parentWidget);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->setStyleSheet(
            "QMenu { background-color: #25090a; border: 1px solid #cf2824; border-radius: 6px; padding: 4px; }"
            "QMenu::item { color: #ebdada; padding: 6px 18px; border-radius: 4px; font-weight: bold; font-size: 11px; }"
            "QMenu::item:selected { background-color: #cf2824; color: #ffffff; }"
            "QMenu::item:disabled { color: #664448; }"
        );

        QAction* moveAct = nullptr;
        if (!sameDir) {
            moveAct = menu->addAction(QIcon(":/icons/folder.svg"), "Move Here");
        } else {
            moveAct = menu->addAction(QIcon(":/icons/folder.svg"), "Move Here (Already in this folder)");
            moveAct->setEnabled(false);
        }
        QAction* copyAct = menu->addAction(QIcon(":/icons/copy.svg"), "Copy Here");
        menu->addSeparator();
        QAction* cancelAct = menu->addAction(QIcon(":/icons/close.svg"), "Cancel");

        QAction* chosen = menu->exec(dropPos);
        if (chosen == moveAct && moveAct != nullptr && !sameDir) {
            FileOperations::instance().move(sourcePaths, cleanTarget);
        } else if (chosen == copyAct) {
            FileOperations::instance().copy(sourcePaths, cleanTarget);
        }
    });

    return true;
}
