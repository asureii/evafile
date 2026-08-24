#include "EvaSuiteBridge.hpp"
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

static QString findExecutable(const QString& name) {
    QStringList candidates = {
        QDir::homePath() + "/.local/bin/" + name,
        "/usr/local/bin/" + name,
        "/usr/bin/" + name
    };
    for (const QString& path : candidates) {
        if (QFileInfo::exists(path) && QFileInfo(path).isExecutable()) {
            return path;
        }
    }
    return QString();
}

bool EvaSuiteBridge::isEvaTermAvailable() {
    return !findExecutable("evaterm").isEmpty();
}

bool EvaSuiteBridge::isEvaSortAvailable() {
    return !findExecutable("evasort").isEmpty();
}

bool EvaSuiteBridge::openInEvaTerm(const QString& directoryPath) {
    QString evatermBin = findExecutable("evaterm");
    if (!evatermBin.isEmpty()) {
        QProcess::startDetached(evatermBin, QStringList(), directoryPath);
        return true;
    }

    // Fallback to kitty / x-terminal-emulator / foot / alacritty
    QStringList fallbacks = {"kitty", "foot", "alacritty", "konsole", "gnome-terminal", "xterm"};
    for (const QString& term : fallbacks) {
        QString bin = findExecutable(term);
        if (!bin.isEmpty()) {
            QProcess::startDetached(bin, QStringList(), directoryPath);
            return true;
        }
    }

    return false;
}

bool EvaSuiteBridge::sortWithEvaSort(const QString& directoryPath, QWidget* parent) {
    QString evasortBin = findExecutable("evasort");
    if (evasortBin.isEmpty()) {
        if (parent) {
            QMessageBox::warning(parent, "EvaSort Not Found",
                "EvaSort executable was not found in ~/.local/bin/evasort or PATH.");
        }
        return false;
    }

    QProcess process;
    process.setWorkingDirectory(directoryPath);
    process.start(evasortBin, QStringList() << directoryPath);
    process.waitForFinished(10000);

    if (process.exitCode() == 0) {
        if (parent) {
            QMessageBox::information(parent, "EvaSort Completed",
                QString("Files in %1 have been organized successfully by EvaSort!").arg(directoryPath));
        }
        return true;
    } else {
        if (parent) {
            QString err = process.readAllStandardError();
            QMessageBox::critical(parent, "EvaSort Error",
                QString("Failed to sort directory: %1").arg(err.isEmpty() ? "Unknown error" : err));
        }
        return false;
    }
}

bool EvaSuiteBridge::openWithDefaultApp(const QString& filePath) {
    return QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}
