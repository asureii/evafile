#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <iostream>
#include "MainWindow.hpp"
#include "ThemeEngine.hpp"
#include "Config.hpp"
#include "EvaFileQmlBridge.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("EvaFile");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("EvaSuite");
    app.setWindowIcon(QIcon(":/icons/evafile.svg"));

    // Apply Crimson Flame Theme
    ThemeEngine::instance().applyToApplication(&app);

    // Command line options
    QCommandLineParser parser;
    parser.setApplicationDescription("EvaFile - Modern Dolphin-inspired file manager with Crimson Flame theming");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption classicOption("classic", "Use classic Qt Widgets interface instead of modern QML interface.");
    parser.addOption(classicOption);

    parser.addPositionalArgument("directory", "Initial directory to open", "[directory]");

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    QString initialPath = QDir::homePath();
    if (!args.isEmpty()) {
        QString candidate = args.first();
        if (candidate.startsWith("~")) {
            candidate = QDir::homePath() + candidate.mid(1);
        }
        candidate = QDir::cleanPath(candidate);
        if (QDir(candidate).exists()) {
            initialPath = candidate;
        }
    }

    // Classic Widgets Mode
    if (parser.isSet(classicOption)) {
        MainWindow window;
        window.navigateTo(initialPath);
        window.show();
        return app.exec();
    }

    // Modern QML Mode (Default)
    QQuickStyle::setStyle("Basic");

    qmlRegisterType<EvaFileFolderModel>("org.evafile", 1, 0, "EvaFileFolderModel");

    auto* controller = new EvaFileController(&app);
    controller->setCurrentPath(initialPath);

    qmlRegisterSingletonInstance<EvaFileController>("org.evafile", 1, 0, "EvaFileController", controller);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("controller", controller);

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
