import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Telefone: agenda, recentes e teclado. A ligação em si aparece na CallOverlay.
Item {
    id: page
    property int tab: 0
    property string typed: ""

    component Row2: Rectangle {
        property string title
        property string subtitle
        property string glyph: ""
        property color glyphColor: Theme.primary
        signal clicked()
        width: ListView.view ? ListView.view.width : 100
        height: 56; radius: 12
        color: m.pressed ? Theme.surfaceHigh : Theme.surfaceVariant
        border.width: Theme.borderWidth; border.color: Theme.outline
        Row {
            anchors.fill: parent; anchors.leftMargin: 14; spacing: 12
            Text { anchors.verticalCenter: parent.verticalCenter; text: glyph; color: glyphColor; font.pixelSize: 22; width: 24 }
            Column {
                anchors.verticalCenter: parent.verticalCenter
                Text { text: title; color: Theme.text; font.pixelSize: Theme.fontNormal }
                Text { text: subtitle; color: Theme.textDim; font.pixelSize: Theme.fontSmall }
            }
        }
        Text { anchors.right: parent.right; anchors.rightMargin: 16; anchors.verticalCenter: parent.verticalCenter; text: "✆"; color: Theme.primary; font.pixelSize: 24 }
        MouseArea { id: m; anchors.fill: parent; onClicked: parent.clicked() }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 3
            spacing: 8

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Repeater {
                    model: ["Contatos", "Recentes", "Teclado"]
                    delegate: IconButton {
                        required property string modelData
                        required property int index
                        Layout.fillWidth: true
                        height: 44; radius: 12; fontSize: 16
                        text: modelData
                        highlighted: page.tab === index
                        onClicked: page.tab = index
                    }
                }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: page.tab

                ListView {
                    clip: true; spacing: 4
                    model: phone.contacts
                    delegate: Row2 {
                        required property var modelData
                        title: modelData.name; subtitle: modelData.number; glyph: modelData.name.charAt(0)
                        onClicked: phone.dial(modelData.number)
                    }
                }

                ListView {
                    clip: true; spacing: 4
                    model: phone.recents
                    delegate: Row2 {
                        required property var modelData
                        title: modelData.name
                        subtitle: (modelData.kind === "in" ? "Recebida" : modelData.kind === "out" ? "Feita" : "Perdida") + " · " + modelData.when
                        glyph: modelData.kind === "in" ? "↙" : modelData.kind === "out" ? "↗" : "✕"
                        glyphColor: modelData.kind === "missed" ? Theme.danger : Theme.primary
                        onClicked: phone.dial(modelData.number)
                    }
                }

                ColumnLayout {
                    spacing: 8
                    Rectangle {
                        Layout.fillWidth: true; height: 56; radius: 12
                        color: Theme.surfaceVariant; border.width: Theme.borderWidth; border.color: Theme.outline
                        Text {
                            anchors.centerIn: parent
                            text: page.typed === "" ? "Digite o número" : page.typed
                            color: page.typed === "" ? Theme.textDim : Theme.text
                            font.pixelSize: page.typed === "" ? Theme.fontNormal : Theme.fontTitle
                        }
                    }
                    GridLayout {
                        Layout.alignment: Qt.AlignHCenter
                        columns: 3; rowSpacing: 8; columnSpacing: 16
                        Repeater {
                            model: ["1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"]
                            delegate: IconButton {
                                required property string modelData
                                size: 56; fontSize: 24; text: modelData
                                onClicked: page.typed += modelData
                            }
                        }
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 16
                        IconButton { size: 56; fontSize: 20; text: "⌫"; enabled: page.typed !== ""; onClicked: page.typed = page.typed.slice(0, -1) }
                        IconButton {
                            width: 150; height: 56; radius: 28; fontSize: 18; highlighted: true
                            text: "✆ Ligar"; enabled: page.typed !== "" && phone.connected
                            onClicked: { phone.dial(page.typed); page.typed = "" }
                        }
                    }
                }
            }
        }

        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 2
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                Text { text: "Celular"; color: Theme.primary; font.pixelSize: Theme.fontLarge }
                Text {
                    text: phone.connected ? phone.deviceName : "Nenhum celular conectado"
                    color: Theme.text; font.pixelSize: Theme.fontNormal; font.bold: true
                }
                Text {
                    visible: phone.connected
                    text: "Sinal " + "▮".repeat(phone.signal) + "▯".repeat(5 - phone.signal) + "   Bateria " + phone.battery + "%"
                    color: Theme.textDim; font.pixelSize: Theme.fontSmall
                }
                Item { Layout.fillHeight: true }
                Text {
                    Layout.fillWidth: true; wrapMode: Text.WordWrap
                    color: Theme.textDim; font.pixelSize: Theme.fontSmall
                    text: (phone.simulated ? "Telefone simulado: use a janela Simulador para receber ligações. " : "") +
                          "O botão de telefone do volante atende e desliga (tecla T no PC)."
                }
            }
        }
    }
}
