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
            [this, client](const QString &uri, int version,
                   const QList<LspCompletionItem> &items) {
                handleCompletion(client, uri, version, items);
            });
    connect(client, &LspClient::completionResolved, this,
            [this, client](const QString &uri, int version,
                   const LspCompletionItem &item) {
                if (!m_tabWidget || !m_completionModel)
                    return;
                auto *editor =
                    qobject_cast<CodeEditor *>(m_tabWidget->currentWidget());
                if (!editor || editor->lspClient() != client ||
                    editor->fileUri() != uri ||
                    editor->documentVersion() != version) {
                    return;
                }
                m_completionModel->updateItem(item);
            });
    if (m_completer) {
        connect(m_completer,
                QOverload<const QModelIndex &>::of(
                    &QCompleter::highlighted),
                this, [this, client](const QModelIndex &) {
                    if (!m_tabWidget || !m_completionModel ||
                        !m_completer)
                        return;
                    auto *editor = qobject_cast<CodeEditor *>(
                        m_tabWidget->currentWidget());
                    if (!editor || editor->lspClient() != client ||
                        !client->isReady()) {
                        return;
                    }
                    const LspCompletionItem item =
                        m_completer->currentItem();
                    if (!item.rawItem.contains(QStringLiteral("data")))
                        return;
                    client->resolveCompletion(editor->fileUri(),
                                              editor->documentVersion(),
                                              item.rawItem);
                });
    }
}

void LspCompletionRouter::handleCompletion(
    LspClient *client, const QString &uri, int version,
    const QList<LspCompletionItem> &items)
{
    if (m_isEnabled && !m_isEnabled())
        return;
    if (!m_tabWidget || !m_completionModel || !m_completer)
        return;

    auto *editor = qobject_cast<CodeEditor *>(m_tabWidget->currentWidget());
    if (!editor || editor->lspClient() != client ||
        editor->fileUri() != uri || editor->documentVersion() != version)
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
