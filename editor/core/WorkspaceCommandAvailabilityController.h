#ifndef WORKSPACECOMMANDAVAILABILITYCONTROLLER_H
#define WORKSPACECOMMANDAVAILABILITYCONTROLLER_H

#include "WorkspaceCommandAvailability.h"
#include "ShellCommand.h"
#include "WorkspaceCommandState.h"

#include <QObject>

class CodeEditor;
class ShellCommandSurface;

class WorkspaceCommandAvailabilityController : public QObject
{
    Q_OBJECT

public:
    explicit WorkspaceCommandAvailabilityController(
        ShellCommandSurface *commandSurface, QObject *parent = nullptr);

    void refresh(const WorkspaceCommandState &state);

private:
    WorkspaceCommandAvailability buildState(
        const WorkspaceCommandState &state) const;
    QString unavailableForEditor(bool canExecute, bool activeZith,
                                 bool hasWorkspaceRoot,
                                 const QString &base) const;
    QString unavailableForStop(bool canExecute, bool taskRunning) const;

    ShellCommandSurface *m_commandSurface = nullptr;
};

#endif
