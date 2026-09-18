import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    height: 52
    color: Theme.bgSurfaceGlass
    border.color: Theme.borderSubtle
    border.width: 1

    signal newFolderRequested()
    signal newFileRequested()

    property bool isEditingPath: false

    // Top rim highlight for glass effect
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: Theme.rimTop
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 6

        // Navigation controls
        IconButton {
            iconSource: "qrc:/icons/arrow-left.svg"
            tooltipText: "Back (Alt+Left)"
            enabled: controller.canGoBack
            onClicked: controller.goBack()
        }

        IconButton {
            iconSource: "qrc:/icons/arrow-right.svg"
            tooltipText: "Forward (Alt+Right)"
            enabled: controller.canGoForward
            onClicked: controller.goForward()
        }

        IconButton {
            iconSource: "qrc:/icons/arrow-up.svg"
            tooltipText: "Up (Alt+Up)"
            enabled: controller.canGoUp
            onClicked: controller.goUp()
        }

        IconButton {
            iconSource: "qrc:/icons/home.svg"
            tooltipText: "Home"
            onClicked: controller.goHome()
        }

        IconButton {
            iconSource: "qrc:/icons/refresh.svg"
            tooltipText: "Reload (Ctrl+R / F5)"
            onClicked: controller.refresh()
        }

        Rectangle {
            width: 1
            height: 22
            color: Theme.borderSubtle
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 2
            Layout.rightMargin: 2
        }

        // Breadcrumb & Path Bar
        Rectangle {
            id: pathContainer
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            color: root.isEditingPath ? Theme.bgInput : Theme.bgCard
            radius: Theme.radiusSmall
            border.color: root.isEditingPath ? Theme.crimson : Theme.borderSubtle
            border.width: 1

            // Mode A: Breadcrumb items
            Flickable {
                id: breadcrumbFlickable
                visible: !root.isEditingPath
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 36
                contentWidth: breadcrumbRow.width
                contentHeight: height
                boundsBehavior: Flickable.StopAtBounds
                clip: true

                Row {
                    id: breadcrumbRow
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 2

                    // Root separator / icon
                    Rectangle {
                        width: 24
                        height: 24
                        radius: 4
                        color: rootCrumbMouse.containsMouse ? Theme.bgCardHover : "transparent"
                        anchors.verticalCenter: parent.verticalCenter

                        Image {
                            anchors.centerIn: parent
                            width: 14
                            height: 14
                            source: "qrc:/icons/disk.svg"
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            id: rootCrumbMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: controller.navigateTo("/")
                        }
                    }

                    Repeater {
                        model: {
                            var p = controller.currentPath;
                            if (p === "/") return [];
                            var parts = p.split("/").filter(function(s) { return s.length > 0; });
                            return parts;
                        }

                        delegate: Row {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: "/"
                                color: Theme.textMuted
                                font.pixelSize: 12
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Rectangle {
                                id: crumbButton
                                width: crumbText.implicitWidth + 12
                                height: 24
                                radius: 4
                                color: crumbMouse.containsMouse ? Theme.bgCardHover : "transparent"
                                anchors.verticalCenter: parent.verticalCenter

                                Text {
                                    id: crumbText
                                    anchors.centerIn: parent
                                    text: modelData
                                    color: (index === crumbModelLength() - 1) ? Theme.textPrimary : Theme.textSecondary
                                    font.pixelSize: 12
                                    font.bold: (index === crumbModelLength() - 1)
                                }

                                MouseArea {
                                    id: crumbMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        var parts = controller.currentPath.split("/").filter(function(s) { return s.length > 0; });
                                        var target = "/" + parts.slice(0, index + 1).join("/");
                                        controller.navigateTo(target);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            function crumbModelLength() {
                var p = controller.currentPath;
                if (p === "/") return 0;
                return p.split("/").filter(function(s) { return s.length > 0; }).length;
            }

            // Click background of breadcrumb to switch to direct edit
            MouseArea {
                anchors.fill: parent
                z: -1
                onClicked: {
                    root.isEditingPath = true;
                    pathInput.text = controller.currentPath;
                    pathInput.forceActiveFocus();
                    pathInput.selectAll();
                }
            }

            // Mode B: Direct Path Text Input
            TextInput {
                id: pathInput
                visible: root.isEditingPath
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 36
                verticalAlignment: TextInput.AlignVCenter
                color: Theme.textPrimary
                font.pixelSize: 12
                selectByMouse: true
                selectionColor: Theme.crimson

                onAccepted: {
                    controller.navigateTo(text);
                    root.isEditingPath = false;
                }

                Keys.onEscapePressed: {
                    root.isEditingPath = false;
                }
            }

            // Edit path icon / switch button
            IconButton {
                anchors.right: parent.right
                anchors.rightMargin: 2
                anchors.verticalCenter: parent.verticalCenter
                implicitWidth: 28
                implicitHeight: 28
                iconSize: 14
                iconSource: root.isEditingPath ? "qrc:/icons/close.svg" : "qrc:/icons/gear.svg"
                tooltipText: root.isEditingPath ? "Cancel Edit" : "Edit Path"
                onClicked: {
                    if (root.isEditingPath) {
                        root.isEditingPath = false;
                    } else {
                        root.isEditingPath = true;
                        pathInput.text = controller.currentPath;
                        pathInput.forceActiveFocus();
                        pathInput.selectAll();
                    }
                }
            }
        }

        // Quick Search Field
        Rectangle {
            Layout.preferredWidth: 180
            Layout.preferredHeight: 34
            color: Theme.bgCard
            radius: Theme.radiusSmall
            border.color: searchInput.activeFocus ? Theme.crimson : Theme.borderSubtle
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 6
                spacing: 4

                Image {
                    width: 14
                    height: 14
                    source: "qrc:/icons/search.svg"
                    fillMode: Image.PreserveAspectFit
                    opacity: 0.6
                }

                TextInput {
                    id: searchInput
                    Layout.fillWidth: true
                    verticalAlignment: TextInput.AlignVCenter
                    color: Theme.textPrimary
                    font.pixelSize: 12
                    selectByMouse: true
                    selectionColor: Theme.crimson
                    text: controller.searchQuery

                    onTextChanged: {
                        controller.searchQuery = text;
                    }

                    Text {
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        text: "Search folder..."
                        color: Theme.textMuted
                        font.pixelSize: 12
                        visible: !searchInput.text && !searchInput.activeFocus
                    }
                }

                IconButton {
                    visible: searchInput.text.length > 0
                    implicitWidth: 20
                    implicitHeight: 20
                    iconSize: 10
                    iconSource: "qrc:/icons/close.svg"
                    onClicked: {
                        searchInput.text = "";
                        controller.searchQuery = "";
                    }
                }
            }
        }

        Rectangle {
            width: 1
            height: 22
            color: Theme.borderSubtle
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 2
            Layout.rightMargin: 2
        }

        // Action cluster
        IconButton {
            iconSource: "qrc:/icons/folder.svg"
            tooltipText: "New Folder (Ctrl+Shift+N)"
            onClicked: root.newFolderRequested()
        }

        IconButton {
            iconSource: "qrc:/icons/file.svg"
            tooltipText: "New File (Ctrl+N)"
            onClicked: root.newFileRequested()
        }

        IconButton {
            iconSource: "qrc:/icons/terminal.svg"
            tooltipText: "Open in EvaTerm (F4)"
            onClicked: controller.openInEvaTerm()
        }

        IconButton {
            iconSource: "qrc:/icons/sort.svg"
            tooltipText: "Sort with EvaSort"
            onClicked: controller.sortWithEvaSort()
        }

        IconButton {
            iconSource: controller.viewMode === 0 ? "qrc:/icons/view-icons.svg" : "qrc:/icons/view-details.svg"
            tooltipText: controller.viewMode === 0 ? "Switch to Icons View" : "Switch to Details View"
            onClicked: controller.viewMode = (controller.viewMode === 0 ? 1 : 0)
        }

        IconButton {
            iconSource: "qrc:/icons/split.svg"
            tooltipText: "Toggle Dual Split Pane (F3)"
            isActive: controller.isSplit
            onClicked: controller.toggleSplit()
        }

        IconButton {
            iconSource: "qrc:/icons/eye.svg"
            tooltipText: "Show Hidden Files (Ctrl+H)"
            isActive: controller.showHidden
            onClicked: controller.showHidden = !controller.showHidden
        }

        // Evalink Drawer toggle
        IconButton {
            iconSource: "qrc:/icons/disk.svg"
            tooltipText: "Evalink Download Manager"
            isActive: controller.evalinkDrawerVisible
            badgeCount: controller.evalinkActiveCount
            onClicked: controller.toggleEvalinkDrawer()
        }

        // Preview Drawer toggle
        IconButton {
            iconSource: "qrc:/icons/preview.svg"
            tooltipText: "Toggle Preview Drawer (Space)"
            isActive: controller.previewVisible
            onClicked: controller.togglePreview()
        }
    }
}
