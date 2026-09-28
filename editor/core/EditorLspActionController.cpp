#include "EditorLspActionController.h"

#include "../editor/Code.h"

#include <QTabWidget>

EditorLspActionController::EditorLspActionController(QTabWidget *tabWidget,
                                                     QObject *parent)
    : QObject(parent), m_tabWidget(tabWidget)
{
}

void EditorLspActionController::attach(CodeEditor *editor)
{
    if (!editor)
        return;

    connect(editor, &CodeEditor::renameRequested, this,
            [this, editor](const QString &uri, int version,
                           const LspPosition &position,
                           const QString &newName) {
                requestRename(editor, uri, version, position, newName);
            });
    connect(editor, &CodeEditor::codeActionsRequested, this,
            [this, editor](const QString &uri, int version,
                           const LspRange &range) {
                requestCodeActions(editor, uri, version, range);
            });
}

bool EditorLspActionController::requestRename(
    CodeEditor *editor, const QString &uri, int version,
    const LspPosition &position, const QString &newName)
{
    flushPendingChanges();
    if (!editor || !editor->lspClient() || !editor->lspClient()->isReady())
        return false;

    editor->lspClient()->requestRename(uri, version, position, newName);
    return true;
}

bool EditorLspActionController::requestCodeActions(
    CodeEditor *editor, const QString &uri, int version, const LspRange &range)
{
    if (!editor || !editor->lspClient() || !editor->lspClient()->isReady())
        return false;

    editor->lspClient()->requestCodeActions(uri, version, range,
                                            editor->diagnostics());
    return true;
}

bool EditorLspActionController::handleShellCommand(ShellCommand command)
{
    if (command != ShellCommand::FormatDocument)
        return false;

    CodeEditor *editor = currentEditor();
    if (!editor || !editor->lspClient() || !editor->lspClient()->isReady())
        return true;

    editor->flushPendingLspChanges();
    editor->lspClient()->requestFormatting(editor->fileUri(),
                                            editor->documentVersion());
    return true;
}

CodeEditor *EditorLspActionController::currentEditor() const
{
    return m_tabWidget
               ? qobject_cast<CodeEditor *>(m_tabWidget->currentWidget())
               : nullptr;
}

void EditorLspActionController::flushPendingChanges() const
{
    if (!m_tabWidget)
        return;

    for (int index = 0; index < m_tabWidget->count(); ++index) {
        if (auto *editor =
                qobject_cast<CodeEditor *>(m_tabWidget->widget(index))) {
            editor->flushPendingLspChanges();
        }
    }
}
