#include "PreviewDrawer.hpp"
#include "EvaSuiteBridge.hpp"
#include <QFileInfo>
#include <QDateTime>
#include <QImageReader>
#include <QPixmap>
#include <QFile>
#include <QCryptographicHash>
#include <QClipboard>
#include <QGuiApplication>
#include <QMimeDatabase>
#include <QMimeType>
#include <QIcon>
#include <QFrame>

#if defined(HAVE_POPPLER_QT6)
#include <poppler-qt6.h>
#endif

PreviewDrawer::PreviewDrawer(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("PreviewContainer");
    setMinimumWidth(260);
    setMaximumWidth(380);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // Header Title
    QLabel* title = new QLabel("INFORMATION & PREVIEW", this);
    title->setStyleSheet("color: #e32a10; font-size: 11px; font-weight: bold; letter-spacing: 0.5px;");
    mainLayout->addWidget(title);

    // Visual Preview Area
    m_imagePreviewLabel = new QLabel(this);
    m_imagePreviewLabel->setAlignment(Qt::AlignCenter);
    m_imagePreviewLabel->setFixedHeight(180);
    m_imagePreviewLabel->setStyleSheet("background-color: #25090a; border: 1px solid #6e282c; border-radius: 6px;");
    m_imagePreviewLabel->setVisible(false);
    mainLayout->addWidget(m_imagePreviewLabel);

    // Text Preview Area
    m_textPreviewEdit = new QTextEdit(this);
    m_textPreviewEdit->setReadOnly(true);
    m_textPreviewEdit->setFixedHeight(180);
    m_textPreviewEdit->setStyleSheet("background-color: #25090a; border: 1px solid #6e282c; font-family: 'JetBrains Mono', 'Fira Code', monospace; font-size: 11px; color: #ebdada;");
    m_textPreviewEdit->setVisible(false);
    mainLayout->addWidget(m_textPreviewEdit);

    // Metadata Card
    QFrame* metaCard = new QFrame(this);
    metaCard->setStyleSheet("background-color: #2a0a0c; border: 1px solid #451316; border-radius: 6px; padding: 6px;");
    QVBoxLayout* metaLayout = new QVBoxLayout(metaCard);
    metaLayout->setContentsMargins(8, 8, 8, 8);
    metaLayout->setSpacing(6);

    m_nameLabel = new QLabel(metaCard);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #ffffff;");

    m_sizeLabel = new QLabel(metaCard);
    m_sizeLabel->setStyleSheet("color: #b58487; font-size: 12px;");

    m_typeLabel = new QLabel(metaCard);
    m_typeLabel->setStyleSheet("color: #b58487; font-size: 12px;");

    m_dateLabel = new QLabel(metaCard);
    m_dateLabel->setStyleSheet("color: #b58487; font-size: 12px;");

    m_permsLabel = new QLabel(metaCard);
    m_permsLabel->setStyleSheet("color: #b58487; font-size: 12px; font-family: monospace;");

    m_hashLabel = new QLabel(metaCard);
    m_hashLabel->setStyleSheet("color: #e6ab3c; font-size: 10px; font-family: monospace;");
    m_hashLabel->setWordWrap(true);
    m_hashLabel->setVisible(false);

    metaLayout->addWidget(m_nameLabel);
    metaLayout->addWidget(m_sizeLabel);
    metaLayout->addWidget(m_typeLabel);
    metaLayout->addWidget(m_dateLabel);
    metaLayout->addWidget(m_permsLabel);
    metaLayout->addWidget(m_hashLabel);

    mainLayout->addWidget(metaCard);

    // Action buttons
    QPushButton* openBtn = new QPushButton("Open Default", this);
    QPushButton* copyPathBtn = new QPushButton("Copy Path", this);
    m_hashButton = new QPushButton("Calculate SHA256", this);

    connect(openBtn, &QPushButton::clicked, this, &PreviewDrawer::onOpenDefault);
    connect(copyPathBtn, &QPushButton::clicked, this, &PreviewDrawer::onCopyPath);
    connect(m_hashButton, &QPushButton::clicked, this, &PreviewDrawer::onCalculateHash);

    mainLayout->addWidget(openBtn);
    mainLayout->addWidget(copyPathBtn);
    mainLayout->addWidget(m_hashButton);

    mainLayout->addStretch(1);

    clearPreview();
}

QString PreviewDrawer::formatSize(qint64 bytes) const {
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return QString("%1 KB").arg(QString::number(bytes / 1024.0, 'f', 1));
    if (bytes < 1024 * 1024 * 1024) return QString("%1 MB").arg(QString::number(bytes / (1024.0 * 1024.0), 'f', 1));
    return QString("%1 GB").arg(QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2));
}

void PreviewDrawer::clearPreview() {
    m_currentPath.clear();
    m_nameLabel->setText("No file selected");
    m_sizeLabel->setText("Size: -");
    m_typeLabel->setText("Type: -");
    m_dateLabel->setText("Modified: -");
    m_permsLabel->setText("Permissions: -");
    m_imagePreviewLabel->setVisible(false);
    m_textPreviewEdit->setVisible(false);
    m_hashLabel->setVisible(false);
    m_hashButton->setEnabled(false);
}

