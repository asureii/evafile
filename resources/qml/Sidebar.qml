import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    width: 220
    color: Theme.bgSurfaceGlass
    border.color: Theme.borderSubtle
    border.width: 1

    // Top rim specular highlight
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: Theme.rimTop
    }

    Flickable {
        anchors.fill: parent
        anchors.bottomMargin: 50
        contentHeight: contentColumn.height + 20
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        Column {
            id: contentColumn
            width: parent.width
            spacing: 16
            topPadding: 12

            // Section: Places
            Column {
                width: parent.width
                spacing: 2

                Text {
                    text: "PLACES"
                    font.pixelSize: 10
                    font.letterSpacing: 1.5
                    font.bold: true
                    color: Theme.textMuted
                    leftPadding: 16
                    bottomPadding: 4
                }

                Repeater {
                    model: controller.places

                    delegate: Rectangle {
                        id: placeItem
                        width: parent.width - 16
                        height: 32
                        anchors.horizontalCenter: parent.horizontalCenter
                        radius: Theme.radiusSmall
                        readonly property bool isCurrent: controller.currentPath === modelData.path
                        color: isCurrent ? Qt.rgba(Theme.crimson.r, Theme.crimson.g, Theme.crimson.b, 0.2) : (mousePlace.containsMouse ? Theme.bgCardHover : "transparent")
                        border.color: isCurrent ? Theme.borderLit : "transparent"
                        border.width: 1

                        Behavior on color { ColorAnimation { duration: Theme.animFast } }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 10

                            Image {
                                width: 16
                                height: 16
                                source: "qrc:/icons/" + modelData.icon + ".svg"
                                fillMode: Image.PreserveAspectFit
                                opacity: placeItem.isCurrent ? 1.0 : 0.8
                            }

                            Text {
                                text: modelData.name
                                color: placeItem.isCurrent ? Theme.textPrimary : Theme.textSecondary
                                font.pixelSize: 12
                                font.bold: placeItem.isCurrent
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                        }

                        MouseArea {
                            id: mousePlace
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: controller.navigateTo(modelData.path)
                        }
                    }
                }
            }

            // Section: Storage Drives
            Column {
                width: parent.width
                spacing: 6
                visible: controller.drives.length > 0

                Text {
                    text: "DRIVES"
                    font.pixelSize: 10
                    font.letterSpacing: 1.5
                    font.bold: true
                    color: Theme.textMuted
                    leftPadding: 16
                    bottomPadding: 2
                }

                Repeater {
                    model: controller.drives

                    delegate: Rectangle {
                        id: driveItem
                        width: parent.width - 16
                        height: 52
                        anchors.horizontalCenter: parent.horizontalCenter
                        radius: Theme.radiusSmall
                        color: mouseDrive.containsMouse ? Theme.bgCardHover : Theme.bgCard
                        border.color: Theme.borderSubtle
                        border.width: 1

                        Column {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4

                            RowLayout {
                                width: parent.width
                                Image {
                                    width: 14
                                    height: 14
                                    source: "qrc:/icons/disk.svg"
                                    fillMode: Image.PreserveAspectFit
                                }
                                Text {
                                    text: modelData.name
                                    color: Theme.textPrimary
                                    font.pixelSize: 11
                                    font.bold: true
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }
                                Text {
                                    text: modelData.usedPercent + "%"
                                    color: modelData.usedPercent > 85 ? Theme.crimsonGlow : Theme.textMuted
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                            }

                            // Storage gauge bar
                            Rectangle {
                                width: parent.width
                                height: 4
                                radius: 2
                                color: Theme.bgBase

                                Rectangle {
                                    height: parent.height
                                    radius: 2
                                    width: Math.min(parent.width, parent.width * (modelData.usedPercent / 100.0))
                                    color: modelData.usedPercent > 90 ? Theme.statusError : (modelData.usedPercent > 75 ? Theme.flameOrange : Theme.crimson)
                                }
                            }
                        }

                        MouseArea {
                            id: mouseDrive
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: controller.navigateTo(modelData.path)
                        }
                    }
                }
            }

            // Section: Bookmarks
            Column {
                width: parent.width
                spacing: 2
                visible: controller.bookmarks.length > 0

                Text {
                    text: "BOOKMARKS"
                    font.pixelSize: 10
                    font.letterSpacing: 1.5
                    font.bold: true
                    color: Theme.textMuted
                    leftPadding: 16
                    bottomPadding: 4
                }

                Repeater {
                    model: controller.bookmarks

                    delegate: Rectangle {
                        id: bookmarkItem
                        width: parent.width - 16
                        height: 30
                        anchors.horizontalCenter: parent.horizontalCenter
                        radius: Theme.radiusSmall
                        color: mouseBookmark.containsMouse ? Theme.bgCardHover : "transparent"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 6
                            spacing: 8

                            Image {
                                width: 14
                                height: 14
                                source: "qrc:/icons/bookmark.svg"
                                fillMode: Image.PreserveAspectFit
                            }

                            Text {
                                text: {
                                    var parts = modelData.split("/");
                                    return parts[parts.length - 1] || modelData;
                                }
                                color: Theme.textSecondary
                                font.pixelSize: 11
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            // Remove bookmark button on hover
                            IconButton {
                                visible: mouseBookmark.containsMouse
                                implicitWidth: 20
                                implicitHeight: 20
                                iconSize: 10
                                iconSource: "qrc:/icons/close.svg"
                                tooltipText: "Remove Bookmark"
                                onClicked: controller.removeBookmark(modelData)
                            }
                        }

                        MouseArea {
                            id: mouseBookmark
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: controller.navigateTo(modelData)
                        }
                    }
                }
            }
        }
    }

    // Bottom Action Dock
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 48
        color: Theme.bgSurface
        border.color: Theme.borderSubtle
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 4

            IconButton {
                iconSource: "qrc:/icons/bookmark.svg"
                tooltipText: "Bookmark Current Directory"
                onClicked: controller.addBookmark(controller.currentPath)
            }

            IconButton {
                iconSource: "qrc:/icons/terminal.svg"
                tooltipText: "Open EvaTerm Here (F4)"
                onClicked: controller.openInEvaTerm()
            }

            IconButton {
                iconSource: "qrc:/icons/sort.svg"
                tooltipText: "Run EvaSort Clean"
                onClicked: controller.sortWithEvaSort()
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "EvaSuite"
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 1.0
                color: Theme.textMuted
                anchors.verticalCenter: parent.verticalCenter
                rightPadding: 6
            }
        }
    }
}
