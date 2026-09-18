import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    height: 28
    color: Theme.bgSurfaceGlass
    border.color: Theme.borderSubtle
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 12

        // Left: Operation progress or Status
        RowLayout {
            spacing: 8
            visible: controller.opRunning

            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: Theme.crimsonGlow

                SequentialAnimation on opacity {
                    loops: Animation.Infinite
                    running: controller.opRunning
                    NumberAnimation { from: 0.3; to: 1.0; duration: 500; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 1.0; to: 0.3; duration: 500; easing.type: Easing.InOutQuad }
                }
            }

            Text {
                text: controller.opText
                color: Theme.textPrimary
                font.pixelSize: 11
                font.bold: true
            }

            Rectangle {
                width: 120
                height: 4
                radius: 2
                color: Theme.bgBase

                Rectangle {
                    height: parent.height
                    radius: 2
                    width: Math.min(parent.width, parent.width * (controller.opPercent / 100.0))
                    color: Theme.crimsonGlow
                }
            }
        }

        RowLayout {
            spacing: 6
            visible: !controller.opRunning
            Layout.fillWidth: true

            Rectangle {
                width: 6
                height: 6
                radius: 3
                color: Theme.statusSuccess
            }

            Text {
                text: controller.statusText
                color: Theme.textSecondary
                font.pixelSize: 11
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }

        Item { Layout.fillWidth: true; visible: controller.opRunning }

        // Free disk space
        Text {
            text: controller.freeSpaceText
            color: Theme.textMuted
            font.pixelSize: 10
            font.bold: true
            font.letterSpacing: 0.5
        }
    }
}
