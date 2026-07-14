#include "SettingsPanel.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {
constexpr int kPanelMargin = 8;
constexpr int kPanelSpacing = 8;
constexpr int kFontSizeMin = 8;
constexpr int kFontSizeMax = 32;
}

SettingsPanel::SettingsPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(kPanelMargin, kPanelMargin, kPanelMargin, kPanelMargin);
    layout->setSpacing(kPanelSpacing);

    auto *title = new QLabel("Settings");
    title->setStyleSheet("color: #c6d0f5; font-weight: bold; font-size: 13px;");
    layout->addWidget(title);

    auto *hint = new QLabel("These options apply to the open editors in this window.");
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #8c8fa1; font-size: 12px;");
    layout->addWidget(hint);

    auto *form = new QFormLayout;
    form->setContentsMargins(0, 4, 0, 0);
    form->setSpacing(kPanelSpacing);

    m_fontSizeSpin = new QSpinBox;
    m_fontSizeSpin->setRange(kFontSizeMin, kFontSizeMax);
    form->addRow("Font size", m_fontSizeSpin);

    m_wordWrapCheck = new QCheckBox("Wrap long lines");
    form->addRow(QString(), m_wordWrapCheck);

    layout->addLayout(form);

    auto *runtimeTitle = new QLabel("Zith Runtime");
    runtimeTitle->setStyleSheet("color: #c6d0f5; font-weight: bold; font-size: 13px; padding-top: 8px;");
    layout->addWidget(runtimeTitle);

    auto *runtimeHint = new QLabel(
        "Helios manages the cached Zith LSP and stdlib runtime used by this window.");
    runtimeHint->setWordWrap(true);
    runtimeHint->setStyleSheet("color: #8c8fa1; font-size: 12px;");
    layout->addWidget(runtimeHint);

    auto *runtimeForm = new QFormLayout;
    runtimeForm->setContentsMargins(0, 4, 0, 0);
    runtimeForm->setSpacing(kPanelSpacing);

    auto makeValueLabel = []() {
        auto *label = new QLabel("Unavailable");
        label->setWordWrap(true);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        label->setStyleSheet("color: #c6d0f5; font-size: 12px;");
        return label;
    };

    m_runtimeStatusValue = makeValueLabel();
    m_runtimeTagValue = makeValueLabel();
    m_runtimeLspPathValue = makeValueLabel();
    m_runtimeStdlibPathValue = makeValueLabel();
    m_runtimeCachePathValue = makeValueLabel();

    runtimeForm->addRow("Status", m_runtimeStatusValue);
    runtimeForm->addRow("Tag", m_runtimeTagValue);
    runtimeForm->addRow("LSP", m_runtimeLspPathValue);
    runtimeForm->addRow("Stdlib", m_runtimeStdlibPathValue);
    runtimeForm->addRow("Cache", m_runtimeCachePathValue);
    layout->addLayout(runtimeForm);

    auto *runtimeButtons = new QHBoxLayout;
    runtimeButtons->setContentsMargins(0, 0, 0, 0);
    runtimeButtons->setSpacing(6);

    m_refreshRuntimeButton = new QPushButton("Refresh runtime");
    m_clearRuntimeCacheButton = new QPushButton("Clear cache");
    runtimeButtons->addWidget(m_refreshRuntimeButton);
    runtimeButtons->addWidget(m_clearRuntimeCacheButton);
    runtimeButtons->addStretch();
    layout->addLayout(runtimeButtons);

    auto *diagTitle = new QLabel("LSP Diagnostics");
    diagTitle->setStyleSheet("color: #c6d0f5; font-weight: bold; font-size: 13px; padding-top: 8px;");
    layout->addWidget(diagTitle);

    auto *diagForm = new QFormLayout;
    diagForm->setContentsMargins(0, 4, 0, 0);
    diagForm->setSpacing(kPanelSpacing);

    m_lspConnectionValue = makeValueLabel();
    m_lspSyncModeValue = makeValueLabel();
    m_lspLastErrorValue = makeValueLabel();
    m_lspLastErrorValue->setStyleSheet("color: #e78284; font-size: 12px;");

    diagForm->addRow("Connection", m_lspConnectionValue);
    diagForm->addRow("Sync mode", m_lspSyncModeValue);
    diagForm->addRow("Last error", m_lspLastErrorValue);
    layout->addLayout(diagForm);

    auto *logLabel = new QLabel("Recent LSP log");
    logLabel->setStyleSheet("color: #8c8fa1; font-size: 12px; padding-top: 4px;");
    layout->addWidget(logLabel);

    m_lspLogView = new QPlainTextEdit;
    m_lspLogView->setReadOnly(true);
    m_lspLogView->setMaximumBlockCount(200);
    m_lspLogView->setFixedHeight(140);
    m_lspLogView->setStyleSheet(
        "QPlainTextEdit { background: #1e1e2e; color: #a5adce; border: 1px solid #363a4f;"
        " border-radius: 4px; font-family: monospace; font-size: 11px; }");
    layout->addWidget(m_lspLogView);

    layout->addStretch();

    setStyleSheet(
        "SettingsPanel { background: #11111b; color: #c6d0f5; }"
        "QSpinBox { background: #1e1e2e; color: #c6d0f5; border: 1px solid #363a4f; border-radius: 4px; padding: 4px 6px; }"
        "QPushButton { background: #363a4f; color: #c6d0f5; border: none; border-radius: 4px; padding: 6px 10px; }"
        "QPushButton:hover { background: #45475a; }"
        "QCheckBox { color: #c6d0f5; }"
        "QLabel { color: #c6d0f5; }"
    );

    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsPanel::fontSizeChanged);
    connect(m_wordWrapCheck, &QCheckBox::toggled,
            this, &SettingsPanel::wordWrapChanged);
    connect(m_refreshRuntimeButton, &QPushButton::clicked,
            this, &SettingsPanel::refreshRuntimeRequested);
    connect(m_clearRuntimeCacheButton, &QPushButton::clicked,
            this, &SettingsPanel::clearRuntimeCacheRequested);
}

