import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

Item {
    id: mediaRoot
    property int tab: 0
    property string folderPath: ""
    // Reavalia quando a biblioteca muda (pendrive conectado, arquivos novos...).
    readonly property var folderEntries: { library.count; library.mounted; return library.folderEntries(folderPath) }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // ---- Lista do pendrive ------------------------------------------------
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            spacing: 8

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Repeater {
                    model: ["Músicas", "Vídeos", "Pastas"]
                    delegate: IconButton {
                        required property string modelData
                        required property int index
                        Layout.fillWidth: true
                        height: 44; radius: 12; fontSize: 16
                        text: modelData
                        highlighted: mediaRoot.tab === index
                        onClicked: {
                            mediaRoot.tab = index
                            if (index < 2) library.showVideos = index === 1
                        }
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                text: library.mounted ? "USB: " + library.rootPath : "Pendrive removido"
                color: Theme.textDim; font.pixelSize: Theme.fontSmall
                elide: Text.ElideMiddle
            }
            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: mediaRoot.tab === 2 ? 1 : 0

                ListView {
                    id: list
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 4
                    model: library
                    delegate: Rectangle {
                        required property int index
                        required property string title
                        required property string subtitle
                        required property bool isVideo
                        width: ListView.view.width
                        height: 56
                        radius: 12
                        color: rowMouse.pressed ? Theme.surfaceHigh : Theme.surfaceVariant
                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            spacing: 12
                            Text { anchors.verticalCenter: parent.verticalCenter; text: isVideo ? "▣" : "♪"; color: Theme.primary; font.pixelSize: 22 }
                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width - 60
                                Text { width: parent.width; text: title; color: Theme.text; font.pixelSize: Theme.fontNormal; elide: Text.ElideRight }
                                Text { width: parent.width; text: subtitle; color: Theme.textDim; font.pixelSize: Theme.fontSmall; elide: Text.ElideRight }
                            }
                        }
                        MouseArea { id: rowMouse; anchors.fill: parent; onClicked: player.playFromLibrary(index) }
                    }
                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 32
                        visible: list.count === 0
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        color: Theme.textDim
                        text: library.mounted
                              ? "Nenhum arquivo.\nCopie músicas/vídeos para a pasta acima — a lista atualiza sozinha."
                              : "Conecte o pendrive (janela Simulador)."
                    }
                }

                // ---- Pastas do pendrive
                ColumnLayout {
                    spacing: 6
                    RowLayout {
                        Layout.fillWidth: true
                        IconButton {
                            size: 40; fontSize: 18; text: "‹"
                            enabled: mediaRoot.folderPath !== ""
                            onClicked: mediaRoot.folderPath = library.parentFolder(mediaRoot.folderPath)
                        }
                        Text {
                            Layout.fillWidth: true
                            text: ["USB"].concat(mediaRoot.folderPath === "" ? [] : mediaRoot.folderPath.split("/")).join("  ›  ")
                            color: Theme.text; font.pixelSize: Theme.fontSmall; elide: Text.ElideLeft
                        }
                    }
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 4
                        model: mediaRoot.folderEntries
                        delegate: Rectangle {
                            required property var modelData
                            readonly property bool isFolder: modelData.kind === "folder"
                            width: ListView.view.width
                            height: 52
                            radius: 12
                            color: entryMouse.pressed ? Theme.surfaceHigh : Theme.surfaceVariant
                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                spacing: 12
                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: isFolder ? "▸" : (modelData.isVideo ? "▣" : "♪")
                                    color: Theme.primary; font.pixelSize: 22
                                }
                                Column {
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: parent.width - 60
                                    Text { width: parent.width; text: isFolder ? modelData.name : modelData.title; color: Theme.text; font.pixelSize: Theme.fontNormal; elide: Text.ElideRight }
                                    Text {
                                        width: parent.width
                                        text: isFolder ? modelData.count + (modelData.count === 1 ? " arquivo" : " arquivos") : (modelData.isVideo ? "Vídeo" : "Música")
                                        color: Theme.textDim; font.pixelSize: Theme.fontSmall
                                    }
                                }
                            }
                            MouseArea {
                                id: entryMouse
                                anchors.fill: parent
                                onClicked: {
                                    if (isFolder) mediaRoot.folderPath = modelData.path
                                    else player.playFolder(mediaRoot.folderPath, modelData.index)
                                }
                            }
                        }
                        Text {
                            anchors.centerIn: parent
                            visible: parent.count === 0
                            text: library.mounted ? "Pasta vazia." : "Pendrive removido."
                            color: Theme.textDim
                        }
                    }
                }
            }
        }

        // ---- Tocando agora ----------------------------------------------------
        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 12
                    color: "black"
                    clip: true

                    VideoOutput {
                        id: videoOutput
                        anchors.fill: parent
                        visible: player.isVideo && app.videoAllowed
                        Component.onCompleted: player.setVideoOutput(videoOutput)
                    }
                    Text {
                        anchors.centerIn: parent
                        visible: !videoOutput.visible
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                        font.pixelSize: player.isVideo ? Theme.fontNormal : 96
                        text: player.isVideo ? "Vídeo oculto com o carro em movimento.\nO áudio continua." : "♫"
                    }
                }
                Text {
                    Layout.fillWidth: true
                    text: player.hasMedia ? player.title : "Nada tocando"
                    color: Theme.text; font.pixelSize: Theme.fontLarge; font.bold: true
                    elide: Text.ElideRight
                }
                Text {
                    Layout.fillWidth: true
                    text: player.error !== "" ? "Erro: " + player.error : player.subtitle
                    color: player.error !== "" ? Theme.danger : Theme.textDim
                    font.pixelSize: Theme.fontSmall
                    elide: Text.ElideRight
                }
                Slider {
                    Layout.fillWidth: true
                    from: 0
                    to: Math.max(1, player.duration)
                    value: player.position
                    enabled: player.hasMedia
                    onMoved: player.seek(value)
                }
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 24
                    IconButton { text: "|◀"; enabled: player.hasMedia; onClicked: player.previous() }
                    IconButton {
                        size: 76; highlighted: true; enabled: player.hasMedia
                        text: player.playing ? "❚❚" : "▶"
                        onClicked: player.togglePlayPause()
                    }
                    IconButton { text: "▶|"; enabled: player.hasNext; onClicked: player.next() }
                }
            }
        }
    }
}
