#include "WorkspaceNavigationController.h"

#include "../panels/FileTreePanel.h"
#include "../panels/GitPanel.h"
#include "../panels/WelcomeWidget.h"

#include <utility>

WorkspaceNavigationController::WorkspaceNavigationController(
    FileTreePanel *fileTree, WelcomeWidget *welcomeWidget, GitPanel *gitPanel,
    Callbacks callbacks, QObject *parent)
    : QObject(parent),
      m_callbacks(std::move(callbacks))
{
    if (fileTree) {
        connect(fileTree, &FileTreePanel::fileActivated, this,
                [this](const QString &path) {
                    if (m_callbacks.openFile)
                        m_callbacks.openFile(path);
                });
        connect(fileTree, &FileTreePanel::fileActivatedInNewTab, this,
                [this](const QString &path) {
                    if (m_callbacks.openFileInNewTab)
                        m_callbacks.openFileInNewTab(path);
                });
        connect(fileTree, &FileTreePanel::projectRootChanged, this,
                [this](const QString &path) {
                    if (m_callbacks.selectWorkspaceRoot)
                        m_callbacks.selectWorkspaceRoot(path);
                });
    }

    if (welcomeWidget) {
        connect(welcomeWidget, &WelcomeWidget::openFolderRequested, this,
                [this]() {
                    if (m_callbacks.openFolder)
                        m_callbacks.openFolder();
                });
        connect(welcomeWidget, &WelcomeWidget::newProjectRequested, this,
                [this]() {
                    if (m_callbacks.newProject)
                        m_callbacks.newProject();
                });
        connect(welcomeWidget, &WelcomeWidget::projectSelected, this,
                [this](const QString &path) {
                    if (m_callbacks.selectWorkspaceRoot)
                        m_callbacks.selectWorkspaceRoot(path);
                });
    }

    if (gitPanel) {
        connect(gitPanel, &GitPanel::fileActivated, this,
                [this](const QString &path) {
                    if (m_callbacks.openFile)
                        m_callbacks.openFile(path);
                });
    }
}

bool WorkspaceNavigationController::handleShellCommand(ShellCommand command)
{
    switch (command) {
    case ShellCommand::NewProject:
        if (m_callbacks.newProject)
            m_callbacks.newProject();
        return true;
    case ShellCommand::OpenFolder:
        if (m_callbacks.openFolder)
            m_callbacks.openFolder();
        return true;
    default:
        return false;
    }
}
