#include "WorkspaceRootInteractionController.h"

#include <QDir>
#include <QFileDialog>
#include <QWidget>

#include <utility>

WorkspaceRootInteractionController::WorkspaceRootInteractionController(
    QWidget *dialogParent, WorkspaceRootController *rootController,
    RequestDirectory requestDirectory, CurrentRoot currentRoot,
    QObject *parent)
    : QObject(parent),
      m_dialogParent(dialogParent),
      m_rootController(rootController),
      m_requestDirectory(std::move(requestDirectory)),
      m_currentRoot(std::move(currentRoot))
{
}

WorkspaceRootController::ActivationResult
WorkspaceRootInteractionController::openFolder()
{
    return activate(Intent::ReplaceCurrentRoot);
}

WorkspaceRootController::ActivationResult
WorkspaceRootInteractionController::newProject()
{
    return activate(Intent::CreateContext);
}

WorkspaceRootController::ActivationResult
WorkspaceRootInteractionController::createContext()
{
    return activate(Intent::CreateContext);
}

WorkspaceRootController::ActivationResult
WorkspaceRootInteractionController::activate(Intent intent)
{
    if (m_rootController == nullptr) {
        return WorkspaceRootController::ActivationResult::Rejected;
    }

    const QString title =
        intent == Intent::ReplaceCurrentRoot
            ? QStringLiteral("Open Folder")
            : QStringLiteral("New project folder");
    const QString initialPath =
        intent == Intent::ReplaceCurrentRoot && m_currentRoot
            ? m_currentRoot()
            : QString();
    const QString path = requestDirectory(title, initialPath);
    if (path.isEmpty()) {
        return WorkspaceRootController::ActivationResult::Rejected;
    }

    return intent == Intent::ReplaceCurrentRoot
               ? m_rootController->replaceCurrentRoot(path)
               : m_rootController->createContext(path);
}

QString WorkspaceRootInteractionController::requestDirectory(
    const QString &title, const QString &initialPath) const
{
    if (m_requestDirectory) {
        return m_requestDirectory(title, initialPath);
    }

    return QFileDialog::getExistingDirectory(
        m_dialogParent, title,
        initialPath.isEmpty() ? QDir::homePath() : initialPath,
        QFileDialog::ShowDirsOnly);
}
