#include "ShellDialogController.h"

#include "../panels/GettingStartedDialog.h"
#include "../panels/LspManagerDialog.h"
#include "../panels/PreferencesDialog.h"
#include "../panels/ShortcutsDialog.h"
#include "../panels/VimHelpDialog.h"

#include <QDialog>

#include <utility>

ShellDialogController::ShellDialogController(
    QWidget *dialogParent, LspManagerDialog *lspManagerDialog,
    RefreshLspManager refreshLspManager, QObject *parent)
    : QObject(parent),
      m_dialogParent(dialogParent),
      m_lspManagerDialog(lspManagerDialog),
      m_refreshLspManager(std::move(refreshLspManager))
{
}

LspManagerDialog *ShellDialogController::lspManagerDialog() const
{
    return m_lspManagerDialog;
}

bool ShellDialogController::handleShellCommand(ShellCommand command)
{
    switch (command) {
    case ShellCommand::Preferences:
        togglePreferences();
        return true;
    case ShellCommand::VimHelp:
        toggleVimHelp();
        return true;
    case ShellCommand::Shortcuts:
        toggleShortcuts();
        return true;
    case ShellCommand::LspManager:
        toggleLspManager();
        return true;
    case ShellCommand::GettingStarted:
        showGettingStarted();
        return true;
    default:
        return false;
    }
}

void ShellDialogController::toggle(QDialog *dialog)
{
    if (!dialog)
        return;

    if (dialog->isVisible()) {
        dialog->close();
        return;
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

QDialog *ShellDialogController::createPreferences()
{
    if (!m_preferencesDialog)
        m_preferencesDialog = new PreferencesDialog(m_dialogParent);
    return m_preferencesDialog;
}

QDialog *ShellDialogController::createShortcuts()
{
    if (!m_shortcutsDialog)
        m_shortcutsDialog = new ShortcutsDialog(m_dialogParent);
    return m_shortcutsDialog;
}

QDialog *ShellDialogController::createVimHelp()
{
    if (!m_vimHelpDialog)
        m_vimHelpDialog = new VimHelpDialog(m_dialogParent);
    return m_vimHelpDialog;
}

void ShellDialogController::togglePreferences()
{
    toggle(createPreferences());
}

void ShellDialogController::toggleShortcuts()
{
    toggle(createShortcuts());
}

void ShellDialogController::toggleLspManager()
{
    if (m_lspManagerDialog && !m_lspManagerDialog->isVisible() &&
        m_refreshLspManager)
        m_refreshLspManager();
    toggle(m_lspManagerDialog);
}

void ShellDialogController::toggleVimHelp()
{
    toggle(createVimHelp());
}

void ShellDialogController::showGettingStarted()
{
    GettingStartedDialog dialog(m_dialogParent);
    dialog.exec();
}
