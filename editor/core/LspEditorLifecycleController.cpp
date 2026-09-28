#include "LspEditorLifecycleController.h"

#include "../editor/Code.h"
#include "../editor/LspClient.h"
#include "LspDocumentCoordinator.h"

#include <QObject>
#include <QTabWidget>

LspEditorLifecycleController::LspEditorLifecycleController(
    QTabWidget *tabWidget, LspDocumentCoordinator *documentCoordinator,
    QObject *parent)
    : QObject(parent),
      m_tabWidget(tabWidget),
      m_documentCoordinator(documentCoordinator)
{
}

void LspEditorLifecycleController::openDocumentsFor(LspClient *client) const
{
    if (!m_tabWidget || !m_documentCoordinator || !client ||
        !client->isReady())
        return;

    for (int index = 0; index < m_tabWidget->count(); ++index) {
        auto *editor =
            qobject_cast<CodeEditor *>(m_tabWidget->widget(index));
        if (!editor || editor->lspClient() != client ||
            editor->filePath().isEmpty() || editor->fileUri().isEmpty()) {
            continue;
        }

        m_documentCoordinator->openDocument(editor);
    }
}

void LspEditorLifecycleController::detachDocumentsFor(
    LspClient *client) const
{
    if (!m_tabWidget || !m_documentCoordinator || !client)
        return;

    for (int index = 0; index < m_tabWidget->count(); ++index) {
        auto *editor =
            qobject_cast<CodeEditor *>(m_tabWidget->widget(index));
        if (editor && editor->lspClient() == client)
            m_documentCoordinator->close(editor);
    }
}
