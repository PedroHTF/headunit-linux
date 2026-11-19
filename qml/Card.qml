import QtQuick

// Painel arredondado; os filhos vão para dentro, com margem.
Rectangle {
    default property alias content: body.data
    property int padding: 16
    color: Theme.surfaceVariant
    radius: Theme.radius
    border.width: Theme.borderWidth
    border.color: Theme.outline

    Item {
        id: body
        anchors.fill: parent
        anchors.margins: parent.padding
    }
}