void PreviewDrawer::setFile(const QString& filePath) {
    if (filePath.isEmpty() || !QFileInfo::exists(filePath)) {
        clearPreview();
        return;
    }

    m_currentPath = filePath;
    m_hashButton->setEnabled(true);
    m_hashLabel->setVisible(false);

    QFileInfo fi(filePath);
    m_nameLabel->setText(fi.fileName());
    m_sizeLabel->setText(QString("Size: %1").arg(fi.isDir() ? "Directory" : formatSize(fi.size())));
    m_dateLabel->setText(QString("Modified: %1").arg(fi.lastModified().toString("yyyy-MM-dd HH:mm:ss")));

    // Permissions string
    QFile::Permissions p = fi.permissions();
    QString perms;
    perms += (fi.isDir() ? "d" : "-");
    perms += (p & QFile::ReadUser ? "r" : "-");
    perms += (p & QFile::WriteUser ? "w" : "-");
    perms += (p & QFile::ExeUser ? "x" : "-");
    perms += (p & QFile::ReadGroup ? "r" : "-");
    perms += (p & QFile::WriteGroup ? "w" : "-");
    perms += (p & QFile::ExeGroup ? "x" : "-");
    perms += (p & QFile::ReadOther ? "r" : "-");
    perms += (p & QFile::WriteOther ? "w" : "-");
    perms += (p & QFile::ExeOther ? "x" : "-");
    m_permsLabel->setText(QString("Permissions: %1").arg(perms));

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filePath);
    m_typeLabel->setText(QString("Type: %1").arg(mime.comment().isEmpty() ? mime.name() : mime.comment()));

    // Preview classification
    QString suffix = fi.suffix().toLower();
    QString mimeName = mime.name();

    if (mimeName.startsWith("image/") || suffix == "png" || suffix == "jpg" || suffix == "jpeg" || suffix == "gif" || suffix == "webp" || suffix == "svg" || suffix == "bmp") {
        renderImagePreview(filePath);
    } else if (suffix == "pdf" || mimeName == "application/pdf") {
        renderPdfPreview(filePath);
    } else if (mimeName.startsWith("text/") || suffix == "txt" || suffix == "cpp" || suffix == "hpp" || suffix == "c" || suffix == "h" || suffix == "rs" || suffix == "py" || suffix == "json" || suffix == "md" || suffix == "toml" || suffix == "ini" || suffix == "conf" || suffix == "sh" || suffix == "cmake") {
        renderTextPreview(filePath);
    } else {
        renderGenericPreview(filePath);
    }
}

void PreviewDrawer::renderImagePreview(const QString& path) {
    m_textPreviewEdit->setVisible(false);
    m_imagePreviewLabel->setVisible(true);

    QPixmap pix(path);
    if (!pix.isNull()) {
        m_imagePreviewLabel->setPixmap(pix.scaled(m_imagePreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_typeLabel->setText(m_typeLabel->text() + QString(" (%1x%2)").arg(pix.width()).arg(pix.height()));
    } else {
        m_imagePreviewLabel->setText("Preview not available");
    }
}

void PreviewDrawer::renderPdfPreview(const QString& path) {
#if defined(HAVE_POPPLER_QT6)
    std::unique_ptr<Poppler::Document> doc = Poppler::Document::load(path);
    if (doc && !doc->isLocked()) {
        std::unique_ptr<Poppler::Page> page = doc->page(0);
        if (page) {
            QImage img = page->renderToImage(100, 100);
            if (!img.isNull()) {
                m_textPreviewEdit->setVisible(false);
                m_imagePreviewLabel->setVisible(true);
                m_imagePreviewLabel->setPixmap(QPixmap::fromImage(img).scaled(m_imagePreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                return;
            }
        }
    }
#endif
    renderGenericPreview(path);
}

void PreviewDrawer::renderTextPreview(const QString& path) {
    m_imagePreviewLabel->setVisible(false);
    m_textPreviewEdit->setVisible(true);

    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.read(4096); // Preview first 4KB
        m_textPreviewEdit->setPlainText(content);
        file.close();
    } else {
        m_textPreviewEdit->setPlainText("Could not read file contents");
    }
}

void PreviewDrawer::renderGenericPreview(const QString& /*path*/) {
    m_imagePreviewLabel->setVisible(false);
    m_textPreviewEdit->setVisible(false);
}

void PreviewDrawer::onOpenDefault() {
    if (!m_currentPath.isEmpty()) {
        EvaSuiteBridge::openWithDefaultApp(m_currentPath);
    }
}

void PreviewDrawer::onCopyPath() {
    if (!m_currentPath.isEmpty()) {
        QGuiApplication::clipboard()->setText(m_currentPath);
    }
}

void PreviewDrawer::onCalculateHash() {
    if (!m_currentPath.isEmpty() && !QFileInfo(m_currentPath).isDir()) {
        QString hash = calculateSHA256(m_currentPath);
        m_hashLabel->setText(QString("SHA256: %1").arg(hash));
        m_hashLabel->setVisible(true);
    }
}

QString PreviewDrawer::calculateSHA256(const QString& path) const {
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Sha256);
        if (hash.addData(&file)) {
            return hash.result().toHex();
        }
    }
    return "Error calculating hash";
}
