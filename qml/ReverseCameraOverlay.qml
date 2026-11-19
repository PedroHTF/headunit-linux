import QtQuick
import HeadUnit.Camera

// Câmera de ré em tela cheia (abre sozinha com a ré engatada).
Rectangle {
    color: "black"
    MouseArea { anchors.fill: parent } // bloqueia toques nas telas de baixo

    ReverseCamera {
        anchors.fill: parent
        settings: cameraSettings
        steeringDeg: vehicle.steeringDeg
    }
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 12
        text: "CÂMERA DE RÉ (simulada)"
        color: "white"; font.pixelSize: Theme.fontNormal
        style: Text.Outline; styleColor: "black"
    }
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        text: "Verifique o entorno antes de manobrar"
        color: "#B3FFFFFF"; font.pixelSize: Theme.fontSmall
        style: Text.Outline; styleColor: "black"
    }
}
