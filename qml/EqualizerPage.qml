import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Equalizador de 10 bandas, balanço/fader e loudness.
Rectangle {
    id: root
    signal closed()
    color: Theme.background
    MouseArea { anchors.fill: parent }

    function freqLabel(hz) { return hz >= 1000 ? (hz / 1000) + "k" : String(hz) }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Text { text: "Equalizador"; color: Theme.primary; font.pixelSize: Theme.fontTitle; Layout.fillWidth: true }
            Text { text: "Predefinição: " + equalizer.preset; color: Theme.textDim; font.pixelSize: Theme.fontSmall }
        }

        Flow {
            Layout.fillWidth: true
            spacing: 8
            Repeater {
                model: equalizer.presets
                delegate: IconButton {
                    required property string modelData
                    width: 112; height: 40; radius: 12; fontSize: 14
                    text: modelData
                    highlighted: equalizer.preset === modelData
                    onClicked: equalizer.applyPreset(modelData)
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            Card {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 3
                RowLayout {
                    anchors.fill: parent
                    spacing: 4
                    Repeater {
                        model: equalizer.frequencies
                        delegate: ColumnLayout {
                            required property int modelData
                            required property int index
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 2
                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                readonly property real g: equalizer.gains[index]
                                text: (g > 0 ? "+" : "") + g.toFixed(1).replace(".", ",")
                                color: Theme.text; font.pixelSize: Theme.fontSmall
                            }
                            Slider {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.fillHeight: true
                                orientation: Qt.Vertical
                                from: -12; to: 12; stepSize: 0.5
                                value: equalizer.gains[index]
                                onMoved: equalizer.setGain(index, value)
                            }
                            Text { Layout.alignment: Qt.AlignHCenter; text: root.freqLabel(modelData); color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                        }
                    }
                }
            }

            Card {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 2
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6
                    Text { text: "Balanço: " + (equalizer.balance < -0.02 ? "esquerda" : equalizer.balance > 0.02 ? "direita" : "centro"); color: Theme.text; font.pixelSize: Theme.fontSmall }
                    Slider { Layout.fillWidth: true; from: -1; to: 1; value: equalizer.balance; onMoved: equalizer.balance = value }
                    Text { text: "Fader: " + (equalizer.fader < -0.02 ? "trás" : equalizer.fader > 0.02 ? "frente" : "centro"); color: Theme.text; font.pixelSize: Theme.fontSmall }
                    Slider { Layout.fillWidth: true; from: -1; to: 1; value: equalizer.fader; onMoved: equalizer.fader = value }
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Loudness"; color: Theme.text; font.pixelSize: Theme.fontNormal; Layout.fillWidth: true }
                        Switch { checked: equalizer.loudness; onToggled: equalizer.loudness = checked }
                    }
                    Item { Layout.fillHeight: true }
                    Text {
                        Layout.fillWidth: true; wrapMode: Text.WordWrap
                        color: Theme.textDim; font.pixelSize: Theme.fontSmall
                        text: "No PC os ajustes são guardados mas ainda não alteram o som. Na central serão aplicados pelo DSP de áudio."
                    }
                }
            }
        }

        RowLayout {
            spacing: 8
            IconButton { width: 150; height: 44; radius: 12; fontSize: 14; text: "Restaurar"; onClicked: equalizer.reset() }
            IconButton { width: 130; height: 44; radius: 12; fontSize: 14; text: "Concluir"; highlighted: true; onClicked: root.closed() }
        }
    }
}
