import QtQuick
import QtQuick.Layouts

Item {
    component InfoRow: RowLayout {
        property string label
        property string value
        Layout.fillWidth: true
        Text { Layout.fillWidth: true; text: label; color: Theme.textDim; font.pixelSize: Theme.fontNormal }
        Text { text: value; color: Theme.text; font.pixelSize: Theme.fontLarge; font.bold: true }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                Text { text: "Estado do veículo"; color: Theme.text; font.pixelSize: Theme.fontLarge }
                InfoRow { label: "Fonte dos dados"; value: vehicle.name }
                InfoRow { label: "Ignição"; value: vehicle.ignitionLabel }
                InfoRow { label: "Marcha"; value: vehicle.gearLabel }
                InfoRow { label: "Velocidade"; value: vehicle.speedKmh.toFixed(0) + " km/h" }
                InfoRow { label: "Volante"; value: vehicle.steeringDeg.toFixed(0) + "°" }
                InfoRow { label: "Vídeo permitido"; value: app.videoAllowed ? "Sim" : "Não (em movimento)" }
                Item { Layout.fillHeight: true }
                Text {
                    Layout.fillWidth: true
                    visible: vehicle.simulated
                    wrapMode: Text.WordWrap
                    color: Theme.textDim; font.pixelSize: Theme.fontSmall
                    text: "Dados simulados: use a janela \"Simulador\" ou o teclado " +
                          "(R = ré, ←/→ = volante, N/B = próxima/anterior, Espaço = play, F = fonte, +/- = volume, M = mudo)."
                }
            }
        }

        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                Text { text: "Desempenho"; color: Theme.text; font.pixelSize: Theme.fontLarge }
                InfoRow { label: "Primeiro quadro"; value: perf.startupMs >= 0 ? perf.startupMs + " ms" : "…" }
                InfoRow { label: "Quadros por segundo"; value: perf.fps }
                InfoRow { label: "Memória (RAM)"; value: perf.memoryMb > 0 ? perf.memoryMb.toFixed(1) + " MB" : "indisponível" }
                Item { Layout.fillHeight: true }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    color: Theme.textDim; font.pixelSize: Theme.fontSmall
                    text: "O PC é bem mais rápido que a central: use estes números para comparar " +
                          "versões entre si. Veja docs/desempenho.md para limitar CPU e memória."
                }
            }
        }
    }
}
