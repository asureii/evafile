import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property var sourceUrls: []
    property string targetDirectory: ""

    anchors.fill: parent
    color: Qt.rgba(0, 0, 0, 0.65)
    z: 100
    visible: opacity > 0.001

    Behavior on opacity { NumberAnimation { duration: Theme.animNormal; easing.type: Theme.easeDecel } }

    MouseArea {
        anchors.fill: parent
        onClicked: root.close()
    }

    Rectangle {
        id: dialogBox
        width: 400
        height: 180
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
            spacing: 14

            Text {
                text: "DROP ACTION"
                color: Theme.crimsonGlow
                font.pixelSize: 13
                font.bold: true
                font.letterSpacing: 1.5
            }

            Text {
                text: "What would you like to do with " + root.sourceUrls.length + " item(s)?"
                color: Theme.textPrimary
                font.pixelSize: 12
                Layout.fillWidth: true
            }

            Text {
                text: "Destination: " + root.targetDirectory
                color: Theme.textMuted
                font.pixelSize: 10
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

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
                        font.pixelSize: 11
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "Move Here"
                    onClicked: {
                        var paths = cleanUrls(root.sourceUrls);
                        controller.movePaths(paths, root.targetDirectory);
                        root.close();
                    }
                    background: Rectangle {
                        color: parent.pressed ? Theme.bgCardActive : (parent.hovered ? Theme.bgCardHover : Theme.bgCard)
                        radius: Theme.radiusSmall
                        border.color: Theme.borderLit
                        border.width: 1
                    }
                    contentItem: Text {
                        text: parent.text
                        color: Theme.textPrimary
                        font.pixelSize: 11
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                Button {
                    text: "Copy Here"
                    onClicked: {
                        var paths = cleanUrls(root.sourceUrls);
                        controller.copyPaths(paths, root.targetDirectory);
                        root.close();
                    }
                    background: Rectangle {
                        color: parent.pressed ? Theme.crimsonHover : (parent.hovered ? Theme.crimsonHover : Theme.crimson)
                        radius: Theme.radiusSmall
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        font.pixelSize: 11
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }

    function open(urls, targetDir) {
        root.sourceUrls = urls;
        root.targetDirectory = targetDir;
        root.opacity = 1.0;
    }

    function close() {
        root.opacity = 0.0;
    }

    function cleanUrls(urls) {
        var result = [];
        for (var i = 0; i < urls.length; ++i) {
            var s = urls[i].toString();
            if (s.startsWith("file://")) {
                s = s.substring(7);
            }
            result.push(s);
        }
        return result;
    }
}
