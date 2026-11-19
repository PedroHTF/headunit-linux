import QtQuick
import QtQuick.Window
import QtQuick.Layouts

Window {
    id: root
    width: 1024
    height: 600
    visible: true
    visibility: startFullscreen ? Window.FullScreen : Window.Windowed
    title: "HeadUnit — " + width + "×" + height
    color: Theme.background

    // Atalhos de teclado = botões do volante (para testar sem o simulador).
    Shortcut { sequence: "N"; onActivated: app.pressKey("next") }
    Shortcut { sequence: "B"; onActivated: app.pressKey("previous") }
    Shortcut { sequence: "Space"; onActivated: app.pressKey("playpause") }
    Shortcut { sequence: "+"; onActivated: app.pressKey("volup") }
    Shortcut { sequence: "-"; onActivated: app.pressKey("voldown") }
    Shortcut { sequence: "M"; onActivated: app.pressKey("mute") }
    Shortcut { sequence: "F"; onActivated: app.pressKey("mode") }
    Shortcut { sequence: "R"; enabled: !!simVehicle; onActivated: simVehicle.setGearByName(vehicle.gearLabel === "R" ? "P" : "R") }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        NavRail {
            id: nav
            Layout.fillHeight: true
            Layout.preferredWidth: 96
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            StatusBar {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: nav.currentIndex

                HomePage { onNavigate: index => nav.currentIndex = index }
                MediaPage {}
                RadioPage {}
                VehiclePage {}
                ProjectionPage {}
            }
        }
    }

    ReverseCameraOverlay {
        anchors.fill: parent
        visible: app.showReverseCamera
    }

    PowerOffOverlay {
        anchors.fill: parent
        visible: !vehicle.powered
    }

    SimulatorWindow {
        mainWindow: root
        visible: showSimulator
    }
}
