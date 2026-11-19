import QtQuick
import QtQuick.Layouts

Item {
    Card {
        anchors.fill: parent
        anchors.margins: 16
        ColumnLayout {
            anchors.centerIn: parent
            width: parent.width * 0.8
            spacing: 12
            Text { Layout.alignment: Qt.AlignHCenter; text: "⧉"; color: Theme.primary; font.pixelSize: 72 }
            Text { Layout.alignment: Qt.AlignHCenter; text: "CarPlay / Android Auto"; color: Theme.text; font.pixelSize: Theme.fontTitle }
            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                color: Theme.textDim; font.pixelSize: Theme.fontNormal
                text: "Neste caminho (Linux próprio) o Zlink não existe. A projeção virá de um dongle USB " +
                      "(tipo Carlinkit), que faz a autenticação com o celular e entrega vídeo H.264 + áudio. " +
                      "Planejado: integrar o protocolo do dongle e exibir o vídeo aqui."
            }
        }
    }
}
