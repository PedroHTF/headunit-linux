import QtQuick
import QtQuick.Layouts
import HeadUnit.Projection

// CarPlay / Android Auto pelo dongle USB. Com o celular conectado, mostra a tela dele.
Item {
    id: page
    readonly property bool connected: projection.state === "connected"

    // ---- Celular projetando
    Item {
        anchors.fill: parent
        visible: page.connected

        ProjectionView {
            id: view
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom; right: strip.left }
            controller: projection
            MouseArea {
                anchors.fill: parent
                function send(action, mouse) {
                    projection.touch(action, Math.max(0, Math.min(1, mouse.x / width)), Math.max(0, Math.min(1, mouse.y / height)))
                }
                onPressed: mouse => send("down", mouse)
                onPositionChanged: mouse => send("move", mouse)
                onReleased: mouse => send("up", mouse)
            }
        }

        // Atalhos para o celular (o resto se faz na própria tela dele).
        Rectangle {
            id: strip
            anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
            width: 64
            color: Theme.surface
            Column {
                anchors.centerIn: parent
                spacing: 12
                IconButton { size: 52; fontSize: 22; text: "⌂"; onClicked: projection.key("home") }
                IconButton { size: 52; fontSize: 22; text: "‹"; visible: projection.phoneType === "Android Auto"; onClicked: projection.key("back") }
                IconButton { size: 52; fontSize: 18; text: "🎙"; onClicked: projection.key("siri") }
            }
            Text {
                anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 8 }
                text: projection.fps + " fps"; color: Theme.textDim; font.pixelSize: 11
            }
        }
    }

    // ---- Sem celular: estado do dongle
    Card {
        anchors.fill: parent
        anchors.margins: 16
        visible: !page.connected
        ColumnLayout {
            anchors.centerIn: parent
            width: parent.width * 0.8
            spacing: 12
            Text { Layout.alignment: Qt.AlignHCenter; text: "⧉"; color: Theme.primary; font.pixelSize: 72 }
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: projection.state === "ready" ? "Dongle pronto" : "Procurando o dongle"
                color: Theme.text; font.pixelSize: Theme.fontTitle
            }
            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                color: Theme.textDim; font.pixelSize: Theme.fontNormal
                text: projection.state === "ready"
                      ? "Conecte o celular: iPhone abre o CarPlay, Android abre o Android Auto. Pelo cabo no dongle, ou sem fio depois de parear pelo Bluetooth."
                      : "Ligue o dongle CarPlay/Android Auto (tipo Carlinkit) numa porta USB da central."
            }
            Text {
                Layout.fillWidth: true
                visible: projection.lastError !== ""
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: projection.lastError; color: Theme.danger; font.pixelSize: Theme.fontSmall
            }
            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                color: Theme.textDim; font.pixelSize: Theme.fontSmall
                text: (projection.simulated ? "Dongle simulado: conecte um celular pela janela Simulador. " : "Dongle USB real. ")
                      + (projection.decoderAvailable ? "" : "Atenção: compilado sem FFmpeg, o vídeo do dongle real não aparece.")
            }
        }
    }
}
