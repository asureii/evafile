import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.evafile 1.0

Rectangle {
    id: root

    property string folderPath: controller.currentPath
    property bool isSplitPane: false
    property var selectedPaths: []
    property string activeItemPath: ""

    signal dropRequested(var sources, string destination)
    signal newFolderRequested(string parentDir)
    signal newFileRequested(string parentDir)

    color: Theme.bgBase
    clip: true

    EvaFileFolderModel {
        id: folderModel
        path: root.folderPath
        filterText: controller.searchQuery
        showHidden: controller.showHidden
    }

    Connections {
        target: controller
        function onFolderRefreshRequested() {
            folderModel.reload();
        }
    }

    // Header for Details Mode
    Rectangle {
        id: detailsHeader
        visible: controller.viewMode === 0
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 28
        color: Theme.bgSurface
        border.color: Theme.borderSubtle
        border.width: 1
        z: 2

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 8

            Text {
                text: "NAME"
                color: Theme.textMuted
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 1.0
                Layout.fillWidth: true
            }

            Text {
                text: "SIZE"
                color: Theme.textMuted
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 1.0
                Layout.preferredWidth: 80
                horizontalAlignment: Text.AlignRight
            }

            Text {
                text: "TYPE"
                color: Theme.textMuted
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 1.0
                Layout.preferredWidth: 110
            }

            Text {
                text: "MODIFIED"
                color: Theme.textMuted
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 1.0
                Layout.preferredWidth: 130
            }
        }
    }

    // View Mode 0: Details ListView
    ListView {
        id: detailsList
        visible: controller.viewMode === 0
        anchors.top: detailsHeader.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        model: folderModel
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { active: true }

        delegate: Rectangle {
            id: rowDelegate
            width: detailsList.width
            height: 36
            readonly property bool isSelected: root.selectedPaths.indexOf(model.path) !== -1
            color: isSelected ? Qt.rgba(Theme.crimson.r, Theme.crimson.g, Theme.crimson.b, 0.24) : (rowMouse.containsMouse ? Theme.bgCardHover : (index % 2 === 0 ? "transparent" : Qt.rgba(0, 0, 0, 0.15)))
            border.color: isSelected ? Theme.borderLit : "transparent"
            border.width: 1

            Behavior on color { ColorAnimation { duration: Theme.animFast } }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8

                Image {
                    width: 20
                    height: 20
                    source: "qrc:/icons/" + model.iconName + ".svg"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: model.name
                    color: rowDelegate.isSelected ? Theme.textPrimary : (model.isHidden ? Theme.textMuted : Theme.textPrimary)
                    font.pixelSize: 12
                    font.bold: model.isDir
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }

                Text {
                    text: model.sizeFormatted
                    color: Theme.textSecondary
                    font.pixelSize: 11
                    Layout.preferredWidth: 80
                    horizontalAlignment: Text.AlignRight
                }

                Text {
                    text: model.type
                    color: Theme.textMuted
                    font.pixelSize: 11
                    Layout.preferredWidth: 110
                    elide: Text.ElideRight
                }

                Text {
                    text: model.modified
                    color: Theme.textMuted
                    font.pixelSize: 11
                    Layout.preferredWidth: 130
                }
            }

            MouseArea {
                id: rowMouse
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onClicked: (mouse) => {
                    if (mouse.button === Qt.RightButton) {
                        root.activeItemPath = model.path;
                        if (root.selectedPaths.indexOf(model.path) === -1) {
                            root.selectedPaths = [model.path];
                        }
                        itemContextMenu.popup();
                    } else {
                        if (mouse.modifiers & Qt.ControlModifier) {
                            var arr = root.selectedPaths.slice();
                            var idx = arr.indexOf(model.path);
                            if (idx >= 0) arr.splice(idx, 1);
                            else arr.push(model.path);
                            root.selectedPaths = arr;
                        } else {
                            root.selectedPaths = [model.path];
                        }
                        controller.previewFile(model.path);
                    }
                }

                onDoubleClicked: {
                    if (model.isDir) {
                        if (root.isSplitPane) {
                            controller.setSplitPath(model.path);
                        } else {
                            controller.navigateTo(model.path);
                        }
                    } else {
                        controller.openFile(model.path);
                    }
                }
            }
        }
    }

    // View Mode 1: Icons GridView with 3D Perspective Tilt Cards
    GridView {
        id: iconsGrid
        visible: controller.viewMode === 1
        anchors.fill: parent
        anchors.margins: 12
        cellWidth: 130
        cellHeight: 140
        model: folderModel
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { active: true }

        delegate: Item {
            id: gridDelegateItem
            width: iconsGrid.cellWidth
            height: iconsGrid.cellHeight

            readonly property bool isSelected: root.selectedPaths.indexOf(model.path) !== -1

            Rectangle {
                id: card
                anchors.centerIn: parent
                width: parent.width - 12
                height: parent.height - 12
                radius: Theme.radiusMedium
                color: gridDelegateItem.isSelected ? Qt.rgba(Theme.crimson.r, Theme.crimson.g, Theme.crimson.b, 0.28) : (gridMouse.containsMouse ? Theme.bgCardHover : Theme.bgCard)
                border.color: gridDelegateItem.isSelected ? Theme.borderLit : (gridMouse.containsMouse ? Theme.borderHighlight : Theme.borderSubtle)
                border.width: 1

                scale: gridMouse.pressed ? 0.95 : (gridMouse.containsMouse ? 1.04 : 1.0)
                Behavior on scale { NumberAnimation { duration: Theme.animFast; easing.type: Theme.easeDecel } }
                Behavior on color { ColorAnimation { duration: Theme.animFast } }
                Behavior on border.color { ColorAnimation { duration: Theme.animFast } }

                property real normX: 0.0
                property real normY: 0.0

                // 3D Perspective Rotation from studio-design-qml
                transform: [
                    Rotation {
                        origin.x: card.width / 2
                        origin.y: card.height / 2
                        axis { x: 0; y: 1; z: 0 }
                        angle: card.normX * 8
                        Behavior on angle { NumberAnimation { duration: 180; easing.type: Theme.easeDecel } }
                    },
                    Rotation {
                        origin.x: card.width / 2
                        origin.y: card.height / 2
                        axis { x: 1; y: 0; z: 0 }
                        angle: -card.normY * 8
                        Behavior on angle { NumberAnimation { duration: 180; easing.type: Theme.easeDecel } }
                    }
                ]

                // Specular Glare Overlay
                Rectangle {
                    anchors.fill: parent
                    radius: parent.radius
                    opacity: gridMouse.containsMouse ? 0.12 : 0.0
                    Behavior on opacity { NumberAnimation { duration: Theme.animFast } }
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#ffffff" }
                        GradientStop { position: 1.0; color: "transparent" }
                    }
                }

                Column {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 6

                    Item {
                        width: parent.width
                        height: 52

                        Image {
                            anchors.centerIn: parent
                            width: 44
                            height: 44
                            source: "qrc:/icons/" + model.iconName + ".svg"
                            fillMode: Image.PreserveAspectFit
                            opacity: model.isHidden ? 0.5 : 1.0
                        }
                    }

                    Text {
                        width: parent.width
                        text: model.name
                        color: gridDelegateItem.isSelected ? "#ffffff" : Theme.textPrimary
                        font.pixelSize: 11
                        font.bold: model.isDir
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideMiddle
                        maximumLineCount: 2
                        wrapMode: Text.WrapAnywhere
                    }

                    Text {
                        width: parent.width
                        text: model.sizeFormatted
                        color: Theme.textMuted
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                MouseArea {
                    id: gridMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onPositionChanged: (m) => {
                        card.normX = (m.x - width / 2) / (width / 2);
                        card.normY = (m.y - height / 2) / (height / 2);
                    }
                    onExited: {
                        card.normX = 0;
                        card.normY = 0;
                    }

                    onClicked: (mouse) => {
                        if (mouse.button === Qt.RightButton) {
                            root.activeItemPath = model.path;
                            if (root.selectedPaths.indexOf(model.path) === -1) {
                                root.selectedPaths = [model.path];
                            }
                            itemContextMenu.popup();
                        } else {
                            if (mouse.modifiers & Qt.ControlModifier) {
                                var arr = root.selectedPaths.slice();
                                var idx = arr.indexOf(model.path);
                                if (idx >= 0) arr.splice(idx, 1);
                                else arr.push(model.path);
                                root.selectedPaths = arr;
                            } else {
                                root.selectedPaths = [model.path];
                            }
                            controller.previewFile(model.path);
                        }
                    }

                    onDoubleClicked: {
                        if (model.isDir) {
                            if (root.isSplitPane) {
                                controller.setSplitPath(model.path);
                            } else {
                                controller.navigateTo(model.path);
                            }
                        } else {
                            controller.openFile(model.path);
                        }
                    }
                }
            }
        }
    }

    // Empty Folder Placeholder
    Item {
        anchors.centerIn: parent
        visible: folderModel.count === 0
        width: 240
        height: 180

        Column {
            anchors.centerIn: parent
            spacing: 12

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 48
                height: 48
                source: "qrc:/icons/folder-open.svg"
                fillMode: Image.PreserveAspectFit
                opacity: 0.35
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: controller.searchQuery.length > 0 ? "No matches for \"" + controller.searchQuery + "\"" : "This folder is empty"
                color: Theme.textMuted
                font.pixelSize: 13
            }
        }
    }

    // Background Click Area (clears selection, background context menu)
    MouseArea {
        anchors.fill: parent
        z: -1
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton) {
                backgroundContextMenu.popup();
            } else {
                root.selectedPaths = [];
            }
        }
    }

    // Drop Area for drag-and-drop file operations
    DropArea {
        id: dropArea
        anchors.fill: parent
        onDropped: (drop) => {
            if (drop.hasUrls) {
                var urls = [];
                for (var i = 0; i < drop.urls.length; ++i) {
                    urls.push(drop.urls[i]);
                }
                root.dropRequested(urls, root.folderPath);
            }
        }
    }

    // Item Context Menu
    Menu {
        id: itemContextMenu

        MenuItem {
            text: "Open"
            icon.source: "qrc:/icons/folder-open.svg"
            onTriggered: controller.openFile(root.activeItemPath)
        }
        MenuItem {
            text: "Open in EvaTerm"
            icon.source: "qrc:/icons/terminal.svg"
            onTriggered: controller.openInEvaTerm(root.activeItemPath)
        }
        MenuItem {
            text: "Sort with EvaSort"
            icon.source: "qrc:/icons/sort.svg"
            onTriggered: controller.sortWithEvaSort(root.activeItemPath)
        }
        MenuItem {
            text: "Copy Location"
            icon.source: "qrc:/icons/copy.svg"
            onTriggered: controller.copyLocation(root.activeItemPath)
        }
        MenuSeparator {}
        MenuItem {
            text: "Copy"
            icon.source: "qrc:/icons/copy.svg"
            onTriggered: controller.copyPaths(root.selectedPaths, "")
        }
        MenuItem {
            text: "Cut"
            icon.source: "qrc:/icons/cut.svg"
            onTriggered: controller.movePaths(root.selectedPaths, "")
        }
        MenuItem {
            text: "Move to Trash"
            icon.source: "qrc:/icons/trash.svg"
            onTriggered: controller.trashPaths(root.selectedPaths)
        }
        MenuItem {
            text: "Delete Permanently"
            onTriggered: controller.deletePaths(root.selectedPaths)
        }
        MenuSeparator {}
        MenuItem {
            text: "Extract Here"
            icon.source: "qrc:/icons/file-archive.svg"
            visible: isZipOrArchive(root.activeItemPath)
            onTriggered: controller.extractZip(root.activeItemPath, root.folderPath)
        }
        MenuItem {
            text: "Extract to Folder..."
            icon.source: "qrc:/icons/file-archive.svg"
            visible: isZipOrArchive(root.activeItemPath)
            onTriggered: {
                var baseName = root.activeItemPath.replace(/^.*[\\\/]/, '').replace(/\.[^/.]+$/, "");
                var dest = root.folderPath + "/" + baseName;
                controller.createFolder(root.folderPath, baseName);
                controller.extractZip(root.activeItemPath, dest);
            }
        }
        MenuItem {
            text: "Compress to ZIP..."
            icon.source: "qrc:/icons/file-archive.svg"
            onTriggered: {
                var zipName = root.folderPath + "/archive.zip";
                controller.compressZip(root.selectedPaths, zipName);
            }
        }
        MenuSeparator {}
        MenuItem {
            text: "Bookmark Folder"
            icon.source: "qrc:/icons/bookmark.svg"
            onTriggered: controller.addBookmark(root.activeItemPath)
        }
        MenuItem {
            text: "Preview"
            icon.source: "qrc:/icons/preview.svg"
            onTriggered: controller.previewFile(root.activeItemPath)
        }
    }

    // Background Context Menu
    Menu {
        id: backgroundContextMenu

        MenuItem {
            text: "New Folder..."
            icon.source: "qrc:/icons/folder.svg"
            onTriggered: root.newFolderRequested(root.folderPath)
        }
        MenuItem {
            text: "New File..."
            icon.source: "qrc:/icons/file.svg"
            onTriggered: root.newFileRequested(root.folderPath)
        }
        MenuSeparator {}
        MenuItem {
            text: "Open in EvaTerm (F4)"
            icon.source: "qrc:/icons/terminal.svg"
            onTriggered: controller.openInEvaTerm(root.folderPath)
        }
        MenuItem {
            text: "Sort with EvaSort"
            icon.source: "qrc:/icons/sort.svg"
            onTriggered: controller.sortWithEvaSort(root.folderPath)
        }
        MenuItem {
            text: "Copy Location"
            icon.source: "qrc:/icons/copy.svg"
            onTriggered: controller.copyLocation(root.folderPath)
        }
        MenuItem {
            text: "Reload (Ctrl+R)"
            icon.source: "qrc:/icons/refresh.svg"
            onTriggered: controller.refresh()
        }
    }

    function isZipOrArchive(filePath) {
        var lower = filePath.toLowerCase();
        return lower.endsWith(".zip") || lower.endsWith(".tar") || lower.endsWith(".gz") || lower.endsWith(".xz") || lower.endsWith(".7z");
    }
}
