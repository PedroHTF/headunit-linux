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
    Shortcut { sequence: "Left"; enabled: !!simVehicle; onActivated: simVehicle.setSteering(vehicle.steeringDeg - 45) }
    Shortcut { sequence: "Right"; enabled: !!simVehicle; onActivated: simVehicle.setSteering(vehicle.steeringDeg + 45) }

    // Logo: ao abrir e, se configurado, toda vez que a ignição liga.
    property bool splash: showSplashOnStart
    property bool wasPowered: vehicle.powered
    property bool cameraSetupOpen: startCameraSetup
    Connections {
        target: vehicle
        function onStateChanged() {
            if (vehicle.powered && !root.wasPowered && logo.onIgnition) root.splash = true
            root.wasPowered = vehicle.powered
        }
    }

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
                SettingsPage {
                    onOpenCameraSetup: root.cameraSetupOpen = true
                    onPreviewLogo: root.splash = true
                }
            }
        }
    }

    Loader {
        anchors.fill: parent
        active: app.showReverseCamera && !root.cameraSetupOpen
        sourceComponent: ReverseCameraOverlay {}
    }

    Loader {
        anchors.fill: parent
        active: root.cameraSetupOpen
        sourceComponent: CameraSetupPage { onClosed: root.cameraSetupOpen = false }
    }

    Loader {
        anchors.fill: parent
        active: root.splash && vehicle.powered && !app.showReverseCamera
        sourceComponent: BootSplash { onFinished: root.splash = false }
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
