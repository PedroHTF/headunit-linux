import QtQuick

// Tela preta com o ACC desligado, como na central real.
Rectangle {
    color: "black"
    MouseArea { anchors.fill: parent }

    Column {
        anchors.centerIn: parent
        spacing: 16
        Text { anchors.horizontalCenter: parent.horizontalCenter; text: "ACC desligado"; color: "gray"; font.pixelSize: Theme.fontNormal }
        IconButton {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: vehicle.simulated
            width: 260; height: 52; radius: 26; fontSize: 16
            text: "Ligar ignição (simulador)"
            highlighted: true
            onClicked: simVehicle.setIgnitionByName("ON")
        }
    }
}
