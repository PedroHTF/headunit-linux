import QtQuick

// Logo padrão: símbolo original de raios em terracota + texto. Os raios crescem um a um.
Rectangle {
    id: root
    property string text: "HR-V"
    property bool animate: true
    property real progress: animate ? 0 : 1
    color: "#0A0A0A"

    NumberAnimation on progress { running: root.animate; from: 0; to: 1; duration: 1500; easing.type: Easing.OutCubic }
    onProgressChanged: canvas.requestPaint()

    // (ângulo, comprimento relativo): 12 raios com leve irregularidade.
    readonly property var rays: {
        const out = []
        for (let i = 0; i < 12; ++i) {
            const jitter = i % 3 === 1 ? 0.06 : (i % 3 === 2 ? -0.04 : 0)
            const len = (i === 3 || i === 9) ? 0.86 : (i % 2 === 0 ? 1.0 : 0.68)
            out.push([i * 2 * Math.PI / 12 + jitter, len])
        }
        return out
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            const p = root.progress
            const cx = width / 2, cy = height * 0.40, r = height * 0.20
            ctx.fillStyle = "#D97757"
            ctx.translate(cx, cy)
            ctx.rotate((1 - p) * -40 * Math.PI / 180)
            for (let i = 0; i < root.rays.length; ++i) {
                const local = Math.max(0, Math.min(1, p * 1.6 - i * 0.05))
                if (local <= 0) continue
                const angle = root.rays[i][0]
                const length = root.rays[i][1] * local + 0.16 * (1 - local)
                drawRay(ctx, r, angle, length)
            }
            ctx.beginPath()
            ctx.arc(0, 0, r * 0.2 * Math.max(p, 0.15), 0, 2 * Math.PI)
            ctx.fill()
        }

        // Pétala afunilada: fina no centro, arredondada e mais larga na ponta.
        function drawRay(ctx, r, angle, length) {
            const inner = r * 0.16, outer = r * length
            const w = r * 0.085 * (0.8 + 0.4 * length)
            const ca = Math.cos(angle), sa = Math.sin(angle)
            const left = [], right = []
            for (let k = 0; k <= 16; ++k) {
                const t = k / 16
                const rr = inner + (outer - inner) * t
                const half = w * (0.15 + 0.85 * Math.sin(t * Math.PI / 2))
                left.push([rr * ca - half * sa, rr * sa + half * ca])
                right.push([rr * ca + half * sa, rr * sa - half * ca])
            }
            ctx.beginPath()
            ctx.moveTo(left[0][0], left[0][1])
            for (let k = 1; k < left.length; ++k) ctx.lineTo(left[k][0], left[k][1])
            for (let k = right.length - 1; k >= 0; --k) ctx.lineTo(right[k][0], right[k][1])
            ctx.closePath()
            ctx.fill()
            ctx.beginPath()
            ctx.arc(outer * ca, outer * sa, w, 0, 2 * Math.PI)
            ctx.fill()
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height * 0.66
        text: root.text
        color: "#F4EDE4"
        font.family: "serif"
        font.bold: true
        font.pixelSize: Math.max(8, parent.height * 0.11)
        font.letterSpacing: Math.max(8, parent.height * 0.11) * 0.18
        opacity: Math.max(0, Math.min(1, (root.progress - 0.55) / 0.45))
    }
}
