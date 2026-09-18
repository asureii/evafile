import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    anchors.fill: parent
    color: Qt.rgba(0, 0, 0, 0.65)
    z: 100
    opacity: 0.0
    visible: opacity > 0.001

    Behavior on opacity { NumberAnimation { duration: Theme.animNormal; easing.type: Theme.easeDecel } }

    MouseArea {
        anchors.fill: parent
        onClicked: root.close()
    }

    Rectangle {
        id: dialogBox
        width: 480
        height: 290
        anchors.centerIn: parent
        radius: Theme.radiusLarge
        color: Theme.bgSurface
        border.color: Theme.borderLit
        border.width: 1

        scale: root.visible ? 1.0 : 0.9
        Behavior on scale { NumberAnimation { duration: Theme.animNormal; easing.type: Theme.easeSpring } }

        MouseArea {
            anchors.fill: parent
            onClicked: {}
        }

        // Top rim highlight
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
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Image {
                    width: 24
                    height: 24
                    source: "qrc:/icons/disk.svg"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: "ADD EVALINK DOWNLOAD"
                    color: Theme.crimsonGlow
                    font.pixelSize: 13
                    font.bold: true
                    font.letterSpacing: 1.5
                    Layout.fillWidth: true
                }
            }

            // Download URL
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text { text: "Download URL (HTTP/HTTPS/FTP/Magnet):"; color: Theme.textSecondary; font.pixelSize: 11; font.bold: true }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    color: Theme.bgInput
                    radius: Theme.radiusSmall
                    border.color: urlInput.activeFocus ? Theme.crimson : Theme.borderSubtle
                    border.width: 1

                    TextInput {
                        id: urlInput
                        anchors.fill: parent
                        anchors.margins: 8
                        verticalAlignment: TextInput.AlignVCenter
                        color: Theme.textPrimary
                        font.pixelSize: 12
                        selectByMouse: true
                        selectionColor: Theme.crimson
                        text: ""
                    }
                }
            }

            // Optional Custom Filename
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text { text: "Custom Filename (optional):"; color: Theme.textSecondary; font.pixelSize: 11; font.bold: true }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    color: Theme.bgInput
                    radius: Theme.radiusSmall
                    border.color: filenameInput.activeFocus ? Theme.crimson : Theme.borderSubtle
                    border.width: 1

                    TextInput {
                        id: filenameInput
                        anchors.fill: parent
                        anchors.margins: 8
                        verticalAlignment: TextInput.AlignVCenter
                        color: Theme.textPrimary
                        font.pixelSize: 12
                        selectByMouse: true
                        selectionColor: Theme.crimson
                        text: ""
                    }
                }
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: root.close()
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
                    text: "Start Download"
                    onClicked: root.startDownload()
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

    function open() {
        urlInput.text = "";
        filenameInput.text = "";
        root.opacity = 1.0;
        urlInput.forceActiveFocus();
    }

    function close() {
        root.opacity = 0.0;
    }

    function startDownload() {
        var url = urlInput.text.trim();
        if (url.length === 0) return;
        var fn = filenameInput.text.trim();
        controller.startEvalinkDownload(url, controller.currentPath, fn);
        close();
    }
}
