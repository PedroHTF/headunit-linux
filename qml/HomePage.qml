import QtQuick
import QtQuick.Layouts

Item {
    id: root
    signal navigate(int index)

    readonly property bool radioActive: app.audioSource === "radio"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 5

            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                Item { Layout.fillHeight: true }
                Text {
                    text: "Tocando agora · " + (root.radioActive ? "Rádio" : "Mídia")
                    color: Theme.primary; font.pixelSize: Theme.fontNormal
                }
                Text {
                    Layout.fillWidth: true
                    text: root.radioActive ? radio.frequencyLabel + " " + radio.unit
                                           : (player.hasMedia ? player.title : "Nada tocando")
                    color: Theme.text; font.pixelSize: Theme.fontTitle; font.bold: true
                    elide: Text.ElideRight
                }
                Text {
                    Layout.fillWidth: true
                    text: root.radioActive ? (radio.stationName || "Rádio")
                                           : (player.hasMedia ? player.subtitle : "Escolha algo em Mídia ou Rádio")
                    color: Theme.textDim; font.pixelSize: Theme.fontNormal
                    elide: Text.ElideRight
                }
                Item { Layout.preferredHeight: 16 }
                // Os mesmos comandos dos botões do volante: valem para a fonte ativa.
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 32
                    IconButton { text: "|◀"; onClicked: app.pressKey("previous") }
                    IconButton {
                        size: 84
                        highlighted: true
                        text: (root.radioActive ? radio.on : player.playing) ? "❚❚" : "▶"
                        onClicked: app.pressKey("playpause")
                    }
                    IconButton { text: "▶|"; onClicked: app.pressKey("next") }
                }
                Item { Layout.fillHeight: true }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 4
            columns: 2
            rowSpacing: 16
            columnSpacing: 16

            Repeater {
                model: [
                    { label: "Mídia", glyph: "♫", page: 1 },
                    { label: "Rádio", glyph: "≋", page: 2 },
                    { label: "Veículo", glyph: "◈", page: 3 },
                    { label: "Projeção", glyph: "⧉", page: 4 }
                ]
                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: Theme.radius
                    color: tileMouse.pressed ? Theme.surfaceHigh : Theme.surfaceVariant
                    Column {
                        anchors.centerIn: parent
                        spacing: 6
                        Text { anchors.horizontalCenter: parent.horizontalCenter; text: modelData.glyph; color: Theme.primary; font.pixelSize: 48 }
                        Text { anchors.horizontalCenter: parent.horizontalCenter; text: modelData.label; color: Theme.text; font.pixelSize: Theme.fontLarge }
                    }
                    MouseArea { id: tileMouse; anchors.fill: parent; onClicked: root.navigate(modelData.page) }
                }
            }
        }
    }
}
