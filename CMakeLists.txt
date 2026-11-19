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
    src/media/MediaLibrary.h src/media/MediaLibrary.cpp
    src/media/MediaPlayerController.h src/media/MediaPlayerController.cpp
    src/app/AppController.h src/app/AppController.cpp
    src/perf/PerfMonitor.h src/perf/PerfMonitor.cpp
)
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

qt_add_qml_module(headunit
    URI HeadUnit
    VERSION 1.0
    QML_FILES ${QML_FILES}
)
target_link_libraries(headunit PRIVATE headunit_core Qt6::Quick Qt6::QuickControls2 Qt6::Multimedia)

# ---- Testes ---------------------------------------------------------------------
if(TARGET Qt6::Test)
    enable_testing()
    foreach(t tst_radio tst_driving_rules tst_media_library)
        qt_add_executable(${t} tests/${t}.cpp)
        target_link_libraries(${t} PRIVATE headunit_core Qt6::Test)
        add_test(NAME ${t} COMMAND ${t})
        set_tests_properties(${t} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    endforeach()
endif()
