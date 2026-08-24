#pragma once

#include <QString>
#include <QWidget>

class EvaSuiteBridge {
public:
    static bool openInEvaTerm(const QString& directoryPath);
    static bool sortWithEvaSort(const QString& directoryPath, QWidget* parent = nullptr);
    static bool openWithDefaultApp(const QString& filePath);
    static bool isEvaTermAvailable();
    static bool isEvaSortAvailable();
};
