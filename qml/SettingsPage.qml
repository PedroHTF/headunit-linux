import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Item {
    id: page
    signal openCameraSetup()
    signal previewLogo()
    signal openEqualizer()

    property var logoCandidates: null
    property string logoMessage: ""

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
            Text { Layout.fillWidth: true; text: title; color: Theme.text; font.pixelSize: Theme.fontNormal }
            Text { Layout.fillWidth: true; text: subtitle; color: Theme.textDim; font.pixelSize: Theme.fontSmall; wrapMode: Text.WordWrap }
        }
        Switch { id: toggle; onToggled: parent.toggled(checked) }
    }
    component Section: Text { Layout.topMargin: 14; color: Theme.primary; font.pixelSize: Theme.fontLarge }
    component Small: Text { Layout.fillWidth: true; color: Theme.textDim; font.pixelSize: Theme.fontSmall; wrapMode: Text.WordWrap }
    component Action: IconButton { height: 40; radius: 12; fontSize: 14; width: implicitW; property int implicitW: 180 }

    FileDialog {
        id: fileDialog
        title: "Escolher logo"
        nameFilters: ["Imagens e vídeos (*.png *.jpg *.jpeg *.webp *.bmp *.mp4 *.mkv *.webm *.mov)"]
        onAccepted: page.logoMessage = logo.importUrl(selectedFile) ? "Logo atualizado." : "Não foi possível ler o arquivo."
    }

    Card {
        anchors.fill: parent
        anchors.margins: 16

        ScrollView {
            anchors.fill: parent
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 8

                Text { text: "Configurações"; color: Theme.primary; font.pixelSize: Theme.fontTitle }

                Section { text: "Aparência"; Layout.topMargin: 4 }
                RowLayout {
                    spacing: 8
                    Text { text: "Tema"; color: Theme.text; font.pixelSize: Theme.fontNormal; Layout.rightMargin: 8 }
                    Repeater {
                        model: [["auto", "Automático"], ["light", "Claro"], ["dark", "Escuro"]]
                        delegate: Action {
                            required property var modelData
                            implicitW: 130
                            text: modelData[1]
                            highlighted: settings.themeMode === modelData[0]
                            onClicked: settings.themeMode = modelData[0]
                        }
                    }
                }
                Small { text: "Automático: escuro das 18h às 6h. Na central, poderá seguir o farol do carro." }
                SettingRow {
                    title: "Alto contraste"
                    subtitle: "Preto e branco puros, cores de destaque fortes e bordas, para ler sob sol forte ou com baixa visão."
                    checked: settings.highContrast
                    onToggled: value => settings.highContrast = value
                }

                Section { text: "Áudio" }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16
                    Small { text: "Equalizador de 10 bandas, balanço, fader e loudness. Atual: " + equalizer.preset + "." }
                    Action { implicitW: 170; text: "Equalizador"; highlighted: true; onClicked: page.openEqualizer() }
                }

                Section { text: "Vídeo" }
                SettingRow {
                    title: "Vídeo em qualquer velocidade"
                    subtitle: "Desligado: o vídeo some acima de 5 km/h e só o áudio continua. Ligado: o vídeo " +
                              "aparece sempre. Use com responsabilidade; o motorista não deve assistir dirigindo."
                    checked: settings.videoWhileDriving
                    onToggled: value => settings.videoWhileDriving = value
                }

                Section { text: "Logo de inicialização" }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16
                    Rectangle {
                        Layout.preferredWidth: 213; Layout.preferredHeight: 120
                        radius: 12; color: "black"; clip: true
                        DefaultLogo { anchors.fill: parent; visible: logo.kind === "default"; animate: false }
                        Image { anchors.fill: parent; visible: logo.kind === "image"; source: logo.kind === "image" ? logo.fileUrl : ""; fillMode: Image.PreserveAspectFit; cache: false }
                        Text { anchors.centerIn: parent; visible: logo.kind === "video"; text: "Vídeo personalizado"; color: "white" }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Small { text: "No pendrive, crie a pasta HeadUnit e coloque logo.png, logo.jpg ou logo.mp4. O arquivo é copiado para a central: depois o pendrive pode sair." }
                        Flow {
                            Layout.fillWidth: true
                            spacing: 8
                            Action { implicitW: 200; text: "Procurar no pendrive"; highlighted: true
                                onClicked: {
                                    page.logoCandidates = logo.findCandidates(library.rootPath)
                                    page.logoMessage = page.logoCandidates.length ? "" : "Nenhum HeadUnit/logo.* encontrado."
                                }
                            }
                            Action { implicitW: 170; text: "Escolher arquivo"; onClicked: fileDialog.open() }
                            Action { implicitW: 120; text: "Ver agora"; onClicked: page.previewLogo() }
                            Action { implicitW: 180; visible: logo.kind !== "default"; text: "Usar logo padrão"; onClicked: logo.useDefault() }
                        }
                        Text { visible: page.logoMessage !== ""; text: page.logoMessage; color: Theme.primary; font.pixelSize: Theme.fontSmall }
                    }
                }
                Repeater {
                    model: page.logoCandidates || []
                    delegate: Rectangle {
                        required property var modelData
                        Layout.fillWidth: true
                        height: 44; radius: 10
                        color: candMouse.pressed ? Theme.surfaceHigh : Theme.surface
                        Text {
                            anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 12
                            text: modelData.name + "  ·  " + (modelData.isVideo ? "vídeo" : "imagem") + " — toque para usar"
                            color: Theme.text; font.pixelSize: Theme.fontSmall
                        }
                        MouseArea {
                            id: candMouse; anchors.fill: parent
                            onClicked: {
                                page.logoMessage = logo.importFile(modelData.path) ? "Logo atualizado." : "Não foi possível copiar o arquivo."
                                page.logoCandidates = null
                            }
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Tempo na tela: " + logo.seconds + " s"; color: Theme.text; font.pixelSize: Theme.fontSmall; Layout.preferredWidth: 160 }
                    Slider { Layout.fillWidth: true; from: 1; to: 8; stepSize: 1; snapMode: Slider.SnapAlways; value: logo.seconds; onMoved: logo.seconds = value }
                }
                SettingRow {
                    title: "Mostrar ao ligar a ignição"
                    subtitle: "Além de quando o sistema inicia. Toque no logo para pular."
                    checked: logo.onIgnition
                    onToggled: value => logo.onIgnition = value
                }

                Section { text: "Câmera de ré" }
                Small { text: "Medidas do carro, posição da câmera, correção do fisheye, linhas de distância (em metros ou em carros) e trajetória pelo volante, com prévia ao vivo." }
                Action { implicitW: 230; text: "Ajustar câmera de ré"; highlighted: true; onClicked: page.openCameraSetup() }

                Small { Layout.topMargin: 14; text: "As configurações ficam salvas entre execuções. CarPlay/Android Auto: veja a tela Projeção." }
            }
        }
    }
}
