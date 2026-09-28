#ifndef WORKSPACENAVIGATIONCONTROLLER_H
#define WORKSPACENAVIGATIONCONTROLLER_H

#include <QObject>
#include <QString>

#include <functional>

#include "ShellCommand.h"

class FileTreePanel;
class GitPanel;
class WelcomeWidget;

class WorkspaceNavigationController : public QObject
{
    Q_OBJECT

public:
    struct Callbacks
    {
        std::function<void(const QString &)> openFile;
        std::function<void(const QString &)> openFileInNewTab;
        std::function<void(const QString &)> selectWorkspaceRoot;
        std::function<void()> openFolder;
        std::function<void()> newProject;
    };

    WorkspaceNavigationController(FileTreePanel *fileTree,
                                  WelcomeWidget *welcomeWidget,
                                  GitPanel *gitPanel,
                                  Callbacks callbacks,
                                  QObject *parent = nullptr);

    bool handleShellCommand(ShellCommand command);

private:
    Callbacks m_callbacks;
};

#endif
