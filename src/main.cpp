#include "app/AppController.h"
#include "app/AppSettings.h"
#include "camera/CameraSettings.h"
#include "camera/ReverseCameraItem.h"
#include "logo/LogoManager.h"
#include "audio/Equalizer.h"
#include "hal/SimulatedPhoneHal.h"
#include "hal/SimulatedRadioHal.h"
#include "hal/SimulatedVehicleHal.h"
#include "media/MediaLibrary.h"
#include "media/MediaPlayerController.h"
#include "perf/PerfMonitor.h"

#include <QCommandLineParser>
#include <QDir>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QTimer>

int main(int argc, char *argv[])
{
    QElapsedTimer sinceStart;
    sinceStart.start();

    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("HeadUnit"));
    QGuiApplication::setApplicationVersion(QStringLiteral(PROJECT_VERSION_STRING));

    QCommandLineParser cli;
    cli.setApplicationDescription(QStringLiteral("Central multimídia — ambiente de desenvolvimento"));
    cli.addHelpOption();
    cli.addVersionOption();
    const QCommandLineOption mediaOpt(QStringLiteral("media"),
        QStringLiteral("Pasta que simula o pendrive USB (padrão: ~/HeadUnitUSB)."), QStringLiteral("pasta"));
    const QCommandLineOption noSimOpt(QStringLiteral("no-simulator"), QStringLiteral("Não abre a janela do simulador."));
    const QCommandLineOption fullOpt(QStringLiteral("fullscreen"), QStringLiteral("Tela cheia (como na central)."));
    const QCommandLineOption pageOpt(QStringLiteral("page"),
        QStringLiteral("Tela inicial: 0 Início, 1 Mídia, 2 Rádio, 3 Telefone, 4 Veículo, 5 Projeção, 6 Configurações."), QStringLiteral("n"), QStringLiteral("0"));
    const QCommandLineOption gearOpt(QStringLiteral("gear"), QStringLiteral("Marcha inicial do simulador (P, R, N, D)."), QStringLiteral("marcha"));
    const QCommandLineOption steerOpt(QStringLiteral("steering"), QStringLiteral("Ângulo inicial do volante no simulador."), QStringLiteral("graus"));
    const QCommandLineOption splashOpt(QStringLiteral("splash"), QStringLiteral("Mostra o logo mesmo com --screenshot."));
    const QCommandLineOption setupOpt(QStringLiteral("camera-setup"), QStringLiteral("Abre direto o ajuste da câmera de ré."));
    const QCommandLineOption shotOpt(QStringLiteral("screenshot"),
        QStringLiteral("Salva uma captura da tela principal em <arquivo> e sai (testes/documentação)."), QStringLiteral("arquivo"));
    const QCommandLineOption themeOpt(QStringLiteral("theme"), QStringLiteral("Tema só nesta execução: light, dark ou auto."), QStringLiteral("tema"));
    const QCommandLineOption hcOpt(QStringLiteral("high-contrast"), QStringLiteral("Alto contraste só nesta execução."));
    const QCommandLineOption eqOpt(QStringLiteral("equalizer"), QStringLiteral("Abre direto o equalizador."));
    const QCommandLineOption callOpt(QStringLiteral("call"), QStringLiteral("Simula uma ligação: incoming ou active."), QStringLiteral("estado"));
    const QCommandLineOption ignOpt(QStringLiteral("ignition"), QStringLiteral("Ignição inicial do simulador (OFF, ACC, ON)."), QStringLiteral("estado"));
    const QCommandLineOption radioOpt(QStringLiteral("radio-on"), QStringLiteral("Começa com o rádio ligado (fonte Rádio)."));
    const QCommandLineOption tabOpt(QStringLiteral("media-tab"), QStringLiteral("Aba inicial da Mídia: 0 Músicas, 1 Vídeos, 2 Pastas."), QStringLiteral("n"), QStringLiteral("0"));
    const QCommandLineOption simShotOpt(QStringLiteral("screenshot-simulator"),
        QStringLiteral("Com --screenshot, salva também a janela do simulador em <arquivo>."), QStringLiteral("arquivo"));
    cli.addOptions({mediaOpt, noSimOpt, fullOpt, pageOpt, shotOpt, gearOpt, steerOpt, splashOpt, setupOpt,
                    ignOpt, radioOpt, tabOpt, simShotOpt, themeOpt, hcOpt, eqOpt, callOpt});
    cli.process(app);

    QString mediaDir = cli.value(mediaOpt);
    if (mediaDir.isEmpty())
        mediaDir = qEnvironmentVariable("HEADUNIT_MEDIA_DIR");
    if (mediaDir.isEmpty())
        mediaDir = QDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).filePath(QStringLiteral("HeadUnitUSB"));
    QDir().mkpath(mediaDir);

    // ---- Montagem: troque as classes Simulated* pelas reais na central. -------
    SimulatedVehicleHal vehicle;
    SimulatedRadioHal radio;
    MediaLibrary library;
    library.setRootPath(QDir::toNativeSeparators(mediaDir));
    MediaPlayerController player(&library);
    if (cli.isSet(gearOpt)) vehicle.setGearByName(cli.value(gearOpt));
    if (cli.isSet(steerOpt)) vehicle.setSteering(cli.value(steerOpt).toDouble());
    if (cli.isSet(ignOpt)) vehicle.setIgnitionByName(cli.value(ignOpt));
    SimulatedPhoneHal phone;
    Equalizer equalizer;
    AppSettings settings;
    if (cli.isSet(themeOpt) || cli.isSet(hcOpt))
        settings.overrideTheme(cli.isSet(themeOpt) ? cli.value(themeOpt) : settings.themeMode(), cli.isSet(hcOpt));
    CameraSettings cameraSettings;
    LogoManager logo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    AppController controller(&vehicle, &radio, &player, &settings, &phone);
    PerfMonitor perf(sinceStart);
    if (cli.isSet(callOpt)) {
        phone.simulateIncoming(QStringLiteral("+55 11 98765-4321"));
        if (cli.value(callOpt) == u"active") phone.accept();
    }
    if (cli.isSet(radioOpt)) {
        controller.onRadioStarted();
        radio.tune(94700);
        radio.powerOn();
    }

    QQuickStyle::setStyle(QStringLiteral("Basic"));
    qmlRegisterType<ReverseCameraItem>("HeadUnit.Camera", 1, 0, "ReverseCamera");
    qmlRegisterUncreatableType<CameraSettings>("HeadUnit.Camera", 1, 0, "CameraSettings", QStringLiteral("Use cameraSettings"));
    QQmlApplicationEngine engine;
    QQmlContext *ctx = engine.rootContext();
    ctx->setContextProperty(QStringLiteral("vehicle"), &vehicle);
    ctx->setContextProperty(QStringLiteral("simVehicle"), &vehicle);
    ctx->setContextProperty(QStringLiteral("radio"), &radio);
    ctx->setContextProperty(QStringLiteral("library"), &library);
    ctx->setContextProperty(QStringLiteral("player"), &player);
    ctx->setContextProperty(QStringLiteral("app"), &controller);
    ctx->setContextProperty(QStringLiteral("perf"), &perf);
    ctx->setContextProperty(QStringLiteral("settings"), &settings);
    ctx->setContextProperty(QStringLiteral("phone"), &phone);
    ctx->setContextProperty(QStringLiteral("simPhone"), &phone);
    ctx->setContextProperty(QStringLiteral("equalizer"), &equalizer);
    ctx->setContextProperty(QStringLiteral("startEqualizer"), cli.isSet(eqOpt));
    ctx->setContextProperty(QStringLiteral("cameraSettings"), &cameraSettings);
    ctx->setContextProperty(QStringLiteral("logo"), &logo);
    // Numa captura de tela o logo atrapalharia; --splash força.
    ctx->setContextProperty(QStringLiteral("showSplashOnStart"), !cli.isSet(shotOpt) || cli.isSet(splashOpt));
    ctx->setContextProperty(QStringLiteral("startCameraSetup"), cli.isSet(setupOpt));
    ctx->setContextProperty(QStringLiteral("showSimulator"), !cli.isSet(noSimOpt));
    ctx->setContextProperty(QStringLiteral("startFullscreen"), cli.isSet(fullOpt));
    ctx->setContextProperty(QStringLiteral("startPage"), cli.value(pageOpt).toInt());
    ctx->setContextProperty(QStringLiteral("startMediaTab"), cli.value(tabOpt).toInt());

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.load(QUrl(QStringLiteral("qrc:/HeadUnit/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return 1;
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    if (window)
        perf.attach(window);

    if (cli.isSet(shotOpt) && window) {
        const QString file = cli.value(shotOpt);
        const QString simFile = cli.value(simShotOpt);
        QTimer::singleShot(1500, window, [window, file, simFile] {
            if (!simFile.isEmpty()) {
                for (QWindow *w : QGuiApplication::topLevelWindows()) {
                    auto *qw = qobject_cast<QQuickWindow *>(w);
                    if (qw && qw != window && qw->title() == u"Simulador") qw->grabWindow().save(simFile);
                }
            }
            const bool ok = window->grabWindow().save(file);
            qInfo().noquote() << (ok ? "Captura salva em" : "Falha ao salvar") << file;
            QCoreApplication::exit(ok ? 0 : 1);
        });
    }

    return app.exec();
}
