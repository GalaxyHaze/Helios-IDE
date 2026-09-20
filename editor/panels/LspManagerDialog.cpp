#include "LspManagerDialog.h"

#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace {
constexpr int kMargin = 10;
constexpr int kSpacing = 8;
constexpr int kSectionSpacing = 14;

QFrame *makeSectionDivider()
{
    auto *divider = new QFrame;
    divider->setFrameShape(QFrame::HLine);
    divider->setObjectName("lspSectionDivider");
    return divider;
}

void setStyleSheetIfChanged(QWidget *widget, const QString &styleSheet)
{
    if (widget->styleSheet() != styleSheet)
        widget->setStyleSheet(styleSheet);
}
}

LspManagerDialog::LspManagerDialog(QWidget *parent)
    : QDialog(parent)
{
    setModal(false);
    resize(680, 620);
    setMinimumWidth(560);

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    auto *scrollContent = new QWidget;
    auto *layout = new QVBoxLayout(scrollContent);
    layout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
    layout->setSpacing(kSpacing);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("lspTitle");
    layout->addWidget(m_titleLabel);

    m_hintLabel = new QLabel(this);
    m_hintLabel->setWordWrap(true);
    layout->addWidget(m_hintLabel);

    layout->addSpacing(kSectionSpacing);
    m_runtimeTitleLabel = new QLabel(this);
    m_runtimeTitleLabel->setObjectName("lspSectionTitle");
    layout->addWidget(m_runtimeTitleLabel);

    m_runtimeHintLabel = new QLabel(this);
    m_runtimeHintLabel->setWordWrap(true);
    layout->addWidget(m_runtimeHintLabel);

    m_lspEnabledCheck = new QCheckBox(this);
    layout->addWidget(m_lspEnabledCheck);

    m_useOnlineZithLspCheck = new QCheckBox(
        "Prefer the latest online Zith LSP instead of the local checkout", this);
    layout->addWidget(m_useOnlineZithLspCheck);

    auto makeValueLabel = []() {
        auto *label = new QLabel("Unavailable");
        label->setWordWrap(true);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        label->setObjectName("lspValue");
        return label;
    };

    auto *runtimeForm = new QFormLayout;
    runtimeForm->setContentsMargins(2, 4, 2, 0);
    runtimeForm->setSpacing(kSpacing);
    m_runtimeStatusValue = makeValueLabel();
    m_runtimeTagValue = makeValueLabel();
    m_runtimeLspPathValue = makeValueLabel();
    m_runtimeStdlibPathValue = makeValueLabel();
    m_runtimeCachePathValue = makeValueLabel();
    m_labelStatus = new QLabel("Status", this);
    m_labelTag = new QLabel("Tag", this);
    m_labelLsp = new QLabel("LSP", this);
    m_labelStdlib = new QLabel("Stdlib", this);
    m_labelCache = new QLabel("Cache", this);
    runtimeForm->addRow(m_labelStatus, m_runtimeStatusValue);
    runtimeForm->addRow(m_labelTag, m_runtimeTagValue);
    runtimeForm->addRow(m_labelLsp, m_runtimeLspPathValue);
    runtimeForm->addRow(m_labelStdlib, m_runtimeStdlibPathValue);
    runtimeForm->addRow(m_labelCache, m_runtimeCachePathValue);
    layout->addLayout(runtimeForm);

    auto *runtimeButtons = new QHBoxLayout;
    runtimeButtons->setContentsMargins(0, 0, 0, 0);
    runtimeButtons->setSpacing(6);
    m_refreshRuntimeButton = new QPushButton(this);
    m_clearRuntimeCacheButton = new QPushButton(this);
    runtimeButtons->addWidget(m_refreshRuntimeButton);
    runtimeButtons->addWidget(m_clearRuntimeCacheButton);
    runtimeButtons->addStretch();
    layout->addLayout(runtimeButtons);

    m_diagTitleLabel = new QLabel(this);
    layout->addWidget(m_diagTitleLabel);

    auto *diagForm = new QFormLayout;
    diagForm->setContentsMargins(2, 4, 2, 0);
    diagForm->setSpacing(kSpacing);
    m_lspConnectionValue = makeValueLabel();
    m_lspSyncModeValue = makeValueLabel();
    m_lspLastErrorValue = makeValueLabel();
    m_labelConnection = new QLabel("Connection", this);
    m_labelSyncMode = new QLabel("Sync mode", this);
    m_labelLastError = new QLabel("Last error", this);
    diagForm->addRow(m_labelConnection, m_lspConnectionValue);
    diagForm->addRow(m_labelSyncMode, m_lspSyncModeValue);
    diagForm->addRow(m_labelLastError, m_lspLastErrorValue);
    layout->addLayout(diagForm);

    m_cLspTitle = new QLabel("C/C++ LSP (clangd)");
    m_cLspTitle->setObjectName("cLspTitle");
    layout->addWidget(m_cLspTitle);

    m_cLspEnabledCheck = new QCheckBox(this);
    m_cLspEnabledCheck->setText("Enable clangd for C-family files");
    layout->addWidget(m_cLspEnabledCheck);

    auto *cLspPathRow = new QHBoxLayout;
    cLspPathRow->setContentsMargins(0, 0, 0, 0);
    cLspPathRow->setSpacing(6);
    m_cLspPathEdit = new QLineEdit(this);
    m_cLspPathEdit->setPlaceholderText("clangd binary or empty to auto-detect from PATH");
    m_cLspPathEdit->setMinimumWidth(320);
    m_cLspBrowseButton = new QPushButton("Browse...", this);
    cLspPathRow->addWidget(m_cLspPathEdit, 1);
    cLspPathRow->addWidget(m_cLspBrowseButton);
    layout->addLayout(cLspPathRow);

    auto *cLspForm = new QFormLayout;
    cLspForm->setContentsMargins(2, 4, 2, 0);
    cLspForm->setSpacing(kSpacing);
    m_labelCLspStatus = new QLabel("Status", this);
    m_labelCLspPath = new QLabel("Resolved path", this);
    m_labelCLspMessage = new QLabel("Message", this);
    m_cLspStatusValue = makeValueLabel();
    m_cLspPathValue = makeValueLabel();
    m_cLspMessageValue = makeValueLabel();
    cLspForm->addRow(m_labelCLspStatus, m_cLspStatusValue);
    cLspForm->addRow(m_labelCLspPath, m_cLspPathValue);
    cLspForm->addRow(m_labelCLspMessage, m_cLspMessageValue);
    layout->addLayout(cLspForm);

    layout->addWidget(makeSectionDivider());
    m_logLabel = new QLabel(this);
    m_logLabel->setObjectName("lspLogTitle");
    layout->addWidget(m_logLabel);

    m_lspLogView = new QPlainTextEdit(this);
    m_lspLogView->setReadOnly(true);
    m_lspLogView->setMaximumBlockCount(400);
    m_lspLogView->setMinimumHeight(160);
    m_lspLogView->setLineWrapMode(QPlainTextEdit::NoWrap);
    layout->addWidget(m_lspLogView, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);

    scrollArea->setWidget(scrollContent);
    outerLayout->addWidget(scrollArea, 1);
    auto *buttonBar = new QWidget(this);
    auto *buttonLayout = new QHBoxLayout(buttonBar);
    buttonLayout->setContentsMargins(kMargin + 4, 6, kMargin + 4, kMargin);
    buttonLayout->addWidget(buttons);
    outerLayout->addWidget(buttonBar);

    connect(m_lspEnabledCheck, &QCheckBox::toggled,
            this, &LspManagerDialog::lspEnabledChanged);
    connect(m_useOnlineZithLspCheck, &QCheckBox::toggled,
            this, &LspManagerDialog::useOnlineZithLspChanged);
    connect(m_cLspEnabledCheck, &QCheckBox::toggled,
            this, &LspManagerDialog::cLspEnabledChanged);
    connect(m_cLspPathEdit, &QLineEdit::editingFinished, this,
            [this]() { emit cLspPathChanged(m_cLspPathEdit->text().trimmed()); });
    connect(m_cLspBrowseButton, &QPushButton::clicked, this, [this]() {
        const QString path =
            QFileDialog::getOpenFileName(this, "Select clangd executable",
                                         m_cLspPathEdit->text(), "clangd (*)");
        if (!path.isEmpty()) {
            m_cLspPathEdit->setText(path);
            emit cLspPathChanged(path);
        }
    });
    connect(m_refreshRuntimeButton, &QPushButton::clicked,
            this, &LspManagerDialog::refreshRuntimeRequested);
    connect(m_clearRuntimeCacheButton, &QPushButton::clicked,
            this, &LspManagerDialog::clearRuntimeCacheRequested);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &LspManagerDialog::applyTheme);
    connect(&TranslationManager::instance(), &TranslationManager::localeChanged,
            this, &LspManagerDialog::applyTranslations);

    applyTheme();
    applyTranslations();
}

