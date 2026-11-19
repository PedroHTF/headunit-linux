import QtQuick
import QtMultimedia

// Logo da inicialização (padrão, imagem ou vídeo do usuário). Toque para pular.
Rectangle {
    id: root
    signal finished()
    color: "#0A0A0A"

    MouseArea { anchors.fill: parent; onClicked: root.finished() }

    Loader {
        anchors.fill: parent
        sourceComponent: logo.kind === "image" ? imageLogo : logo.kind === "video" ? videoLogo : defaultLogo
    }

    Component { id: defaultLogo; DefaultLogo { text: logo.text } }

    Component {
        id: imageLogo
        Image { source: logo.fileUrl; fillMode: Image.PreserveAspectFit; cache: false; asynchronous: true }
    }

    Component {
        id: videoLogo
        Item {
            VideoOutput { id: out; anchors.fill: parent }
            MediaPlayer {
                source: logo.fileUrl
                videoOutput: out
                audioOutput: AudioOutput { muted: true } // logo em vídeo começa sem som
                onMediaStatusChanged: if (mediaStatus === MediaPlayer.EndOfMedia) root.finished()
                Component.onCompleted: play()
            }
        }
    }

    Timer {
        running: true
        interval: logo.kind === "video" ? 15000 : logo.seconds * 1000
        onTriggered: root.finished()
    }
}
