import QtQuick
import QtQuick.Layouts

Rectangle {
    color: Theme.background

    property string clock: Qt.formatTime(new Date(), "HH:mm")
    Timer {
        interval: 1000; running: true; repeat: true
        onTriggered: parent.clock = Qt.formatTime(new Date(), "HH:mm")
    }

    readonly property var sourceNames: ({ none: "—", media: "Mídia", radio: "Rádio", projection: "Celular" })

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 18

        Text { text: clock; color: Theme.text; font.pixelSize: Theme.fontLarge; font.bold: true }
        Item { Layout.fillWidth: true }
        Text { text: "Fonte: " + sourceNames[app.audioSource]; color: Theme.text; font.pixelSize: Theme.fontSmall }
        Text { text: "Marcha " + vehicle.gearLabel; color: Theme.text; font.pixelSize: Theme.fontSmall }
        Text { text: vehicle.speedKmh.toFixed(0) + " km/h"; color: Theme.text; font.pixelSize: Theme.fontSmall }
        Text {
            text: "Vol " + Math.round(player.volume * 100) + (player.muted ? " (mudo)" : "")
            color: Theme.text; font.pixelSize: Theme.fontSmall
        }
        Text {
            text: perf.fps + " fps · " + (perf.memoryMb > 0 ? perf.memoryMb.toFixed(0) + " MB" : "?")
            color: Theme.textDim; font.pixelSize: Theme.fontSmall
        }
        Text {
            text: "Veículo: " + vehicle.name
            color: vehicle.simulated ? Theme.secondary : Theme.primary
            font.pixelSize: Theme.fontSmall
        }
    }
}
