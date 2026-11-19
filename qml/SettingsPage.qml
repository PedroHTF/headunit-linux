import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    component SettingRow: RowLayout {
        property string title
        property string subtitle
        property alias checked: toggle.checked
        signal toggled(bool value)
        Layout.fillWidth: true
        spacing: 16
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            Text { Layout.fillWidth: true; text: title; color: Theme.text; font.pixelSize: Theme.fontLarge }
            Text { Layout.fillWidth: true; text: subtitle; color: Theme.textDim; font.pixelSize: Theme.fontSmall; wrapMode: Text.WordWrap }
        }
        Switch { id: toggle; onToggled: parent.toggled(checked) }
    }

    Card {
        anchors.fill: parent
        anchors.margins: 16

        ColumnLayout {
            anchors.fill: parent
            spacing: 16

            Text { text: "Configurações"; color: Theme.primary; font.pixelSize: Theme.fontTitle }

            Text { text: "Vídeo"; color: Theme.text; font.pixelSize: Theme.fontNormal; font.bold: true }
            SettingRow {
                title: "Vídeo em qualquer velocidade"
                subtitle: "Desligado: o vídeo some acima de 5 km/h e só o áudio continua. Ligado: o vídeo " +
                          "aparece sempre. Use com responsabilidade; o motorista não deve assistir dirigindo."
                checked: settings.videoWhileDriving
                onToggled: value => settings.videoWhileDriving = value
            }

            Item { Layout.fillHeight: true }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: Theme.textDim; font.pixelSize: Theme.fontSmall
                text: "As configurações ficam salvas entre execuções. CarPlay/Android Auto: veja a tela Projeção."
            }
        }
    }
}
