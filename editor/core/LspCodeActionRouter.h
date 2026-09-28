#ifndef LSPCODEACTIONROUTER_H
#define LSPCODEACTIONROUTER_H

#include "../editor/LspClient.h"

#include <QJsonObject>
#include <QMenu>
#include <QObject>
#include <functional>

class LspCodeActionRouter : public QObject
{
public:
    using ApplyWorkspaceEdit = std::function<void(const QJsonObject &)>;
    using ExecuteWorkspaceCommand =
        std::function<void(LspClient *, const QJsonObject &)>;
    using PresentMenu = std::function<void(QMenu *)>;

    struct Callbacks
    {
        ApplyWorkspaceEdit applyWorkspaceEdit;
        ExecuteWorkspaceCommand executeCommand;
        PresentMenu presentMenu;
    };

    explicit LspCodeActionRouter(QWidget *menuParent,
                                 Callbacks callbacks,
                                 QObject *parent = nullptr);

    void attach(LspClient *client);

private:
    void handleCodeActions(LspClient *client, const QJsonArray &actions);
    QMenu *createMenu(LspClient *client, const QJsonArray &actions);

    QWidget *m_menuParent;
    ApplyWorkspaceEdit m_applyWorkspaceEdit;
    ExecuteWorkspaceCommand m_executeCommand;
    PresentMenu m_presentMenu;
};

#endif