void LspManagerDialog::setLspEnabled(bool enabled)
{
    QSignalBlocker blocker(m_lspEnabledCheck);
    m_lspEnabledCheck->setChecked(enabled);
    m_refreshRuntimeButton->setEnabled(enabled);
}

void LspManagerDialog::setUseOnlineZithLsp(bool enabled)
{
    QSignalBlocker blocker(m_useOnlineZithLspCheck);
    m_useOnlineZithLspCheck->setChecked(enabled);
}

void LspManagerDialog::setCLspEnabled(bool enabled)
{
    QSignalBlocker blocker(m_cLspEnabledCheck);
    m_cLspEnabledCheck->setChecked(enabled);
}

void LspManagerDialog::setCLspPath(const QString &path)
{
    QSignalBlocker blocker(m_cLspPathEdit);
    m_cLspPathEdit->setText(path);
}

void LspManagerDialog::setCLspInfo(const QString &status,
                                   const QString &resolvedPath,
                                   const QString &message)
{
    m_cLspStatusValue->setText(status.isEmpty() ? "Unavailable" : status);
    m_cLspPathValue->setText(resolvedPath.isEmpty() ? "Unavailable" : resolvedPath);
    m_cLspMessageValue->setText(message.isEmpty() ? QString() : message);
}

void LspManagerDialog::setRuntimeInfo(const QString &status,
                                      const QString &tag,
                                      const QString &lspPath,
                                      const QString &stdlibPath,
                                      const QString &cachePath)
{
    m_rawStatus = status;
    m_rawTag = tag;
    m_rawLspPath = lspPath;
    m_rawStdlibPath = stdlibPath;
    m_rawCachePath = cachePath;
    updateRuntimeInfoDisplay();
}

