#include "ContextWorkspaceController.h"

#include <utility>

ContextWorkspaceController::ContextWorkspaceController(
    ContextManager *contextManager, Callbacks callbacks, QObject *parent)
    : QObject(parent),
      m_callbacks(std::move(callbacks)),
      m_contextManager(contextManager)
{
    if (contextManager != nullptr) {
        connect(contextManager, &ContextManager::contextChanged, this,
                &ContextWorkspaceController::handleContextChanged);
    }
}

void ContextWorkspaceController::handleContextChanged(
    int index, const Context &context,
    ContextManager::ContextChangeReason reason)
{
    if (m_callbacks.applyWorkspaceRoot) {
        m_callbacks.applyWorkspaceRoot(context.rootPath);
    }
    if (m_callbacks.updateContextIndicator) {
        m_callbacks.updateContextIndicator(
            index, m_contextManager != nullptr ? m_contextManager->count() : 0);
    }

    const bool enabled =
        m_callbacks.lspEnabled && m_callbacks.lspEnabled();
    const bool running =
        m_callbacks.zithLspRunning && m_callbacks.zithLspRunning();
    if (enabled && running && m_callbacks.ensureLspRuntime) {
        m_callbacks.ensureLspRuntime(true);
    }

    if (reason != ContextManager::ContextChangeReason::RootChanged &&
        m_callbacks.restoreSession) {
        m_callbacks.restoreSession(context);
    }

    emit workspaceContextApplied();
}
