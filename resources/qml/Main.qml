import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.evafile 1.0

ApplicationWindow {
    id: appWindow

    width: 1180
    height: 740
    minimumWidth: 720
    minimumHeight: 480
    visible: true
    title: "EvaFile - " + controller.currentPath
    color: Theme.bgVoid

    // Global Key Shortcuts
    Shortcut {
        sequence: "F4"
        onActivated: controller.openInEvaTerm()
    }
    Shortcut {
        sequence: "F3"
        onActivated: controller.toggleSplit()
    }
    Shortcut {
        sequence: "F5"
        onActivated: controller.refresh()
    }
    Shortcut {
        sequence: "Ctrl+R"
        onActivated: controller.refresh()
    }
    Shortcut {
        sequence: "Ctrl+H"
        onActivated: controller.showHidden = !controller.showHidden
    }
    Shortcut {
        sequence: "Space"
        onActivated: controller.togglePreview()
    }
    Shortcut {
        sequence: "Alt+Left"
        onActivated: controller.goBack()
    }
    Shortcut {
        sequence: "Alt+Right"
        onActivated: controller.goForward()
    }
    Shortcut {
        sequence: "Alt+Up"
        onActivated: controller.goUp()
    }
    Shortcut {
        sequence: "Ctrl+Shift+N"
        onActivated: createItemDialog.openFolderDialog(controller.currentPath)
    }
    Shortcut {
        sequence: "Ctrl+N"
        onActivated: createItemDialog.openFileDialog(controller.currentPath)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header Bar
        HeaderBar {
            id: headerBar
            Layout.fillWidth: true
            onNewFolderRequested: createItemDialog.openFolderDialog(controller.currentPath)
            onNewFileRequested: createItemDialog.openFileDialog(controller.currentPath)
        }

        // Main Center Body
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            RowLayout {
                anchors.fill: parent
                spacing: 0

                // Collapsible Sidebar
                Sidebar {
                    id: sidebar
                    Layout.fillHeight: true
                }

                // File Views (Single or Dual Pane)
                Item {
                    id: fileViewsContainer
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    RowLayout {
                        anchors.fill: parent
                        spacing: 0

                        // Main Active Pane
                        FileView {
                            id: mainPane
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            folderPath: controller.currentPath
                            isSplitPane: false

                            onDropRequested: (urls, dest) => dropActionDialog.open(urls, dest)
                            onNewFolderRequested: (dir) => createItemDialog.openFolderDialog(dir)
                            onNewFileRequested: (dir) => createItemDialog.openFileDialog(dir)
                        }

                        // Split Pane Divider
                        Rectangle {
                            visible: controller.isSplit
                            Layout.fillHeight: true
                            Layout.preferredWidth: 2
                            color: Theme.borderSubtle
                        }

                        // Secondary Pane
                        FileView {
                            id: splitPane
                            visible: controller.isSplit
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            folderPath: controller.splitPath
                            isSplitPane: true

                            onDropRequested: (urls, dest) => dropActionDialog.open(urls, dest)
                            onNewFolderRequested: (dir) => createItemDialog.openFolderDialog(dir)
                            onNewFileRequested: (dir) => createItemDialog.openFileDialog(dir)
                        }
                    }
                }

                // Sliding Preview Drawer
                PreviewDrawer {
                    id: previewDrawer
                    Layout.fillHeight: true
                    visible: controller.previewVisible
                }

                // Sliding Evalink Drawer
                EvalinkDrawer {
                    id: evalinkDrawer
                    Layout.fillHeight: true
                    visible: controller.evalinkDrawerVisible
                    onNewDownloadRequested: evalinkDialog.open()
                }
            }
        }

        // Bottom Status Bar
        StatusBar {
            id: statusBar
            Layout.fillWidth: true
        }
    }

    // Modal Dialogs
    CreateItemDialog {
        id: createItemDialog
    }

    EvalinkDialog {
        id: evalinkDialog
    }

    DropActionDialog {
        id: dropActionDialog
    }
}
