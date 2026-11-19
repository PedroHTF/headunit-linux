import QtQuick

// Botão redondo com um símbolo/texto. Grande o bastante para usar com o carro andando.
Rectangle {
    id: root
    property string text
    property bool highlighted: false
    property bool enabled: true
    property int size: 64
    property int fontSize: size * 0.4
    signal clicked()
    signal pressAndHold()

    width: size
    height: size
    radius: size / 2
    opacity: enabled ? 1 : 0.35
    color: highlighted ? Theme.primary : (mouse.pressed ? Theme.surfaceHigh : Theme.surface)

    Text {
        anchors.centerIn: parent
        text: root.text
        color: root.highlighted ? Theme.primaryText : Theme.text
        font.pixelSize: root.fontSize
        font.bold: true
    }
    MouseArea {
        id: mouse
        anchors.fill: parent
        enabled: root.enabled
        onClicked: root.clicked()
        onPressAndHold: root.pressAndHold()
    }
}
