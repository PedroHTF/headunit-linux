import QtQuick

Rectangle {
    id: root
    property int currentIndex: startPage
    readonly property var pages: [
        { label: "Início", glyph: "⌂" },
        { label: "Mídia", glyph: "♫" },
        { label: "Rádio", glyph: "≋" },
        { label: "Veículo", glyph: "◈" },
        { label: "Projeção", glyph: "⧉" },
        { label: "Config.", glyph: "⚙" }
    ]
    color: Theme.surface

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 12
        spacing: 8

        Repeater {
            model: root.pages
            delegate: Rectangle {
                required property var modelData
                required property int index
                width: 80
                height: 72
                radius: 14
                color: root.currentIndex === index ? Theme.surfaceHigh : "transparent"

                Column {
                    anchors.centerIn: parent
                    spacing: 2
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.glyph
                        font.pixelSize: 28
                        color: root.currentIndex === index ? Theme.primary : Theme.textDim
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.label
                        font.pixelSize: Theme.fontSmall
                        color: root.currentIndex === index ? Theme.text : Theme.textDim
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: root.currentIndex = index
                }
            }
        }
    }
}
