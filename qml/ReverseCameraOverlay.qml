import QtQuick

// Câmera de ré simulada: fundo + linhas-guia. Na central, o fundo vira a imagem
// da câmera (QtMultimedia Camera / entrada AV via V4L2).
Rectangle {
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#3A4048" }
        GradientStop { position: 0.6; color: "#1C1F24" }
        GradientStop { position: 1.0; color: "#2B2A26" }
    }

    MouseArea { anchors.fill: parent } // bloqueia toques nas telas de baixo

    Canvas {
        anchors.fill: parent
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            ctx.lineWidth = 6
            const w = width, h = height
            const xAt = (y, left) => {
                const t = (y - 0.45) / (0.98 - 0.45)
                const half = 0.18 + t * 0.17
                return w * (0.5 + (left ? -half : half))
            }
            const bands = [[0.45, 0.62, "#4CAF50"], [0.62, 0.80, "#FFC107"], [0.80, 0.98, "#F44336"]]
            for (const [top, bottom, color] of bands) {
                ctx.strokeStyle = color
                for (const left of [true, false]) {
                    ctx.beginPath()
                    ctx.moveTo(xAt(top, left), h * top)
                    ctx.lineTo(xAt(bottom, left), h * bottom)
                    ctx.stroke()
                }
                ctx.beginPath()
                ctx.moveTo(xAt(top, true), h * top); ctx.lineTo(xAt(top, true) + w * 0.06, h * top)
                ctx.moveTo(xAt(top, false), h * top); ctx.lineTo(xAt(top, false) - w * 0.06, h * top)
                ctx.stroke()
            }
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 16
        text: "CÂMERA DE RÉ (simulada)"
        color: "white"; font.pixelSize: Theme.fontNormal
    }
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        text: "Verifique o entorno antes de manobrar"
        color: "#B3FFFFFF"; font.pixelSize: Theme.fontSmall
    }
}
