#include "LspManagerDialog.h"

#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace {
constexpr int kMargin = 10;
constexpr int kSpacing = 8;

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
    resize(560, 540);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
    layout->setSpacing(kSpacing);

    m_titleLabel = new QLabel(this);
    layout->addWidget(m_titleLabel);

    m_hintLabel = new QLabel(this);
    m_hintLabel->setWordWrap(true);
    layout->addWidget(m_hintLabel);

    m_runtimeTitleLabel = new QLabel(this);
    layout->addWidget(m_runtimeTitleLabel);

    m_runtimeHintLabel = new QLabel(this);
    m_runtimeHintLabel->setWordWrap(true);
    layout->addWidget(m_runtimeHintLabel);

    m_lspEnabledCheck = new QCheckBox(this);
    layout->addWidget(m_lspEnabledCheck);

    auto makeValueLabel = []() {
        auto *label = new QLabel("Unavailable");
        label->setWordWrap(true);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        label->setStyleSheet("font-size: 12px;");
        return label;
    };

    auto *runtimeForm = new QFormLayout;
    runtimeForm->setContentsMargins(0, 4, 0, 0);
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
    diagForm->setContentsMargins(0, 4, 0, 0);
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

    m_logLabel = new QLabel(this);
    layout->addWidget(m_logLabel);

    m_lspLogView = new QPlainTextEdit(this);
    m_lspLogView->setReadOnly(true);
    m_lspLogView->setMaximumBlockCount(400);
    layout->addWidget(m_lspLogView, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);
    layout->addWidget(buttons);

    connect(m_lspEnabledCheck, &QCheckBox::toggled,
            this, &LspManagerDialog::lspEnabledChanged);
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
    const QString borderHex =
        tm.customColor("sidebarBorder", QColor("#363a4f")).name();

    setStyleSheetIfChanged(
        m_titleLabel,
        QString("color: %1; font-weight: bold; font-size: 14px;")
            .arg(windowTextHex));
    setStyleSheetIfChanged(
        m_hintLabel, QString("color: %1; font-size: 12px;").arg(textHex));
    setStyleSheetIfChanged(
        m_runtimeTitleLabel,
        QString("color: %1; font-weight: bold; font-size: 13px;")
            .arg(windowTextHex));
    setStyleSheetIfChanged(
        m_runtimeHintLabel, QString("color: %1; font-size: 12px;").arg(textHex));
    setStyleSheetIfChanged(
        m_diagTitleLabel,
        QString("color: %1; font-weight: bold; font-size: 13px; padding-top: 8px;")
            .arg(windowTextHex));
    setStyleSheetIfChanged(
        m_logLabel, QString("color: %1; font-size: 12px; padding-top: 4px;").arg(textHex));
    setStyleSheetIfChanged(
        m_lspLastErrorValue,
        QString("color: %1; font-size: 12px;")
            .arg(pal.color(QPalette::BrightText).name()));
    setStyleSheetIfChanged(
        m_lspLogView,
        QString("QPlainTextEdit { background: %1; color: %2; border: 1px solid %3; border-radius: 4px; font-family: monospace; font-size: 11px; }")
            .arg(altBaseHex, textHex, borderHex));
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

    updateRuntimeInfoDisplay();
    updateDiagnosticsDisplay();
}
