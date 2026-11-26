cmake_minimum_required(VERSION 3.21)
project(headunit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8) # fontes em UTF-8 (acentos)
endif()

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick QuickControls2 Multimedia)
find_package(Qt6 6.4 QUIET COMPONENTS Test)

qt_standard_project_setup()
if(QT_KNOWN_POLICY_QTP0001)
    qt_policy(SET QTP0001 OLD) # mantém os QML em qrc:/HeadUnit/
endif()

# ---- Núcleo (sem interface): HAL, mídia, controle do app, métricas -------------
qt_add_library(headunit_core STATIC
    src/hal/DrivingRules.h
    src/hal/VehicleHal.h src/hal/VehicleHal.cpp
    src/hal/SimulatedVehicleHal.h src/hal/SimulatedVehicleHal.cpp
    src/hal/RadioHal.h src/hal/RadioHal.cpp
    src/hal/SimulatedRadioHal.h src/hal/SimulatedRadioHal.cpp
    src/hal/PhoneHal.h src/hal/PhoneHal.cpp
    src/hal/SimulatedPhoneHal.h src/hal/SimulatedPhoneHal.cpp
    src/hal/VCard.h src/hal/VCard.cpp
    src/audio/Equalizer.h src/audio/Equalizer.cpp
    src/media/MediaLibrary.h src/media/MediaLibrary.cpp
    src/media/MediaPlayerController.h src/media/MediaPlayerController.cpp
    src/app/AppController.h src/app/AppController.cpp
    src/app/AppSettings.h src/app/AppSettings.cpp
    src/perf/PerfMonitor.h src/perf/PerfMonitor.cpp
    src/camera/CameraModel.h src/camera/CameraModel.cpp
    src/camera/Undistort.h src/camera/Undistort.cpp
    src/camera/Guidelines.h src/camera/Guidelines.cpp
    src/camera/SyntheticScene.h src/camera/SyntheticScene.cpp
    src/camera/CameraSettings.h src/camera/CameraSettings.cpp
    src/camera/ReverseCameraItem.h src/camera/ReverseCameraItem.cpp
    src/logo/LogoManager.h src/logo/LogoManager.cpp
    src/media/FolderTree.h
    src/projection/DongleProtocol.h src/projection/DongleProtocol.cpp
    src/projection/DongleTransport.h
    src/projection/SimulatedDongleTransport.h src/projection/SimulatedDongleTransport.cpp
    src/projection/H264Decoder.h src/projection/H264Decoder.cpp
    src/projection/ProjectionController.h src/projection/ProjectionController.cpp
    src/projection/ProjectionView.h src/projection/ProjectionView.cpp
)

# ---- Dongle CarPlay/Android Auto: USB real (libusb) e vídeo (FFmpeg), se houver --------
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBUSB IMPORTED_TARGET libusb-1.0)
    pkg_check_modules(FFMPEG IMPORTED_TARGET libavcodec libavutil libswscale)
endif()
if(LIBUSB_FOUND)
    target_sources(headunit_core PRIVATE src/projection/UsbDongleTransport.h src/projection/UsbDongleTransport.cpp)
    target_compile_definitions(headunit_core PUBLIC HEADUNIT_HAVE_LIBUSB)
    target_link_libraries(headunit_core PUBLIC PkgConfig::LIBUSB)
    message(STATUS "Dongle USB: libusb encontrada")
else()
    message(STATUS "Dongle USB: sem libusb (só dongle simulado)")
endif()
if(FFMPEG_FOUND)
    target_compile_definitions(headunit_core PUBLIC HEADUNIT_HAVE_FFMPEG)
    target_link_libraries(headunit_core PUBLIC PkgConfig::FFMPEG)
    message(STATUS "Vídeo H.264: FFmpeg encontrado")
else()
    message(STATUS "Vídeo H.264: sem FFmpeg (vídeo do dongle real não será decodificado)")
