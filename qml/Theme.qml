pragma Singleton
import QtQuick

// Paletas: escuro, claro e as versões em alto contraste (mesmos valores do headunit-android).
// Escolhidas em Configurações > Aparência (settings.dark / settings.highContrast).
QtObject {
    readonly property bool dark: settings.dark
    readonly property bool highContrast: settings.highContrast

    readonly property var palettes: ({
        dark: {
            background: "#0B0E13", surface: "#12161D", surfaceVariant: "#1B212A", surfaceHigh: "#252D38",
            primary: "#4FC3F7", primaryText: "#00222E", secondary: "#80CBC4",
            text: "#E6E9EF", textDim: "#9AA3B2", outline: "#3A4350", danger: "#F44336"
        },
        light: {
            background: "#F3F5F8", surface: "#FFFFFF", surfaceVariant: "#E6EAF0", surfaceHigh: "#D5DBE4",
            primary: "#0277BD", primaryText: "#FFFFFF", secondary: "#00796B",
            text: "#12161D", textDim: "#4A5260", outline: "#B8C0CC", danger: "#C62828"
        },
        darkHc: {
            background: "#000000", surface: "#000000", surfaceVariant: "#1C1C1C", surfaceHigh: "#333333",
            primary: "#FFD600", primaryText: "#000000", secondary: "#00E5FF",
            text: "#FFFFFF", textDim: "#FFFFFF", outline: "#FFFFFF", danger: "#FF5252"
        },
        lightHc: {
            background: "#FFFFFF", surface: "#FFFFFF", surfaceVariant: "#EDEDED", surfaceHigh: "#CFCFCF",
            primary: "#0033CC", primaryText: "#FFFFFF", secondary: "#6A00A8",
            text: "#000000", textDim: "#000000", outline: "#000000", danger: "#B00020"
        }
    })
    readonly property var p: palettes[(dark ? "dark" : "light") + (highContrast ? "Hc" : "")]

    readonly property color background: p.background
    readonly property color surface: p.surface
    readonly property color surfaceVariant: p.surfaceVariant
    readonly property color surfaceHigh: p.surfaceHigh
    readonly property color primary: p.primary
    readonly property color primaryText: p.primaryText
    readonly property color secondary: p.secondary
    readonly property color text: p.text
    readonly property color textDim: p.textDim
    readonly property color outline: p.outline
    readonly property color danger: p.danger
    /** Borda dos cartões e botões: só no alto contraste. */
    readonly property int borderWidth: highContrast ? 2 : 0

    readonly property int radius: 16
    readonly property int fontSmall: 13
    readonly property int fontNormal: 16
    readonly property int fontLarge: 22
    readonly property int fontTitle: 30
}
