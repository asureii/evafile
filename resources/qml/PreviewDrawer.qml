import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    width: 320
    color: Theme.bgSurfaceGlass
    border.color: Theme.borderSubtle
    border.width: 1

    // Top rim highlight
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: Theme.rimTop
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        // Drawer Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "INSPECTION"
                font.pixelSize: 11
                font.bold: true
                font.letterSpacing: 1.5
                color: Theme.crimsonGlow
                Layout.fillWidth: true
            }

            IconButton {
                implicitWidth: 26
                implicitHeight: 26
                iconSize: 12
                iconSource: "qrc:/icons/close.svg"
                tooltipText: "Close Preview"
                onClicked: controller.setPreviewVisible(false)
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.borderSubtle
        }

        // Preview viewport
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 220
            color: Theme.bgCard
            radius: Theme.radiusMedium
            border.color: Theme.borderSubtle
            border.width: 1
            clip: true

            // Image Preview
            Image {
                visible: controller.previewIsImage
                anchors.fill: parent
                anchors.margins: 8
                source: controller.previewIsImage ? "file://" + controller.previewPath : ""
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                smooth: true
            }

            // Text Preview
            Flickable {
                visible: !controller.previewIsImage && controller.previewContent.length > 0
                anchors.fill: parent
                anchors.margins: 10
                contentWidth: textContent.implicitWidth
                contentHeight: textContent.implicitHeight
                boundsBehavior: Flickable.StopAtBounds
                clip: true

                Text {
                    id: textContent
                    text: controller.previewContent
                    color: Theme.textSecondary
                    font.family: "monospace"
                    font.pixelSize: 10
                    wrapMode: Text.WrapAnywhere
                }
            }

            // Generic / Media Placeholder
            Column {
                visible: !controller.previewIsImage && controller.previewContent.length === 0
                anchors.centerIn: parent
                spacing: 8

                Image {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 48
                    height: 48
                    source: controller.previewIsMedia ? "qrc:/icons/file-video.svg" : "qrc:/icons/file.svg"
                    fillMode: Image.PreserveAspectFit
                    opacity: 0.6
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: controller.previewIsMedia ? "Media Preview Unavailable" : "No Quick Preview"
                    color: Theme.textMuted
                    font.pixelSize: 11
                }
            }
        }

        // Metadata Fields
        Column {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: controller.previewTitle
                color: Theme.textPrimary
                font.pixelSize: 14
                font.bold: true
                elide: Text.ElideMiddle
                width: parent.width
            }

            RowLayout {
                width: parent.width
                Text { text: "SIZE:"; color: Theme.textMuted; font.pixelSize: 10; font.bold: true; Layout.preferredWidth: 65 }
                Text { text: controller.previewSize; color: Theme.textSecondary; font.pixelSize: 11; Layout.fillWidth: true }
            }

            RowLayout {
                width: parent.width
                Text { text: "SHA-256:"; color: Theme.textMuted; font.pixelSize: 10; font.bold: true; Layout.preferredWidth: 65 }
                Text { text: controller.previewChecksum; color: Theme.textSecondary; font.family: "monospace"; font.pixelSize: 10; Layout.fillWidth: true; elide: Text.ElideRight }
            }

            RowLayout {
                width: parent.width
                Text { text: "PATH:"; color: Theme.textMuted; font.pixelSize: 10; font.bold: true; Layout.preferredWidth: 65 }
                Text { text: controller.previewPath; color: Theme.textSecondary; font.pixelSize: 10; Layout.fillWidth: true; elide: Text.ElideMiddle }
            }
        }

        Item { Layout.fillHeight: true }

        // Action Buttons
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Button {
                Layout.fillWidth: true
                text: "Open with Default App"
                onClicked: controller.openFile(controller.previewPath)
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
                Layout.fillWidth: true
                text: "Open in EvaTerm"
                onClicked: controller.openInEvaTerm(controller.previewPath)
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

            Button {
                Layout.fillWidth: true
                text: "Copy Location"
                onClicked: controller.copyLocation(controller.previewPath)
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
        }
    }
}
