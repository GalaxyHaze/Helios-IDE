#include "LspCodeActionRouter.h"

#include "../editor/Code.h"

#include <QCursor>
#include <QTabWidget>
#include <memory>
#include <utility>

LspCodeActionRouter::LspCodeActionRouter(
    QWidget *menuParent, QTabWidget *tabWidget, Callbacks callbacks,
    QObject *parent)
    : QObject(parent),
      m_menuParent(menuParent),
      m_tabWidget(tabWidget),
      m_applyWorkspaceEdit(std::move(callbacks.applyWorkspaceEdit)),
      m_executeCommand(std::move(callbacks.executeCommand)),
      m_presentMenu(std::move(callbacks.presentMenu))
{
}

void LspCodeActionRouter::attach(LspClient *client)
{
    if (!client)
        return;

    connect(client, &LspClient::codeActionsResult, this,
            [this, client](const QString &uri, int version,
                           const QJsonArray &actions) {
                handleCodeActions(client, uri, version, actions);
            });
}

void LspCodeActionRouter::handleCodeActions(LspClient *client,
                                            const QString &uri, int version,
                                            const QJsonArray &actions)
{
    if (actions.isEmpty() || !isCurrentDocument(client, uri, version))
        return;

    std::unique_ptr<QMenu> menu(createMenu(client, actions));
    if (m_presentMenu)
        m_presentMenu(menu.get());
    else
        menu->exec(QCursor::pos());
}

bool LspCodeActionRouter::isCurrentDocument(
    LspClient *client, const QString &uri, int version) const
{
    if (!m_tabWidget)
        return false;
    auto *editor = qobject_cast<CodeEditor *>(m_tabWidget->currentWidget());
    return editor && editor->lspClient() == client &&
           editor->fileUri() == uri && editor->documentVersion() == version;
}

QMenu *LspCodeActionRouter::createMenu(LspClient *client,
                                       const QJsonArray &actions)
{
    auto *menu = new QMenu(m_menuParent);
    for (const QJsonValue &value : actions) {
        const QJsonObject action = value.toObject();
        QAction *entry =
            menu->addAction(action.value("title").toString("Code action"));
        const QJsonObject command = action.value("command").toObject();
        if (action.contains("edit")) {
            const QJsonObject edit = action.value("edit").toObject();
            connect(entry, &QAction::triggered, this,
                    [this, edit]() {
                        if (m_applyWorkspaceEdit)
                            m_applyWorkspaceEdit(edit);
                    });
        } else if (!command.isEmpty() && client &&
                   client->supports(LspClient::Capability::ExecuteCommand)) {
            connect(entry, &QAction::triggered, this,
                    [this, client, command]() {
                        if (m_executeCommand)
                            m_executeCommand(client, command);
                    });
        } else {
            entry->setEnabled(false);
        }
    }
    return menu;
}
