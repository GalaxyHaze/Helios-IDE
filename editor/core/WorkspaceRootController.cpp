#include "WorkspaceRootController.h"

#include "ContextManager.h"

#include <QFileInfo>

#include <utility>

WorkspaceRootController::WorkspaceRootController(
    ContextManager *contextManager, Callbacks callbacks, QObject *parent)
    : QObject(parent),
      m_contextManager(contextManager),
      m_callbacks(std::move(callbacks))
{
}

WorkspaceRootController::ActivationResult
WorkspaceRootController::replaceCurrentRoot(const QString &path)
{
    return activate(path, false, true);
}

WorkspaceRootController::ActivationResult
WorkspaceRootController::createContext(const QString &path)
{
    return activate(path, true, true);
}

WorkspaceRootController::ActivationResult
WorkspaceRootController::restoreCurrentRoot(const QString &path)
{
    return activate(path, false, false);
}

QString WorkspaceRootController::normalizeDirectory(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists() || !info.isDir()) {
        return {};
    }

    return info.absoluteFilePath();
}

WorkspaceRootController::ActivationResult
WorkspaceRootController::activate(const QString &path, bool createContext,
                                  bool persistActivation)
{
    if (m_contextManager == nullptr) {
        return ActivationResult::Rejected;
    }

    const QString normalized = normalizeDirectory(path);
    if (normalized.isEmpty()) {
        return ActivationResult::Rejected;
    }

    if (persistActivation && m_callbacks.saveCurrentContext) {
        m_callbacks.saveCurrentContext();
    }

    if (createContext) {
        m_contextManager->appendNew(normalized);
    } else {
        m_contextManager->setCurrentRoot(normalized);
    }

    if (persistActivation && m_callbacks.persistRecentProject) {
        m_callbacks.persistRecentProject(normalized);
    }
    if (persistActivation && m_callbacks.refreshRecentProjects) {
        m_callbacks.refreshRecentProjects();
    }

    if (createContext) {
        return ActivationResult::CreatedContext;
    }
    return persistActivation ? ActivationResult::ReplacedCurrentContext
                             : ActivationResult::RestoredCurrentContext;
}