void LspManagerDialog::updateRuntimeInfoDisplay()
{
    auto &tr = TranslationManager::instance();
    const QString unav = tr.translate("lsp.val_unavailable");
    m_runtimeStatusValue->setText(m_rawStatus.isEmpty() ? unav : m_rawStatus);
    m_runtimeTagValue->setText(m_rawTag.isEmpty() ? unav : m_rawTag);
    m_runtimeLspPathValue->setText(m_rawLspPath.isEmpty() ? unav : m_rawLspPath);
    m_runtimeStdlibPathValue->setText(m_rawStdlibPath.isEmpty() ? unav : m_rawStdlibPath);
    m_runtimeCachePathValue->setText(m_rawCachePath.isEmpty() ? unav : m_rawCachePath);
}

void LspManagerDialog::setLspDiagnostics(const QString &connection,
                                         const QString &syncMode,
                                         const QString &lastError)
{
    m_rawConnection = connection;
    m_rawSyncMode = syncMode;
    m_rawLastError = lastError;
    updateDiagnosticsDisplay();
}

void LspManagerDialog::updateDiagnosticsDisplay()
{
    auto &tr = TranslationManager::instance();
    const QString unknown = tr.translate("lsp.val_unknown");
    const QString none = tr.translate("lsp.val_none");
    m_lspConnectionValue->setText(m_rawConnection.isEmpty() ? unknown : m_rawConnection);
    m_lspSyncModeValue->setText(m_rawSyncMode.isEmpty() ? unknown : m_rawSyncMode);
    m_lspLastErrorValue->setText(m_rawLastError.isEmpty() ? none : m_rawLastError);
}

void LspManagerDialog::appendLspLog(const QString &line)
{
    if (!line.isEmpty())
        m_lspLogView->appendPlainText(line);
}

void LspManagerDialog::clearLspLog()
{
    m_lspLogView->clear();
}

