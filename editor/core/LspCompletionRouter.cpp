#include "LspCompletionRouter.h"

#include "SnippetManager.h"

#include "../editor/Code.h"
#include "../editor/LspCompletionModel.h"

#include <QTabWidget>

LspCompletionRouter::LspCompletionRouter(
    Dependencies dependencies, std::function<bool()> isEnabled,
    QObject *parent)
    : QObject(parent),
      m_tabWidget(dependencies.tabWidget),
      m_snippetManager(dependencies.snippetManager),
      m_completer(dependencies.completer),
      m_completionModel(dependencies.completionModel),
      m_isEnabled(std::move(isEnabled))
{
}

void LspCompletionRouter::attach(LspClient *client)
{
    if (!client)
        return;

    connect(client, &LspClient::completionResults, this,
            [this](const QString &uri, int,
                   const QList<LspCompletionItem> &items) {
                handleCompletion(uri, items);
            });
}

void LspCompletionRouter::handleCompletion(
    const QString &uri, const QList<LspCompletionItem> &items)
{
    if (m_isEnabled && !m_isEnabled())
        return;
    if (!m_tabWidget || !m_completionModel || !m_completer)
        return;

    auto *editor = qobject_cast<CodeEditor *>(m_tabWidget->currentWidget());
    if (!editor || editor->fileUri() != uri)
        return;

    QList<LspCompletionItem> all = items;
    if (m_snippetManager)
        all.append(m_snippetManager->allSnippets());
    m_completionModel->setItems(all);
    if (!all.isEmpty()) {
        m_completer->setWidget(editor);
        m_completer->complete();
    }
}
