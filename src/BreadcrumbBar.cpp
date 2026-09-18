#include "BreadcrumbBar.hpp"
#include "DragDropHelper.hpp"
#include <QDir>
#include <QFileInfo>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QShortcut>

class BreadcrumbPill : public QPushButton {
public:
    explicit BreadcrumbPill(const QString& text, const QString& path, QWidget* parent = nullptr)
        : QPushButton(text, parent), m_path(path)
    {
        setProperty("class", "breadcrumb-pill");
        setProperty("path", path);
        setAcceptDrops(true);
    }

protected:
    void dragEnterEvent(QDragEnterEvent* event) override {
        if (DragDropHelper::handleDragEnter(event)) {
            event->acceptProposedAction();
        } else {
            QPushButton::dragEnterEvent(event);
        }
    }

    void dragMoveEvent(QDragMoveEvent* event) override {
        if (DragDropHelper::handleDragMove(event)) {
            event->acceptProposedAction();
        } else {
            QPushButton::dragMoveEvent(event);
        }
    }

    void dropEvent(QDropEvent* event) override {
        if (!m_path.isEmpty() && QDir(m_path).exists()) {
            DragDropHelper::executeDrop(event, m_path, this);
        } else {
            event->ignore();
        }
    }

private:
    QString m_path;
};

BreadcrumbBar::BreadcrumbBar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("BreadcrumbContainer");
    setFixedHeight(34);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(4, 2, 4, 2);
    mainLayout->setSpacing(4);

    // Pill container
    m_pillContainer = new QWidget(this);
    m_pillLayout = new QHBoxLayout(m_pillContainer);
    m_pillLayout->setContentsMargins(0, 0, 0, 0);
    m_pillLayout->setSpacing(2);
    m_pillLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // Editable text address bar
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setVisible(false);
    m_pathEdit->setPlaceholderText("Enter folder path... (e.g. /home/camellia/Downloads)");

    m_completerModel = new QFileSystemModel(this);
    m_completerModel->setRootPath("/");
    m_completerModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Hidden);
    m_completer = new QCompleter(m_completerModel, this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_pathEdit->setCompleter(m_completer);

    mainLayout->addWidget(m_pillContainer, 1);
    mainLayout->addWidget(m_pathEdit, 1);

    connect(m_pathEdit, &QLineEdit::returnPressed, this, &BreadcrumbBar::onEditFinished);

    // Escape shortcut in line edit to cancel edit
    QShortcut* escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), m_pathEdit);
    connect(escShortcut, &QShortcut::activated, this, &BreadcrumbBar::switchToPillMode);

    setPath(QDir::homePath());
}

void BreadcrumbBar::setPath(const QString& path) {
    QString clean = QDir::cleanPath(path);
    if (clean.isEmpty()) clean = "/";
    m_currentPath = clean;
    m_pathEdit->setText(clean);
    updateBreadcrumbs();
    switchToPillMode();
}

void BreadcrumbBar::updateBreadcrumbs() {
    // Clear old pills safely
    QLayoutItem* item;
    while ((item = m_pillLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->hide();
            item->widget()->deleteLater();
        }
        delete item;
    }

    QStringList parts = m_currentPath.split('/', Qt::SkipEmptyParts);
    QString accumulatedPath = "";

    // Root button
    BreadcrumbPill* rootBtn = new BreadcrumbPill("/", "/", m_pillContainer);
    connect(rootBtn, &QPushButton::clicked, this, &BreadcrumbBar::onSegmentClicked);
    m_pillLayout->addWidget(rootBtn);

    for (int i = 0; i < parts.size(); ++i) {
        QLabel* sep = new QLabel("›", m_pillContainer);
        sep->setStyleSheet("color: #b58487; font-size: 13px; font-weight: bold; padding: 0 2px;");
        m_pillLayout->addWidget(sep);

        accumulatedPath += "/" + parts[i];
        BreadcrumbPill* btn = new BreadcrumbPill(parts[i], accumulatedPath, m_pillContainer);
        connect(btn, &QPushButton::clicked, this, &BreadcrumbBar::onSegmentClicked);
        m_pillLayout->addWidget(btn);
    }

    m_pillLayout->addStretch(1);
}

void BreadcrumbBar::onSegmentClicked() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        QString path = btn->property("path").toString();
        if (!path.isEmpty() && QDir(path).exists()) {
            m_currentPath = path;
            updateBreadcrumbs();
            emit pathChanged(path);
        }
    }
}

void BreadcrumbBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && !m_isEditMode) {
        // If clicked in empty space, switch to edit mode
        switchToEditMode();
    }
    QWidget::mousePressEvent(event);
}

void BreadcrumbBar::focusAddressBar() {
    switchToEditMode();
}

void BreadcrumbBar::switchToEditMode() {
    m_isEditMode = true;
    m_pillContainer->setVisible(false);
    m_pathEdit->setVisible(true);
    m_pathEdit->setText(m_currentPath);
    m_pathEdit->selectAll();
    m_pathEdit->setFocus();
}

void BreadcrumbBar::switchToPillMode() {
    m_isEditMode = false;
    m_pathEdit->setVisible(false);
    m_pillContainer->setVisible(true);
}

void BreadcrumbBar::onEditFinished() {
    QString target = m_pathEdit->text().trimmed();
    if (target.startsWith("~")) {
        target = QDir::homePath() + target.mid(1);
    }
    target = QDir::cleanPath(target);

    if (QDir(target).exists()) {
        m_currentPath = target;
        updateBreadcrumbs();
        switchToPillMode();
        emit pathChanged(target);
    } else {
        // Flash red or keep in edit mode
        m_pathEdit->setStyleSheet("border: 1px solid #e32a10; background-color: #441316;");
    }
}
