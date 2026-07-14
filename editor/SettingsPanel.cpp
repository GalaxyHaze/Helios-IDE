#include "SettingsPanel.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
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
    layout->addStretch();

    setStyleSheet(
        "SettingsPanel { background: #11111b; color: #c6d0f5; }"
        "QSpinBox { background: #1e1e2e; color: #c6d0f5; border: 1px solid #363a4f; border-radius: 4px; padding: 4px 6px; }"
        "QCheckBox { color: #c6d0f5; }"
        "QLabel { color: #c6d0f5; }"
    );

    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsPanel::fontSizeChanged);
    connect(m_wordWrapCheck, &QCheckBox::toggled,
            this, &SettingsPanel::wordWrapChanged);
}

void SettingsPanel::setFontSize(int pointSize)
{
    m_fontSizeSpin->setValue(pointSize);
}

void SettingsPanel::setWordWrapEnabled(bool enabled)
{
    m_wordWrapCheck->setChecked(enabled);
}
