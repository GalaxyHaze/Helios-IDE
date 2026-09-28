#ifndef SHELLDIALOGCONTROLLER_H
#define SHELLDIALOGCONTROLLER_H

#include <QObject>

#include <functional>

#include "ShellCommand.h"

class LspManagerDialog;
class QDialog;
class QWidget;

class ShellDialogController : public QObject
{
    Q_OBJECT

public:
    using RefreshLspManager = std::function<void()>;

    ShellDialogController(QWidget *dialogParent,
                          LspManagerDialog *lspManagerDialog,
                          RefreshLspManager refreshLspManager,
                          QObject *parent = nullptr);

    LspManagerDialog *lspManagerDialog() const;

    bool handleShellCommand(ShellCommand command);

    void togglePreferences();
    void toggleShortcuts();
    void toggleLspManager();
    void toggleVimHelp();
    void showGettingStarted();

private:
    void toggle(QDialog *dialog);
    QDialog *createPreferences();
    QDialog *createShortcuts();
    QDialog *createVimHelp();

    QWidget *m_dialogParent = nullptr;
    LspManagerDialog *m_lspManagerDialog = nullptr;
    QDialog *m_preferencesDialog = nullptr;
    QDialog *m_shortcutsDialog = nullptr;
    QDialog *m_vimHelpDialog = nullptr;
    RefreshLspManager m_refreshLspManager;
};

#endif
