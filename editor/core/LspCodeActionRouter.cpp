#include "LspCodeActionRouter.h"

#include <QCursor>
#include <memory>
#include <utility>

LspCodeActionRouter::LspCodeActionRouter(
    QWidget *menuParent, Callbacks callbacks, QObject *parent)
    : QObject(parent),
      m_menuParent(menuParent),
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
            [this, client](const QString &, int, const QJsonArray &actions) {
                handleCodeActions(client, actions);
            });
}

void LspCodeActionRouter::handleCodeActions(LspClient *client,
                                            const QJsonArray &actions)
{
    if (actions.isEmpty())
        return;

    std::unique_ptr<QMenu> menu(createMenu(client, actions));
    if (m_presentMenu)
        m_presentMenu(menu.get());
    else
        menu->exec(QCursor::pos());
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