endif()
# ---- Telefone real (oFono/BlueZ via D-Bus): só no Linux --------------------------------
option(HEADUNIT_PHONE_DBUS "Telefone real via oFono/obexd (D-Bus)" ON)
if(UNIX AND NOT APPLE AND HEADUNIT_PHONE_DBUS)
    find_package(Qt6 6.4 QUIET COMPONENTS DBus)
endif()
if(HEADUNIT_PHONE_DBUS AND TARGET Qt6::DBus)
    target_sources(headunit_core PRIVATE
        src/hal/OfonoTypes.h
        src/hal/OfonoPhoneHal.h src/hal/OfonoPhoneHal.cpp
        src/hal/PbapClient.h src/hal/PbapClient.cpp)
    target_compile_definitions(headunit_core PUBLIC HEADUNIT_HAVE_DBUS)
    target_link_libraries(headunit_core PUBLIC Qt6::DBus)
    message(STATUS "Telefone: oFono/PBAP via D-Bus disponível")
else()
    message(STATUS "Telefone: sem D-Bus (só telefone simulado)")
endif()
target_include_directories(headunit_core PUBLIC src)
target_link_libraries(headunit_core PUBLIC Qt6::Core Qt6::Gui Qt6::Quick Qt6::Multimedia)
if(WIN32)
    target_link_libraries(headunit_core PUBLIC psapi)
endif()

# ---- Aplicativo -----------------------------------------------------------------
qt_add_executable(headunit src/main.cpp)
target_compile_definitions(headunit PRIVATE PROJECT_VERSION_STRING="${PROJECT_VERSION}")

set(QML_FILES
    qml/Main.qml
    qml/Theme.qml
    qml/StatusBar.qml
    qml/NavRail.qml
    qml/HomePage.qml
    qml/MediaPage.qml
    qml/RadioPage.qml
    qml/VehiclePage.qml
    qml/ProjectionPage.qml
    qml/SettingsPage.qml
    qml/PhonePage.qml
    qml/CallOverlay.qml
    qml/EqualizerPage.qml
    qml/CameraSetupPage.qml
    qml/BootSplash.qml
    qml/DefaultLogo.qml
    qml/ReverseCameraOverlay.qml
    qml/PowerOffOverlay.qml
    qml/SimulatorWindow.qml
    qml/Card.qml
    qml/IconButton.qml
)
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
foreach(f ${QML_FILES})
    get_filename_component(name ${f} NAME)
    set_source_files_properties(${f} PROPERTIES QT_RESOURCE_ALIAS ${name})
endforeach()

set_source_files_properties(qml/images/boot_logo.jpg PROPERTIES QT_RESOURCE_ALIAS boot_logo.jpg)
qt_add_qml_module(headunit
    URI HeadUnit
    VERSION 1.0
    QML_FILES ${QML_FILES}
    RESOURCES qml/images/boot_logo.jpg
)
target_link_libraries(headunit PRIVATE headunit_core Qt6::Quick Qt6::QuickControls2 Qt6::Multimedia)

# ---- Testes ---------------------------------------------------------------------
if(TARGET Qt6::Test)
    enable_testing()
    foreach(t tst_radio tst_driving_rules tst_media_library tst_camera tst_logo_folders tst_phone_eq tst_dongle tst_vcard)
        qt_add_executable(${t} tests/${t}.cpp)
        target_link_libraries(${t} PRIVATE headunit_core Qt6::Test)
        add_test(NAME ${t} COMMAND ${t})
        set_tests_properties(${t} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    endforeach()
    # oFono e obexd falsos num D-Bus de sessão próprio (dbus-run-session), sem Bluetooth.
    find_program(DBUS_RUN_SESSION dbus-run-session)
    if(HEADUNIT_PHONE_DBUS AND TARGET Qt6::DBus AND DBUS_RUN_SESSION)
        qt_add_executable(tst_ofono tests/tst_ofono.cpp)
        target_link_libraries(tst_ofono PRIVATE headunit_core Qt6::Test Qt6::DBus)
        add_test(NAME tst_ofono COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:tst_ofono>)
        set_tests_properties(tst_ofono PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    endif()
endif()
