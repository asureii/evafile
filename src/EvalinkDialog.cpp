#include "EvalinkDialog.hpp"
#include <QFileDialog>
#include <QClipboard>
#include <QGuiApplication>
#include <QMessageBox>

EvalinkDialog::EvalinkDialog(const QString& currentDirectory, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Evalink Download");
    setMinimumWidth(480);
    setWindowIcon(QIcon(":/icons/downloads.svg"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    // Header
    QLabel* headerLabel = new QLabel("Add New Download via Evalink", this);
    headerLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #ebdada;");
    layout->addWidget(headerLabel);

    // URL input
    QLabel* urlLabel = new QLabel("Download URL / Magnet / Torrent:", this);
    urlLabel->setStyleSheet("color: #b58487; font-size: 12px;");
    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText("https://example.com/file.zip or magnet:?xt=...");
    layout->addWidget(urlLabel);
    layout->addWidget(m_urlEdit);

    // Auto-fill from clipboard
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        QString clipText = clipboard->text().trimmed();
        if (clipText.startsWith("http://") || clipText.startsWith("https://") || clipText.startsWith("magnet:?") || clipText.startsWith("ftp://")) {
            m_urlEdit->setText(clipText);
            m_urlEdit->selectAll();
        }
    }

    // Directory
    QLabel* dirLabel = new QLabel("Save to Folder:", this);
    dirLabel->setStyleSheet("color: #b58487; font-size: 12px;");
    layout->addWidget(dirLabel);

    QHBoxLayout* dirLayout = new QHBoxLayout();
    m_dirEdit = new QLineEdit(this);
    m_dirEdit->setText(currentDirectory.isEmpty() ? QDir::homePath() + "/Downloads" : currentDirectory);
    QPushButton* browseBtn = new QPushButton("Browse...", this);
    connect(browseBtn, &QPushButton::clicked, this, &EvalinkDialog::onBrowseDirectory);
    dirLayout->addWidget(m_dirEdit);
    dirLayout->addWidget(browseBtn);
    layout->addLayout(dirLayout);

    // Filename
    QLabel* fnLabel = new QLabel("Custom Filename (Optional):", this);
    fnLabel->setStyleSheet("color: #b58487; font-size: 12px;");
    m_filenameEdit = new QLineEdit(this);
    m_filenameEdit->setPlaceholderText("Leave empty to use remote filename");
    layout->addWidget(fnLabel);
    layout->addWidget(m_filenameEdit);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch(1);

    m_cancelBtn = new QPushButton("Cancel", this);
    m_startBtn = new QPushButton("Start Download", this);
    m_startBtn->setStyleSheet(
        "background-color: #cf2824; color: #ffffff; font-weight: bold; border-radius: 4px; padding: 6px 16px;"
    );

    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_startBtn, &QPushButton::clicked, this, &EvalinkDialog::onStartDownload);

    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addWidget(m_startBtn);
    layout->addLayout(btnLayout);
}

QString EvalinkDialog::url() const {
    return m_urlEdit->text().trimmed();
}

QString EvalinkDialog::destinationDirectory() const {
    return m_dirEdit->text().trimmed();
}

QString EvalinkDialog::customFilename() const {
    return m_filenameEdit->text().trimmed();
}

void EvalinkDialog::onBrowseDirectory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Download Directory", m_dirEdit->text());
    if (!dir.isEmpty()) {
        m_dirEdit->setText(dir);
    }
}

void EvalinkDialog::onStartDownload() {
    if (url().isEmpty()) {
        QMessageBox::warning(this, "Missing URL", "Please enter a valid download URL.");
        return;
    }
    accept();
}
