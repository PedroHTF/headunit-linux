import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

// Painel de simulação: tudo que, na central, viria do carro (MCU/CAN) ou do pendrive.
Window {
    id: sim
    property Window mainWindow
    width: 400
    height: 600
    x: mainWindow ? mainWindow.x + mainWindow.width + 12 : 0
    y: mainWindow ? mainWindow.y : 0
    title: "Simulador"
    color: Theme.surface

    component SectionTitle: Text { color: Theme.primary; font.pixelSize: Theme.fontNormal; font.bold: true }
    component Choice: IconButton {
        width: 64; height: 44; radius: 10; fontSize: 15
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: 16
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 10

            SectionTitle { text: "Ignição / ACC" }
            Row {
                spacing: 8
                Repeater {
                    model: [["OFF", "Desligado"], ["ACC", "ACC"], ["ON", "Ligado"]]
                    delegate: Choice {
                        required property var modelData
                        width: 104
                        text: modelData[1]
                        highlighted: vehicle.ignitionLabel === modelData[1]
                        onClicked: simVehicle.setIgnitionByName(modelData[0])
                    }
                }
            }

            SectionTitle { text: "Marcha (R abre a câmera de ré)" }
            Row {
                spacing: 8
                Repeater {
                    model: ["P", "R", "N", "D"]
                    delegate: Choice {
                        required property string modelData
                        text: modelData
                        highlighted: vehicle.gearLabel === modelData
                        onClicked: simVehicle.setGearByName(modelData)
                    }
                }
            }

            SectionTitle { text: "Velocidade: " + vehicle.speedKmh.toFixed(0) + " km/h" }
            Slider {
                Layout.fillWidth: true
                from: 0; to: 200
                value: vehicle.speedKmh
                onMoved: simVehicle.setSpeed(value)
            }

            SectionTitle { text: "Botões do volante" }
            GridLayout {
                columns: 4
                columnSpacing: 8
                rowSpacing: 8
                Repeater {
                    model: [["previous", "|◀"], ["playpause", "▶❚❚"], ["next", "▶|"], ["mode", "Fonte"],
                            ["voldown", "Vol−"], ["volup", "Vol+"], ["mute", "Mudo"]]
                    delegate: Choice {
                        required property var modelData
                        width: 80
                        text: modelData[1]
                        onClicked: simVehicle.pressKey(modelData[0])
                    }
                }
            }

            SectionTitle { text: "Pendrive USB" }
            RowLayout {
                Layout.fillWidth: true
                Switch {
                    checked: library.mounted
                    onToggled: library.mounted = checked
                }
                Text { text: library.mounted ? "Conectado" : "Removido"; color: Theme.text; font.pixelSize: Theme.fontNormal }
                Item { Layout.fillWidth: true }
                Button {
                    text: "Abrir pasta"
                    onClicked: {
                        const p = library.rootPath.replace(/\\/g, "/")
                        Qt.openUrlExternally(p.startsWith("/") ? "file://" + p : "file:///" + p)
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                text: library.rootPath
                color: Theme.textDim; font.pixelSize: Theme.fontSmall
                wrapMode: Text.WrapAnywhere
            }

            SectionTitle { text: "Desempenho" }
            Text {
                color: Theme.text; font.pixelSize: Theme.fontSmall
                text: "Primeiro quadro: " + (perf.startupMs >= 0 ? perf.startupMs + " ms" : "…") +
                      "\nQuadros/s: " + perf.fps +
                      "\nMemória: " + (perf.memoryMb > 0 ? perf.memoryMb.toFixed(1) + " MB" : "indisponível")
            }

            SectionTitle { text: "Teclado (janela principal)" }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: Theme.textDim; font.pixelSize: Theme.fontSmall
                text: "R ré · N próxima · B anterior · Espaço play/pause · F fonte · +/− volume · M mudo"
            }
        }
    }
}
