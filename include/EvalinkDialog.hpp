#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class EvalinkDialog : public QDialog {
    Q_OBJECT
public:
    explicit EvalinkDialog(const QString& currentDirectory, QWidget* parent = nullptr);

    QString url() const;
    QString destinationDirectory() const;
    QString customFilename() const;

private slots:
    void onBrowseDirectory();
    void onStartDownload();

private:
    QLineEdit* m_urlEdit;
    QLineEdit* m_dirEdit;
    QLineEdit* m_filenameEdit;
    QPushButton* m_startBtn;
    QPushButton* m_cancelBtn;
};
