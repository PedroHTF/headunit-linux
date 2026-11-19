import QtQuick
import QtQuick.Layouts

Item {
    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 2
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Repeater {
                    model: ["FM", "AM"]
                    delegate: IconButton {
                        required property string modelData
                        size: 48; fontSize: 16
                        text: modelData
                        highlighted: radio.band === modelData
                        onClicked: radio.setBand(modelData)
                    }
                }
                Item { Layout.fillWidth: true }
                Text { visible: radio.simulated; text: "simulado, sem áudio"; color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                IconButton {
                    size: 48; fontSize: 14
                    text: radio.on ? "ON" : "OFF"
                    highlighted: radio.on
                    onClicked: {
                        if (radio.on) {
                            radio.powerOff()
                        } else {
                            app.onRadioStarted()
                            radio.powerOn()
                        }
                    }
                }
            }

            Card {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Column {
                    anchors.centerIn: parent
                    spacing: 6
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 8
                        Text {
                            text: radio.frequencyLabel
                            font.pixelSize: 80; font.bold: true
                            color: Theme.primary
                            opacity: radio.on ? 1 : 0.4
                        }
                        Text { anchors.baseline: parent.children[0].baseline; text: radio.unit; color: Theme.text; font.pixelSize: Theme.fontLarge }
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: Theme.text; font.pixelSize: Theme.fontLarge
                        text: radio.seeking ? "Buscando…" : !radio.on ? "Rádio desligado" : (radio.stationName || "Sem estação")
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: radio.on && !radio.seeking && radio.rdsText !== ""
                        text: radio.rdsText
                        color: Theme.textDim; font.pixelSize: Theme.fontNormal
                    }
                    Row { // barras de sinal
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 3
                        Repeater {
                            model: 5
                            delegate: Rectangle {
                                required property int index
                                anchors.bottom: parent.bottom
                                width: 8; height: 6 * (index + 1); radius: 2
                                color: radio.signal >= (index + 1) * 18 ? Theme.primary : "#33888888"
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 28
                IconButton { text: "◀◀"; size: 72; onClicked: radio.seekDown() }
                IconButton { text: "‹"; size: 72; fontSize: 40; onClicked: radio.stepDown() }
                IconButton { text: "›"; size: 72; fontSize: 40; onClicked: radio.stepUp() }
                IconButton { text: "▶▶"; size: 72; onClicked: radio.seekUp() }
            }

            Text { text: "Memórias (toque: sintonizar · segure: salvar)"; color: Theme.textDim; font.pixelSize: Theme.fontSmall }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Repeater {
                    model: radio.presets
                    delegate: Rectangle {
                        required property int modelData
                        required property int index
                        Layout.fillWidth: true
                        height: 52
                        radius: 12
                        readonly property bool selected: modelData === radio.frequencyKhz
                        color: selected ? Theme.primary : Theme.surfaceVariant
                        Text {
                            anchors.centerIn: parent
                            text: modelData > 0 ? radio.formatFrequency(modelData) : "—"
                            color: parent.selected ? Theme.primaryText : Theme.text
                            font.pixelSize: Theme.fontNormal; font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: radio.recallPreset(index)
                            onPressAndHold: radio.savePreset(index)
                        }
                    }
                }
            }
        }

        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            padding: 12

            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Text { Layout.fillWidth: true; text: "Estações"; color: Theme.text; font.pixelSize: Theme.fontLarge }
                    IconButton { size: 44; fontSize: 14; text: "Busca"; onClicked: radio.scan() }
                }
                Text {
                    visible: radio.foundStations.length === 0
                    text: "Toque em Busca para procurar."
                    color: Theme.textDim; font.pixelSize: Theme.fontSmall
                }
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 4
                    model: radio.foundStations
                    delegate: Rectangle {
                        required property var modelData
                        width: ListView.view.width
                        height: 44
                        radius: 10
                        color: stMouse.pressed ? Theme.surfaceHigh : Theme.surface
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            text: modelData.label + "   " + modelData.name
                            color: Theme.text; font.pixelSize: Theme.fontNormal
                        }
                        MouseArea { id: stMouse; anchors.fill: parent; onClicked: radio.tune(modelData.frequencyKhz) }
                    }
                }
            }
        }
    }
}
