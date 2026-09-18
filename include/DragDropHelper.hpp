#pragma once

#include <QStringList>
#include <QUrl>
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QIcon>
#include <QMenu>
#include <QWidget>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QCursor>

class DragDropHelper {
public:
    static void startDrag(QWidget* sourceWidget, const QStringList& paths, const QIcon& primaryIcon = QIcon());
    static bool handleDragEnter(QDragEnterEvent* event);
    static bool handleDragMove(QDragMoveEvent* event);
    static bool executeDrop(QDropEvent* event, const QString& targetDir, QWidget* parentWidget);
};
