pragma Singleton
import QtQuick

// Tema escuro fixo: no carro, à noite, tela clara ofusca.
QtObject {
    readonly property color background: "#0B0E13"
    readonly property color surface: "#12161D"
    readonly property color surfaceVariant: "#1B212A"
    readonly property color surfaceHigh: "#252D38"
    readonly property color primary: "#4FC3F7"
    readonly property color primaryText: "#00222E"
    readonly property color secondary: "#80CBC4"
    readonly property color text: "#E6E9EF"
    readonly property color textDim: "#9AA3B2"
    readonly property color danger: "#F44336"
    readonly property int radius: 16
    readonly property int fontSmall: 13
    readonly property int fontNormal: 16
    readonly property int fontLarge: 22
    readonly property int fontTitle: 30
}
