#include "ShortcutTreePresenter.h"

#include "ShortcutCatalog.h"
#include "TranslationManager.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>

void ShortcutTreePresenter::populate(QTreeWidget *tree)
{
    if (!tree)
        return;

    tree->clear();
    for (const ShortcutCategory &category : ShortcutCatalog::categories()) {
        auto *categoryItem = new QTreeWidgetItem;
        categoryItem->setData(0, Qt::UserRole, category.translationKey);
        categoryItem->setFont(0, QFont(QString(), -1, QFont::Bold));
        tree->addTopLevelItem(categoryItem);

        for (const ShortcutEntry &entry : category.entries) {
            auto *item = new QTreeWidgetItem(categoryItem);
            item->setData(0, Qt::UserRole, entry.translationKey);
            item->setData(1, Qt::UserRole, entry.keySequence);
        }

        categoryItem->setExpanded(true);
    }
}

void ShortcutTreePresenter::refreshTranslations(QTreeWidget *tree)
{
    if (!tree)
        return;

    auto &tr = TranslationManager::instance();
    tree->setHeaderLabels({tr.translate("shortcut.header_action"),
                           tr.translate("shortcut.header_key")});

    for (int categoryIndex = 0;
         categoryIndex < tree->topLevelItemCount(); ++categoryIndex) {
        QTreeWidgetItem *categoryItem = tree->topLevelItem(categoryIndex);
        categoryItem->setText(
            0, tr.translate(categoryItem->data(0, Qt::UserRole).toString()));

        for (int itemIndex = 0; itemIndex < categoryItem->childCount();
             ++itemIndex) {
            QTreeWidgetItem *item = categoryItem->child(itemIndex);
            item->setText(
                0, tr.translate(item->data(0, Qt::UserRole).toString()));
            item->setText(1, item->data(1, Qt::UserRole).toString());
        }
    }
}