void LspManagerDialog::applyTheme()
{
    auto &tm = ThemeManager::instance();
    QPalette pal = tm.palette();
    const QString textHex = pal.color(QPalette::Text).name();
    const QString windowTextHex = pal.color(QPalette::WindowText).name();
    const QString altBaseHex = pal.color(QPalette::AlternateBase).name();
    const QString canvasHex = pal.color(QPalette::Window).name();
    const QString borderHex =
        tm.customColor("sidebarBorder", QColor("#363a4f")).name();

    setStyleSheetIfChanged(
        this,
        QString("QDialog { background: %1; }"
                "QScrollArea { background: transparent; border: none; }"
                "QScrollArea > QWidget > QWidget { background: transparent; }")
            .arg(canvasHex));
    setStyleSheetIfChanged(
        m_titleLabel,
        QString("color: %1; font-weight: bold; font-size: 15px;")
            .arg(windowTextHex));
    setStyleSheetIfChanged(
        m_hintLabel, QString("color: %1; font-size: 12px; padding-bottom: 6px;").arg(textHex));
    setStyleSheetIfChanged(
        m_runtimeTitleLabel,
        QString("color: %1; font-weight: bold; font-size: 13px; padding-bottom: 2px;")
            .arg(windowTextHex));
    setStyleSheetIfChanged(
        m_runtimeHintLabel, QString("color: %1; font-size: 12px;").arg(textHex));
    setStyleSheetIfChanged(
        m_diagTitleLabel,
        QString("color: %1; font-weight: bold; font-size: 13px; padding-top: 8px; border-top: 1px solid %2; padding-top: 10px; margin-top: 6px;")
            .arg(windowTextHex, borderHex));
    setStyleSheetIfChanged(
        m_logLabel,
        QString("color: %1; font-size: 12px;")
            .arg(windowTextHex));
    setStyleSheetIfChanged(
        m_lspLastErrorValue,
        QString("color: %1; font-size: 12px;")
            .arg(pal.color(QPalette::BrightText).name()));
    for (QLabel *label : {
             m_labelStatus, m_labelTag, m_labelLsp, m_labelStdlib,
             m_labelCache, m_labelConnection, m_labelSyncMode,
             m_labelLastError, m_labelCLspStatus, m_labelCLspPath,
             m_labelCLspMessage}) {
        setStyleSheetIfChanged(
            label,
            QString("color: %1; font-size: 12px; font-weight: 600;")
                .arg(textHex));
    }
    for (QLabel *label : {
             m_runtimeStatusValue, m_runtimeTagValue, m_runtimeLspPathValue,
             m_runtimeStdlibPathValue, m_runtimeCachePathValue,
             m_lspConnectionValue, m_lspSyncModeValue, m_cLspStatusValue,
             m_cLspPathValue, m_cLspMessageValue}) {
        setStyleSheetIfChanged(
            label, QString("color: %1; font-size: 12px;").arg(textHex));
    }
    setStyleSheetIfChanged(
        m_cLspTitle,
        QString("color: %1; font-weight: bold; font-size: 13px; padding-top: 2px;")
            .arg(windowTextHex));
    setStyleSheetIfChanged(
        m_lspLogView,
        QString("QPlainTextEdit { background: %1; color: %2; border: 1px solid %3; border-radius: 4px; font-family: 'DejaVu Sans Mono', 'Fira Code', monospace; font-size: 11px; }")
            .arg(altBaseHex, textHex, borderHex));
    setStyleSheetIfChanged(
        m_lspEnabledCheck, QString("color: %1; font-size: 12px;").arg(textHex));
    setStyleSheetIfChanged(
        m_cLspEnabledCheck,
        QString("color: %1; font-size: 12px;").arg(textHex));
    setStyleSheetIfChanged(
        m_cLspPathEdit,
        QString("QLineEdit { background: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 4px 6px; }")
            .arg(altBaseHex, textHex, borderHex));
    for (QPushButton *button : {m_refreshRuntimeButton, m_clearRuntimeCacheButton}) {
        setStyleSheetIfChanged(
            button,
            QString("QPushButton { color: %1; background: %2; border: 1px solid %3; border-radius: 4px; padding: 5px 10px; }"
                    "QPushButton:hover { background: %4; }")
                .arg(textHex, altBaseHex, borderHex,
                     tm.customColor("treeHover", QColor("#363a4f")).name()));
    }
}

void LspManagerDialog::applyTranslations()
{
    auto &tr = TranslationManager::instance();
    setWindowTitle(tr.translate("lsp.window_title"));
    m_titleLabel->setText(tr.translate("lsp.title"));
    m_hintLabel->setText(tr.translate("lsp.hint"));
    m_runtimeTitleLabel->setText(tr.translate("lsp.runtime_title"));
    m_runtimeHintLabel->setText(tr.translate("lsp.runtime_hint"));
    m_lspEnabledCheck->setText(tr.translate("lsp.enable"));
    m_refreshRuntimeButton->setText(tr.translate("lsp.refresh"));
    m_clearRuntimeCacheButton->setText(tr.translate("lsp.clear_cache"));
    m_diagTitleLabel->setText(tr.translate("lsp.diagnostics_title"));
    m_logLabel->setText(tr.translate("lsp.log_title"));

    m_labelStatus->setText(tr.translate("lsp.label_status"));
    m_labelTag->setText(tr.translate("lsp.label_tag"));
    m_labelLsp->setText(tr.translate("lsp.label_lsp"));
    m_labelStdlib->setText(tr.translate("lsp.label_stdlib"));
    m_labelCache->setText(tr.translate("lsp.label_cache"));
    m_labelConnection->setText(tr.translate("lsp.label_connection"));
    m_labelSyncMode->setText(tr.translate("lsp.label_sync_mode"));
    m_labelLastError->setText(tr.translate("lsp.label_last_error"));
    m_cLspTitle->setText(tr.translate("lsp.c_title"));
    m_cLspEnabledCheck->setText(tr.translate("lsp.c_enable"));
    m_cLspPathEdit->setPlaceholderText(tr.translate("lsp.c_path_placeholder"));
    m_cLspBrowseButton->setText(tr.translate("lsp.c_browse"));
    m_labelCLspStatus->setText(tr.translate("lsp.c_status"));
    m_labelCLspPath->setText(tr.translate("lsp.c_resolved_path"));
    m_labelCLspMessage->setText(tr.translate("lsp.c_message"));

    updateRuntimeInfoDisplay();
    updateDiagnosticsDisplay();
}
