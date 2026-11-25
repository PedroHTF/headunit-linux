import QtQuick
import QtQuick.Layouts

// Ligação em andamento ou chegando: aparece por cima de qualquer tela.
Rectangle {
    id: root
    color: Theme.dark ? "#F5000000" : "#F5FFFFFF"
    MouseArea { anchors.fill: parent }

    property bool showKeypad: false
    readonly property bool incoming: phone.callState === "incoming"

    function clock(s) {
        const m = Math.floor(s / 60), r = s % 60
        return m + ":" + (r < 10 ? "0" : "") + r
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 14

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: root.incoming ? "Chamada recebida" : phone.callState === "dialing" ? "Chamando…" : root.clock(phone.callSeconds)
            color: Theme.primary; font.pixelSize: Theme.fontLarge
        }
        Text { Layout.alignment: Qt.AlignHCenter; text: phone.callName; color: Theme.text; font.pixelSize: 40; font.bold: true }
        Text {
            Layout.alignment: Qt.AlignHCenter
            visible: phone.callName !== phone.callNumber
            text: phone.callNumber; color: Theme.textDim; font.pixelSize: Theme.fontNormal
        }

        GridLayout {
            Layout.alignment: Qt.AlignHCenter
            visible: root.showKeypad && !root.incoming
            columns: 3; rowSpacing: 6; columnSpacing: 12
            Repeater {
                model: ["1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"]
                delegate: IconButton { required property string modelData; size: 48; fontSize: 20; text: modelData; onClicked: phone.sendDtmf(modelData) }
            }
        }

        Item { Layout.preferredHeight: 10 }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 40
            visible: root.incoming
            Rectangle {
                width: 96; height: 96; radius: 48; color: "#D32F2F"
                Text { anchors.centerIn: parent; text: "✕"; color: "white"; font.pixelSize: 40 }
                MouseArea { anchors.fill: parent; onClicked: phone.reject() }
            }
            Rectangle {
                width: 96; height: 96; radius: 48; color: "#2E7D32"
                Text { anchors.centerIn: parent; text: "✆"; color: "white"; font.pixelSize: 40 }
                MouseArea { anchors.fill: parent; onClicked: phone.accept() }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 28
            visible: !root.incoming
            IconButton { size: 72; fontSize: 15; text: phone.muted ? "Mudo ✓" : "Mudo"; highlighted: phone.muted; onClicked: phone.setMuted(!phone.muted) }
            Rectangle {
                width: 96; height: 96; radius: 48; color: "#D32F2F"
                Text { anchors.centerIn: parent; text: "✕"; color: "white"; font.pixelSize: 40 }
                MouseArea { anchors.fill: parent; onClicked: phone.hangup() }
            }
            IconButton { size: 72; fontSize: 15; text: "Teclado"; highlighted: root.showKeypad; onClicked: root.showKeypad = !root.showKeypad }
        }
    }
}
