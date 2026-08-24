#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <iostream>
#include "MainWindow.hpp"
#include "ThemeEngine.hpp"
#include "Config.hpp"

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

    MainWindow window;
    if (!args.isEmpty()) {
        window.navigateTo(initialPath);
    }
    window.show();

    return app.exec();
}
