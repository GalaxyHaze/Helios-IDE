#ifndef WORKSPACECOMMANDSTATE_H
#define WORKSPACECOMMANDSTATE_H

#include <QString>

struct WorkspaceCommandState
{
    bool canExecuteWorkspaceCommand = false;
    bool activeZithEditor = false;
    bool hasCurrentFile = false;
    bool hasWorkspaceRoot = false;
    bool taskRunning = false;
    bool canFormat = false;
    QString workspaceRoot;
    QString runningTaskId;
};

#endif
