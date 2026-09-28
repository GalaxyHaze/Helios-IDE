#ifndef SHORTCUTTREEPRESENTER_H
#define SHORTCUTTREEPRESENTER_H

class QTreeWidget;

class ShortcutTreePresenter
{
public:
    static void populate(QTreeWidget *tree);
    static void refreshTranslations(QTreeWidget *tree);
};

#endif
