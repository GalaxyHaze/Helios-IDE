#ifndef SHORTCUTSDIALOG_H
#define SHORTCUTSDIALOG_H

#include <QDialog>

class QLabel;
class QTreeWidget;

class ShortcutsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShortcutsDialog(QWidget *parent = nullptr);

private slots:
    void applyTheme();
    void applyTranslations();

private:
    void initializeShortcutTree();
    void updateShortcutTexts();

    QLabel *m_titleLabel = nullptr;
    QLabel *m_hintLabel = nullptr;
    QTreeWidget *m_shortcutsTree = nullptr;
};

#endif
