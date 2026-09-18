import QtQuick
import QtQuick.Controls

Item {
    id: root

    property url iconSource: ""
    property string tooltipText: ""
    property bool isActive: false
    property real iconSize: 18
    property int badgeCount: 0
    property color badgeColor: Theme.crimson
    property color activeColor: Theme.crimson

    signal clicked()

    implicitWidth: 36
    implicitHeight: 36

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: Theme.radiusSmall
        color: {
            if (root.isActive) return Qt.rgba(Theme.crimson.r, Theme.crimson.g, Theme.crimson.b, 0.22);
            if (mouseArea.pressed) return Theme.bgCardActive;
            if (mouseArea.containsMouse) return Theme.bgCardHover;
            return "transparent";
        }
        border.color: root.isActive ? Theme.borderLit : (mouseArea.containsMouse ? Theme.borderHighlight : "transparent")
        border.width: 1

        scale: mouseArea.pressed ? 0.92 : (mouseArea.containsMouse ? 1.05 : 1.0)
        Behavior on scale { NumberAnimation { duration: Theme.animFast; easing.type: Theme.easeDecel } }
        Behavior on color { ColorAnimation { duration: Theme.animFast } }
        Behavior on border.color { ColorAnimation { duration: Theme.animFast } }

        Image {
            id: icon
            anchors.centerIn: parent
            width: root.iconSize
            height: root.iconSize
            source: root.iconSource
            sourceSize.width: root.iconSize * 2
            sourceSize.height: root.iconSize * 2
            fillMode: Image.PreserveAspectFit
            opacity: root.enabled ? (root.isActive || mouseArea.containsMouse ? 1.0 : 0.75) : 0.3
            Behavior on opacity { NumberAnimation { duration: Theme.animFast } }
        }

        // Active indicator dot
        Rectangle {
            visible: root.isActive
            width: 4
            height: 4
            radius: 2
            color: Theme.crimsonGlow
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 3
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // Badge counter (e.g. for downloads)
        Rectangle {
            visible: root.badgeCount > 0
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 2
            anchors.rightMargin: 2
            width: Math.max(14, badgeLabel.implicitWidth + 6)
            height: 14
            radius: 7
            color: root.badgeColor

            Text {
                id: badgeLabel
                anchors.centerIn: parent
                text: root.badgeCount > 99 ? "99+" : root.badgeCount.toString()
                color: "#ffffff"
                font.pixelSize: 9
                font.bold: true
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    // Lightweight ToolTip
    ToolTip {
        id: toolTip
        visible: mouseArea.containsMouse && root.tooltipText.length > 0
        delay: 500
        timeout: 4000
        text: root.tooltipText
        contentItem: Text {
            text: toolTip.text
            color: Theme.textPrimary
            font.pixelSize: 11
        }
        background: Rectangle {
            color: Theme.bgSurface
            radius: 4
            border.color: Theme.borderSubtle
            border.width: 1
        }
    }
}
