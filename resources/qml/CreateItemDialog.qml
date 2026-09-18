import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property bool isFolder: true
    property string parentDirectory: controller.currentPath

    signal accepted()
    signal rejected()

    anchors.fill: parent
    color: Qt.rgba(0, 0, 0, 0.65)
    z: 100
    visible: opacity > 0.001

    Behavior on opacity { NumberAnimation { duration: Theme.animNormal; easing.type: Theme.easeDecel } }

    MouseArea {
        anchors.fill: parent
        onClicked: root.rejected()
    }

    Rectangle {
        id: dialogBox
        width: 440
        height: root.isFolder ? 220 : 340
        anchors.centerIn: parent
        radius: Theme.radiusLarge
        color: Theme.bgSurface
        border.color: Theme.borderLit
        border.width: 1

        scale: root.visible ? 1.0 : 0.9
        Behavior on scale { NumberAnimation { duration: Theme.animNormal; easing.type: Theme.easeSpring } }

        MouseArea {
            anchors.fill: parent
            onClicked: {} // Block clicks from closing dialog
        }

        // Top rim specular highlight
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 12
            height: 1
            color: Theme.rimTop
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 14

            // Header
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Image {
                    width: 24
                    height: 24
                    source: root.isFolder ? "qrc:/icons/folder.svg" : "qrc:/icons/file.svg"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: root.isFolder ? "CREATE NEW FOLDER" : "CREATE NEW FILE"
                    color: Theme.crimsonGlow
                    font.pixelSize: 13
                    font.bold: true
                    font.letterSpacing: 1.5
                    Layout.fillWidth: true
                }
            }

            Text {
                text: "Destination: " + root.parentDirectory
                color: Theme.textMuted
                font.pixelSize: 11
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }

            // Name input
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    text: root.isFolder ? "Folder Name:" : "File Name:"
                    color: Theme.textSecondary
                    font.pixelSize: 11
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    color: Theme.bgInput
                    radius: Theme.radiusSmall
                    border.color: nameInput.activeFocus ? Theme.crimson : Theme.borderSubtle
                    border.width: 1

                    TextInput {
                        id: nameInput
                        anchors.fill: parent
                        anchors.margins: 8
                        verticalAlignment: TextInput.AlignVCenter
                        color: Theme.textPrimary
                        font.pixelSize: 13
                        selectByMouse: true
                        selectionColor: Theme.crimson
                        text: root.isFolder ? "New Folder" : "NewFile.txt"

                        Component.onCompleted: {
                            forceActiveFocus();
                            selectAll();
                        }

                        onAccepted: root.confirmCreate()
                    }
                }
            }

            // File format template selection (if file)
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                visible: !root.isFolder

                Text {
                    text: "Quick Format / Template:"
                    color: Theme.textSecondary
                    font.pixelSize: 11
                    font.bold: true
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 6

                    Repeater {
                        model: [
                            { name: ".txt", label: "Text", ext: ".txt", template: "" },
                            { name: ".md", label: "Markdown", ext: ".md", template: "# Title\n\n" },
                            { name: ".cpp", label: "C++", ext: ".cpp", template: "#include <iostream>\n\nint main() {\n    return 0;\n}\n" },
                            { name: ".py", label: "Python", ext: ".py", template: "#!/usr/bin/env python3\n\n" },
                            { name: ".sh", label: "Shell", ext: ".sh", template: "#!/usr/bin/env bash\n\n" },
                            { name: ".json", label: "JSON", ext: ".json", template: "{\n  \"name\": \"\"\n}\n" },
                            { name: ".qml", label: "QML", ext: ".qml", template: "import QtQuick\n\nItem {\n}\n" }
                        ]

                        delegate: Rectangle {
                            width: formatLabel.implicitWidth + 14
                            height: 24
                            radius: 4
                            color: formatMouse.containsMouse ? Theme.bgCardHover : Theme.bgCard
                            border.color: Theme.borderSubtle
                            border.width: 1

                            Text {
                                id: formatLabel
                                anchors.centerIn: parent
                                text: modelData.name
                                color: Theme.textSecondary
                                font.pixelSize: 11
                            }

                            MouseArea {
                                id: formatMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    var current = nameInput.text;
                                    var dotIdx = current.lastIndexOf(".");
                                    if (dotIdx !== -1) {
                                        current = current.substring(0, dotIdx);
                                    }
                                    nameInput.text = current + modelData.ext;
                                    root.selectedTemplate = modelData.template;
                                }
                            }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }

            // Dialog Buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: root.rejected()
                    background: Rectangle {
                        color: parent.pressed ? Theme.bgCardActive : (parent.hovered ? Theme.bgCardHover : Theme.bgCard)
                        radius: Theme.radiusSmall
                        border.color: Theme.borderSubtle
                        border.width: 1
                    }
                    contentItem: Text {
                        text: parent.text
                        color: Theme.textSecondary
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                Button {
                    text: root.isFolder ? "Create Folder" : "Create File"
                    onClicked: root.confirmCreate()
                    background: Rectangle {
                        color: parent.pressed ? Theme.crimsonHover : (parent.hovered ? Theme.crimsonHover : Theme.crimson)
                        radius: Theme.radiusSmall
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        font.pixelSize: 12
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }

    property string selectedTemplate: ""

    function openFolderDialog(dir) {
        root.isFolder = true;
        root.parentDirectory = dir || controller.currentPath;
        nameInput.text = "New Folder";
        root.opacity = 1.0;
        nameInput.forceActiveFocus();
        nameInput.selectAll();
    }

    function openFileDialog(dir) {
        root.isFolder = false;
        root.parentDirectory = dir || controller.currentPath;
        nameInput.text = "NewFile.txt";
        root.selectedTemplate = "";
        root.opacity = 1.0;
        nameInput.forceActiveFocus();
        nameInput.selectAll();
    }

    function confirmCreate() {
        var name = nameInput.text.trim();
        if (name.length === 0) return;

        if (root.isFolder) {
            controller.createFolder(root.parentDirectory, name);
        } else {
            controller.createFile(root.parentDirectory, name, root.selectedTemplate);
        }
        root.accepted();
        root.opacity = 0.0;
    }
}
