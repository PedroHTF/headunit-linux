import QtQuick

// Logo padrão: imagem HR-V (qml/images/boot_logo.jpg). Entra com um fade suave.
Rectangle {
    id: root
    property bool animate: true
    color: "black"

    Image {
        id: img
        anchors.fill: parent
        source: "boot_logo.jpg"
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
        opacity: root.animate ? 0 : 1
        scale: root.animate ? 1.04 : 1

        ParallelAnimation {
            running: root.animate
            NumberAnimation { target: img; property: "opacity"; to: 1; duration: 900; easing.type: Easing.OutCubic }
            NumberAnimation { target: img; property: "scale"; to: 1; duration: 1400; easing.type: Easing.OutCubic }
        }
    }
}