void SettingsPanel::setFontSize(int pointSize)
{
    m_fontSizeSpin->setValue(pointSize);
}

void SettingsPanel::setWordWrapEnabled(bool enabled)
{
    m_wordWrapCheck->setChecked(enabled);
}

void SettingsPanel::setRuntimeInfo(const QString &status,
                                   const QString &tag,
                                   const QString &lspPath,
                                   const QString &stdlibPath,
                                   const QString &cachePath)
{
    m_runtimeStatusValue->setText(status.isEmpty() ? "Unavailable" : status);
    m_runtimeTagValue->setText(tag.isEmpty() ? "Unavailable" : tag);
    m_runtimeLspPathValue->setText(lspPath.isEmpty() ? "Unavailable" : lspPath);
    m_runtimeStdlibPathValue->setText(stdlibPath.isEmpty() ? "Unavailable" : stdlibPath);
    m_runtimeCachePathValue->setText(cachePath.isEmpty() ? "Unavailable" : cachePath);
}

void SettingsPanel::setLspDiagnostics(const QString &connection,
                                      const QString &syncMode,
                                      const QString &lastError)
{
    if (m_lspConnectionValue)
        m_lspConnectionValue->setText(connection.isEmpty() ? "Unknown" : connection);
    if (m_lspSyncModeValue)
        m_lspSyncModeValue->setText(syncMode.isEmpty() ? "Unknown" : syncMode);
    if (m_lspLastErrorValue)
        m_lspLastErrorValue->setText(lastError.isEmpty() ? "None" : lastError);
}

void SettingsPanel::appendLspLog(const QString &line)
{
    if (m_lspLogView && !line.isEmpty())
        m_lspLogView->appendPlainText(line);
}

void SettingsPanel::clearLspLog()
{
    if (m_lspLogView)
        m_lspLogView->clear();
}
