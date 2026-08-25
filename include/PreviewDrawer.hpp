#pragma once

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>

class PreviewDrawer : public QWidget {
    Q_OBJECT
public:
    explicit PreviewDrawer(QWidget* parent = nullptr);

    void setFile(const QString& filePath);
    void clearPreview();

signals:
    void openFileRequested(const QString& path);
    void openInTerminalRequested(const QString& dirPath);

private slots:
    void onCalculateHash();
    void onOpenDefault();
    void onCopyPath();

private:
    void renderImagePreview(const QString& path);
    void renderPdfPreview(const QString& path);
    void renderArchivePreview(const QString& path);
    void renderTextPreview(const QString& path);
    void renderGenericPreview(const QString& path);
    QString formatSize(qint64 bytes) const;
    QString calculateSHA256(const QString& path) const;

    QString m_currentPath;
    QLabel* m_iconLabel;
    QLabel* m_nameLabel;
    QLabel* m_sizeLabel;
    QLabel* m_typeLabel;
    QLabel* m_dateLabel;
    QLabel* m_permsLabel;
    QLabel* m_imagePreviewLabel;
    QTextEdit* m_textPreviewEdit;
    QLabel* m_hashLabel;
    QPushButton* m_hashButton;
    QWidget* m_previewContainer;
};
