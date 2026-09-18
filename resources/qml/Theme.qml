pragma Singleton
import QtQuick

QtObject {
    id: theme

    // Palette: Crimson Flame
    readonly property color bgVoid: "#0d0204"
    readonly property color bgBase: "#140406"
    readonly property color bgSurface: "#1e0609"
    readonly property color bgSurfaceGlass: Qt.rgba(0.12, 0.03, 0.04, 0.82)
    readonly property color bgCard: "#26080b"
    readonly property color bgCardHover: "#350c10"
    readonly property color bgCardActive: "#451015"
    readonly property color bgInput: "#180507"

    // Accents
    readonly property color crimson: "#cf2824"
    readonly property color crimsonGlow: "#e32a10"
    readonly property color crimsonHover: "#f03218"
    readonly property color flameOrange: "#ff5e1a"
    readonly property color accentCyan: "#38ef7d"

    // Borders & Glass
    readonly property color borderSubtle: "#3b1114"
    readonly property color borderHighlight: "#54191e"
    readonly property color borderLit: Qt.rgba(0.81, 0.16, 0.14, 0.6)
    readonly property color borderGlass: Qt.rgba(1.0, 1.0, 1.0, 0.08)
    readonly property color rimTop: Qt.rgba(1.0, 1.0, 1.0, 0.14)

    // Typography
    readonly property color textPrimary: "#f5e8e8"
    readonly property color textSecondary: "#b88a8d"
    readonly property color textMuted: "#7a5255"
    readonly property color textDisabled: "#4e3134"

    // Semantic States
    readonly property color statusSuccess: "#2ecc71"
    readonly property color statusWarning: "#f39c12"
    readonly property color statusError: "#e74c3c"
    readonly property color statusInfo: "#3498db"

    // Radii & Dimensions
    readonly property real radiusSmall: 6
    readonly property real radiusMedium: 10
    readonly property real radiusLarge: 14
    readonly property real radiusPill: 999

    // Animation Durations & Easing
    readonly property int animFast: 150
    readonly property int animNormal: 240
    readonly property int animSlow: 350
    readonly property var easeDecel: Easing.OutCubic
    readonly property var easeSpring: Easing.OutBack
    readonly property var easeLinear: Easing.Linear
}
