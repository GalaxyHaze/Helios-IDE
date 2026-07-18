#include "ShortcutsDialog.h"

#include "../core/ThemeManager.h"
#include "../core/TranslationManager.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>
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

    initializeShortcutTree();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &ShortcutsDialog::applyTheme);
    connect(&TranslationManager::instance(), &TranslationManager::localeChanged,
            this, &ShortcutsDialog::applyTranslations);
    applyTheme();
    applyTranslations();
}

void ShortcutsDialog::initializeShortcutTree()
{
    const auto addCategory = [this](const QString &categoryKey,
                                    const QList<QPair<QString, QString>> &items) {
        auto *categoryItem = new QTreeWidgetItem;
        categoryItem->setData(0, Qt::UserRole, categoryKey);
        categoryItem->setFont(0, QFont(QString(), -1, QFont::Bold));
        m_shortcutsTree->addTopLevelItem(categoryItem);

        for (const auto &item : items) {
            auto *child = new QTreeWidgetItem(categoryItem);
            child->setData(0, Qt::UserRole, item.first);
            child->setData(1, Qt::UserRole, item.second);
        }

        categoryItem->setExpanded(true);
    };

    addCategory("shortcut.cat.file", {
        { "shortcut.new_file", "Ctrl+N" },
        { "shortcut.new_window", "Ctrl+Shift+N" },
        { "shortcut.new_project", "Ctrl+Alt+N" },
        { "shortcut.open", "Ctrl+O" },
        { "shortcut.open_folder", "Ctrl+Alt+O" },
        { "shortcut.save", "Ctrl+S" },
        { "shortcut.exit", "Ctrl+Q" }
    });

    addCategory("shortcut.cat.edit_search", {
        { "shortcut.find", "Ctrl+F" },
        { "shortcut.replace", "Ctrl+H" },
        { "shortcut.find_next", "F3 / Ctrl+G" },
        { "shortcut.find_prev", "Shift+F3 / Ctrl+Shift+G" }
    });

    addCategory("shortcut.cat.navigation_view", {
        { "shortcut.explorer", "Ctrl+Shift+E" },
        { "shortcut.global_search", "Ctrl+Shift+F" },
        { "shortcut.git_panel", "Ctrl+Shift+G" },
        { "shortcut.settings", "Ctrl+," },
        { "shortcut.hide_sidebar", "Ctrl+Shift+X" },
        { "shortcut.go_back", "Alt+Left" },
        { "shortcut.go_forward", "Alt+Right" }
    });

    addCategory("shortcut.cat.lsp_tools", {
        { "shortcut.go_definition", "F12 / Ctrl+Click" },
        { "shortcut.go_implementation", "Ctrl+F12" },
        { "shortcut.format_doc", "Ctrl+Alt+L / Alt+Shift+F" },
        { "shortcut.completion", "Ctrl+Space" },
        { "shortcut.build", "Ctrl+B" },
        { "shortcut.check", "Ctrl+Shift+C" },
        { "shortcut.compile", "Ctrl+Shift+B" },
        { "shortcut.run", "Ctrl+Shift+R" },
        { "shortcut.restart_lsp", "Ctrl+Shift+L" },
        { "shortcut.getting_started", "F1" }
    });
}

void ShortcutsDialog::updateShortcutTexts()
{
    auto &tr = TranslationManager::instance();

    m_shortcutsTree->setHeaderLabels({tr.translate("shortcut.header_action"),
                                      tr.translate("shortcut.header_key")});
    for (int categoryIndex = 0;
         categoryIndex < m_shortcutsTree->topLevelItemCount();
         ++categoryIndex) {
        QTreeWidgetItem *categoryItem =
            m_shortcutsTree->topLevelItem(categoryIndex);
        categoryItem->setText(0,
                              tr.translate(categoryItem->data(0, Qt::UserRole)
                                               .toString()));

        for (int itemIndex = 0; itemIndex < categoryItem->childCount();
             ++itemIndex) {
            QTreeWidgetItem *item = categoryItem->child(itemIndex);
            item->setText(0,
                          tr.translate(item->data(0, Qt::UserRole).toString()));
            item->setText(1, item->data(1, Qt::UserRole).toString());
        }
    }
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
    updateShortcutTexts();
}
