#include "app/AppController.h"
#include "app/AppSettings.h"
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
        QStringLiteral("Tela inicial: 0 Início, 1 Mídia, 2 Rádio, 3 Veículo, 4 Projeção, 5 Configurações."), QStringLiteral("n"), QStringLiteral("0"));
    const QCommandLineOption shotOpt(QStringLiteral("screenshot"),
        QStringLiteral("Salva uma captura da tela principal em <arquivo> e sai (testes/documentação)."), QStringLiteral("arquivo"));
    cli.addOptions({mediaOpt, noSimOpt, fullOpt, pageOpt, shotOpt});
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
    AppSettings settings;
    AppController controller(&vehicle, &radio, &player, &settings);
    PerfMonitor perf(sinceStart);

    QQuickStyle::setStyle(QStringLiteral("Basic"));
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
    ctx->setContextProperty(QStringLiteral("showSimulator"), !cli.isSet(noSimOpt));
    ctx->setContextProperty(QStringLiteral("startFullscreen"), cli.isSet(fullOpt));
    ctx->setContextProperty(QStringLiteral("startPage"), cli.value(pageOpt).toInt());

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
        QTimer::singleShot(1500, window, [window, file] {
            const bool ok = window->grabWindow().save(file);
            qInfo().noquote() << (ok ? "Captura salva em" : "Falha ao salvar") << file;
            QCoreApplication::exit(ok ? 0 : 1);
        });
    }

    return app.exec();
}
