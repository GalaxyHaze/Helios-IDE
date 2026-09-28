#ifndef CONTEXTWORKSPACECONTROLLER_H
#define CONTEXTWORKSPACECONTROLLER_H

#include "ContextManager.h"

#include <QObject>
#include <QString>

#include <functional>

class ContextWorkspaceController : public QObject
{
    Q_OBJECT

public:
    struct Callbacks
    {
        std::function<void(const QString &)> applyWorkspaceRoot;
        std::function<void(int, int)> updateContextIndicator;
        std::function<void(const Context &)> restoreSession;
        std::function<bool()> lspEnabled;
        std::function<bool()> zithLspRunning;
        std::function<void(bool)> ensureLspRuntime;
    };

    ContextWorkspaceController(ContextManager *contextManager,
                               Callbacks callbacks,
                               QObject *parent = nullptr);

signals:
    void workspaceContextApplied();

private:
    void handleContextChanged(
        int index, const Context &context,
        ContextManager::ContextChangeReason reason);

    Callbacks m_callbacks;
    ContextManager *m_contextManager = nullptr;
};

#endif
