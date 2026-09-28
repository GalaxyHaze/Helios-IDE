#include "ShortcutsDialog.h"

#include "../core/ShortcutTreePresenter.h"
#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QTreeWidget>
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

ShortcutsDialog::ShortcutsDialog(QWidget *parent)
    : QDialog(parent)
{
    setModal(false);
    resize(520, 480);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
    layout->setSpacing(kSpacing);

    m_titleLabel = new QLabel(this);
    layout->addWidget(m_titleLabel);

    m_hintLabel = new QLabel(this);
    m_hintLabel->setWordWrap(true);
    layout->addWidget(m_hintLabel);

    m_shortcutsTree = new QTreeWidget(this);
    m_shortcutsTree->setColumnCount(2);
    m_shortcutsTree->setAlternatingRowColors(true);
    m_shortcutsTree->setSelectionMode(QAbstractItemView::NoSelection);
    m_shortcutsTree->setFocusPolicy(Qt::NoFocus);
    m_shortcutsTree->setAnimated(true);
    m_shortcutsTree->header()->setStretchLastSection(true);
    m_shortcutsTree->header()->setDefaultSectionSize(170);
    layout->addWidget(m_shortcutsTree, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);
    layout->addWidget(buttons);

    ShortcutTreePresenter::populate(m_shortcutsTree);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &ShortcutsDialog::applyTheme);
    connect(&TranslationManager::instance(), &TranslationManager::localeChanged,
            this, &ShortcutsDialog::applyTranslations);
    applyTheme();
    applyTranslations();
}

void ShortcutsDialog::applyTheme()
{
    auto &tm = ThemeManager::instance();
    QPalette pal = tm.palette();
    const QString baseHex = pal.color(QPalette::Base).name();
    const QString altBaseHex = pal.color(QPalette::AlternateBase).name();
    const QString textHex = pal.color(QPalette::Text).name();
    const QString windowTextHex = pal.color(QPalette::WindowText).name();
    const QString borderHex =
        tm.customColor("sidebarBorder", QColor("#363a4f")).name();

    setStyleSheetIfChanged(
        m_titleLabel,
        QString("color: %1; font-weight: bold; font-size: 14px;")
            .arg(windowTextHex));
    setStyleSheetIfChanged(
        m_hintLabel, QString("color: %1; font-size: 12px;").arg(textHex));
    setStyleSheetIfChanged(
        m_shortcutsTree,
        QString(
            "QTreeWidget { background: %1; color: %2; border: 1px solid %3; border-radius: 6px; }"
            "QTreeWidget::item { padding: 4px; color: %2; }"
            "QHeaderView::section { background: %4; color: %2; border: 1px solid %3; font-weight: bold; padding: 4px; }")
            .arg(altBaseHex, textHex, borderHex, baseHex));
}

void ShortcutsDialog::applyTranslations()
{
    auto &tr = TranslationManager::instance();
    setWindowTitle(tr.translate("shortcuts.window_title"));
    m_titleLabel->setText(tr.translate("shortcuts.title"));
    m_hintLabel->setText(tr.translate("shortcuts.hint"));
    ShortcutTreePresenter::refreshTranslations(m_shortcutsTree);
}
