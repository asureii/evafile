import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    width: 360
    color: Theme.bgSurfaceGlass
    border.color: Theme.borderSubtle
    border.width: 1

    signal newDownloadRequested()

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
                text: "EVALINK ENGINE"
                font.pixelSize: 11
                font.bold: true
                font.letterSpacing: 1.5
                color: Theme.crimsonGlow
            }

            Rectangle {
                Layout.preferredHeight: 20
                Layout.preferredWidth: speedLabel.implicitWidth + 12
                radius: 10
                color: Qt.rgba(Theme.crimson.r, Theme.crimson.g, Theme.crimson.b, 0.25)
                border.color: Theme.borderLit
                border.width: 1

                Text {
                    id: speedLabel
                    anchors.centerIn: parent
                    text: controller.evalinkGlobalSpeed
                    color: Theme.textPrimary
                    font.pixelSize: 10
                    font.bold: true
                }
            }

            Item { Layout.fillWidth: true }

            IconButton {
                implicitWidth: 26
                implicitHeight: 26
                iconSize: 12
                iconSource: "qrc:/icons/close.svg"
                tooltipText: "Close Evalink"
                onClicked: controller.setEvalinkDrawerVisible(false)
            }
        }

        // Action Toolbar
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                Layout.fillWidth: true
                text: "+ Add Download"
                onClicked: root.newDownloadRequested()
                background: Rectangle {
                    color: parent.pressed ? Theme.bgCardActive : (parent.hovered ? Theme.bgCardHover : Theme.bgCard)
                    radius: Theme.radiusSmall
                    border.color: Theme.crimson
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
                Layout.preferredWidth: 100
                text: "Clear Done"
                onClicked: controller.purgeEvalink()
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

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.borderSubtle
        }

        // Downloads Task List
        ListView {
            id: taskList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: controller.evalinkTasks
            spacing: 8
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { active: true }

            delegate: Rectangle {
                width: taskList.width
                height: 80
                radius: Theme.radiusSmall
                color: Theme.bgCard
                border.color: Theme.borderSubtle
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Text {
                            text: modelData.filename
                            color: Theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                            Layout.fillWidth: true
                            elide: Text.ElideMiddle
                        }

                        // Status Badge
                        Rectangle {
                            height: 18
                            width: statusBadgeText.implicitWidth + 10
                            radius: 4
                            color: {
                                if (modelData.status === "active") return Qt.rgba(0.18, 0.8, 0.44, 0.2);
                                if (modelData.status === "paused") return Qt.rgba(0.95, 0.61, 0.07, 0.2);
                                if (modelData.status === "complete") return Qt.rgba(0.2, 0.6, 1.0, 0.2);
                                return Qt.rgba(0.9, 0.3, 0.2, 0.2);
                            }
                            border.color: {
                                if (modelData.status === "active") return Theme.statusSuccess;
                                if (modelData.status === "paused") return Theme.statusWarning;
                                if (modelData.status === "complete") return Theme.statusInfo;
                                return Theme.statusError;
                            }
                            border.width: 1

                            Text {
                                id: statusBadgeText
                                anchors.centerIn: parent
                                text: modelData.status.toUpperCase()
                                color: Theme.textPrimary
                                font.pixelSize: 9
                                font.bold: true
                            }
                        }
                    }

                    // Progress bar
                    Rectangle {
                        Layout.fillWidth: true
                        height: 6
                        radius: 3
                        color: Theme.bgBase

                        Rectangle {
                            height: parent.height
                            radius: 3
                            width: Math.min(parent.width, parent.width * (modelData.progressPercent / 100.0))
                            color: modelData.status === "active" ? Theme.crimsonGlow : Theme.crimson
                        }
                    }

                    // Telemetry and Controls
                    RowLayout {
                        Layout.fillWidth: true

                        Text {
                            text: modelData.completedLength + " / " + modelData.totalLength + " (" + modelData.progressPercent + "%)"
                            color: Theme.textMuted
                            font.pixelSize: 10
                        }

                        Item { Layout.fillWidth: true }

                        Text {
                            text: modelData.downloadSpeed
                            color: Theme.textSecondary
                            font.pixelSize: 10
                            font.bold: true
                            visible: modelData.status === "active"
                        }

                        // Pause / Resume button
                        IconButton {
                            implicitWidth: 20
                            implicitHeight: 20
                            iconSize: 10
                            iconSource: modelData.status === "paused" ? "qrc:/icons/refresh.svg" : "qrc:/icons/close.svg"
                            tooltipText: modelData.status === "paused" ? "Resume" : "Pause"
                            onClicked: {
                                if (modelData.status === "paused") {
                                    controller.resumeEvalink(modelData.gid);
                                } else {
                                    controller.pauseEvalink(modelData.gid);
                                }
                            }
                        }

                        // Remove button
                        IconButton {
                            implicitWidth: 20
                            implicitHeight: 20
                            iconSize: 10
                            iconSource: "qrc:/icons/trash.svg"
                            tooltipText: "Remove Task"
                            onClicked: controller.removeEvalink(modelData.gid)
                        }
                    }
                }
            }
        }

        // Empty state
        Item {
            visible: controller.evalinkTasks.length === 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            Column {
                anchors.centerIn: parent
                spacing: 8

                Image {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 44
                    height: 44
                    source: "qrc:/icons/disk.svg"
                    fillMode: Image.PreserveAspectFit
                    opacity: 0.3
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "No Active Downloads"
                    color: Theme.textMuted
                    font.pixelSize: 12
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Evalink is waiting for download links"
                    color: Theme.textDisabled
                    font.pixelSize: 10
                }
            }
        }
    }
}
