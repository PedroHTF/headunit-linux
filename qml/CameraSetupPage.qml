import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import HeadUnit.Camera

// Calibração da câmera de ré com prévia ao vivo. Dica: cena "Grade 1 m".
Rectangle {
    id: root
    signal closed()
    color: Theme.background
    MouseArea { anchors.fill: parent }

    readonly property var v: cameraSettings.values
    property real previewSteering: 0

    component Header: Text {
        Layout.topMargin: 12
        color: Theme.primary; font.pixelSize: Theme.fontNormal; font.bold: true
    }
    component Adjust: ColumnLayout {
        property string label
        property string key
        property real from
        property real to
        property int decimals: 2
        property string unit: ""
        Layout.fillWidth: true
        spacing: 0
        RowLayout {
            Layout.fillWidth: true
            Text { Layout.fillWidth: true; text: label; color: Theme.text; font.pixelSize: Theme.fontSmall }
            Text { text: Number(root.v[key]).toFixed(decimals).replace(".", ",") + unit; color: Theme.text; font.pixelSize: Theme.fontSmall; font.bold: true }
        }
        Slider {
            Layout.fillWidth: true
            from: parent.from; to: parent.to
            value: root.v[parent.key]
            onMoved: cameraSettings.set(parent.key, value)
        }
    }
    component Toggle: RowLayout {
        property string label
        property string key
        Layout.fillWidth: true
        Text { Layout.fillWidth: true; text: label; color: Theme.text; font.pixelSize: Theme.fontSmall }
        Switch { checked: root.v[parent.key]; onToggled: cameraSettings.set(parent.key, checked) }
    }
    component Chips: Flow {
        id: chips
        property string key
        property var options: []
        property var labelOf: x => x
        Layout.fillWidth: true
        spacing: 6
        Repeater {
            model: parent.options
            delegate: IconButton {
                required property real modelData
                width: 76; height: 36; radius: 10; fontSize: 13
                text: chips.labelOf(modelData)
                highlighted: root.v[chips.key].indexOf(modelData) >= 0
                onClicked: cameraSettings.toggleInList(chips.key, modelData)
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: false
            Layout.preferredWidth: root.width * 0.55
            Layout.maximumWidth: root.width * 0.55
            spacing: 8
            ReverseCamera {
                Layout.preferredWidth: root.width * 0.55
                Layout.preferredHeight: root.width * 0.55 * 9 / 16
                settings: cameraSettings
                steeringDeg: root.previewSteering
            }
            Text { text: "Volante (prévia): " + root.previewSteering.toFixed(0) + "°"; color: Theme.text; font.pixelSize: Theme.fontSmall }
            Slider { Layout.fillWidth: true; from: -540; to: 540; value: root.previewSteering; onMoved: root.previewSteering = value }
            RowLayout {
                spacing: 8
                Text { text: "Cena:"; color: Theme.text; font.pixelSize: Theme.fontSmall }
                IconButton { width: 140; height: 36; radius: 10; fontSize: 13; text: "Estacionamento"; highlighted: root.v["img.scene"] === "parking"; onClicked: cameraSettings.set("img.scene", "parking") }
                IconButton { width: 110; height: 36; radius: 10; fontSize: 13; text: "Grade 1 m"; highlighted: root.v["img.scene"] === "grid"; onClicked: cameraSettings.set("img.scene", "grid") }
            }
            Item { Layout.fillHeight: true }
            RowLayout {
                spacing: 8
                IconButton { width: 170; height: 44; radius: 12; fontSize: 14; text: "Restaurar padrão"; onClicked: cameraSettings.reset() }
                IconButton { width: 130; height: 44; radius: 12; fontSize: 14; text: "Concluir"; highlighted: true; onClicked: root.closed() }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: Theme.surfaceVariant
            ScrollView {
                anchors.fill: parent
                anchors.margins: 12
                contentWidth: availableWidth
                ColumnLayout {
                    width: parent.width
                    spacing: 2

                    Header { text: "Imagem"; Layout.topMargin: 0 }
                    Toggle { label: "Remover fisheye"; key: "img.removeFisheye" }
                    Adjust { label: "Intensidade do fisheye"; key: "lens.fisheye"; from: 0; to: 0.95 }
                    Adjust { label: "Campo de visão da lente"; key: "lens.fov"; from: 90; to: 190; decimals: 0; unit: "°" }
                    Adjust { visible: root.v["img.removeFisheye"]; label: "Campo de visão após correção"; key: "img.correctedFov"; from: 60; to: 150; decimals: 0; unit: "°" }

                    Header { text: "Posição da câmera" }
                    Adjust { label: "Altura do chão"; key: "cam.height"; from: 0.3; to: 1.6; unit: " m" }
                    Adjust { label: "Inclinação para baixo"; key: "cam.pitch"; from: 0; to: 75; decimals: 0; unit: "°" }
                    Adjust { label: "Rotação lateral"; key: "cam.yaw"; from: -20; to: 20; decimals: 1; unit: "°" }
                    Adjust { label: "Giro da imagem"; key: "cam.roll"; from: -15; to: 15; decimals: 1; unit: "°" }
                    Adjust { label: "Deslocamento do centro"; key: "cam.offsetX"; from: -0.6; to: 0.6; unit: " m" }

                    Header { text: "Medidas do carro" }
                    Adjust { label: "Largura"; key: "car.width"; from: 1.4; to: 2.3; unit: " m" }
                    Adjust { label: "Comprimento (marca \"carros\")"; key: "car.length"; from: 3.0; to: 6.5; unit: " m" }
                    Adjust { label: "Entre-eixos"; key: "car.wheelbase"; from: 2.0; to: 3.6; unit: " m" }
                    Adjust { label: "Eixo traseiro → para-choque"; key: "car.overhang"; from: 0.4; to: 1.6; unit: " m" }
                    Adjust { label: "Relação volante/rodas"; key: "car.ratio"; from: 10; to: 22; decimals: 1; unit: " : 1" }
                    Adjust { label: "Ângulo máximo das rodas"; key: "car.maxWheel"; from: 25; to: 45; decimals: 0; unit: "°" }

                    Header { text: "Linhas" }
                    Toggle { label: "Linhas fixas (largura e distâncias)"; key: "lines.static" }
                    Toggle { label: "Trajetória pelo volante"; key: "lines.dynamic" }
                    Toggle { label: "Inverter lado da trajetória"; key: "lines.invert" }
                    Adjust { label: "Comprimento da trajetória"; key: "lines.dynLength"; from: 1; to: 8; decimals: 1; unit: " m" }
                    Adjust { label: "Ajuste lateral das linhas"; key: "lines.offsetX"; from: -0.4; to: 0.4; unit: " m" }
                    Adjust { label: "Ajuste de distância das linhas"; key: "lines.offsetY"; from: -0.6; to: 0.6; unit: " m" }
                    Text { text: "Marcas de distância"; color: Theme.text; font.pixelSize: Theme.fontSmall; Layout.topMargin: 6 }
                    Chips { key: "lines.markers"; options: [0.3, 0.5, 1, 1.5, 2, 3]; labelOf: x => String(x).replace(".", ",") + " m" }
                    Text { text: "Marcas em carros (usa o comprimento)"; color: Theme.text; font.pixelSize: Theme.fontSmall; Layout.topMargin: 6 }
                    Chips { key: "lines.carMarkers"; options: [1, 2, 3]; labelOf: x => x === 1 ? "1 carro" : x + " carros" }
                }
            }
        }
    }
}
