import QtQuick

// Logo padrão: imagem "HR-V" cromada com reflexo em terracota e asterisco
// (gerada por tools/make_logo.py). Entra com um fade suave.
Rectangle {
    id: root
    property bool animate: true
    color: "black"

    Image {
        id: img
        anchors.fill: parent
        source: "boot_logo.png"
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
