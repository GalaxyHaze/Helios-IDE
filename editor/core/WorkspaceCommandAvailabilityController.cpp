#include "WorkspaceCommandAvailabilityController.h"

#include "../editor/Code.h"
#include "../editor/LspClient.h"
#include "ShellCommandSurface.h"

#include <utility>

WorkspaceCommandAvailabilityController::WorkspaceCommandAvailabilityController(
    ShellCommandSurface *commandSurface, QObject *parent)
    : QObject(parent),
      m_commandSurface(commandSurface)
{
}

void WorkspaceCommandAvailabilityController::refresh(
    const WorkspaceCommandState &commandState)
{
    if (m_commandSurface)
        m_commandSurface->setWorkspaceActionState(buildState(commandState));
}

WorkspaceCommandAvailability
WorkspaceCommandAvailabilityController::buildState(
    const WorkspaceCommandState &commandState) const
{
    WorkspaceCommandAvailability state;
    const bool canExecute = commandState.canExecuteWorkspaceCommand;
    const bool activeZith = commandState.activeZithEditor;
    const bool hasWorkspaceRoot = commandState.hasWorkspaceRoot;
    const bool taskRunning = commandState.taskRunning;

    state.canBuild = canExecute && activeZith && hasWorkspaceRoot;
    state.canCheck =
        canExecute && activeZith && commandState.hasCurrentFile;
    state.canFormat = commandState.canFormat;
    state.canRun =
        canExecute && activeZith && hasWorkspaceRoot && !taskRunning;
    state.canStop = canExecute && taskRunning;

    state.buildTooltip =
        unavailableForEditor(canExecute, activeZith, hasWorkspaceRoot,
                             "Build project (Ctrl+B)");
    state.checkTooltip =
        state.canCheck
            ? QStringLiteral("Check current file (Ctrl+Shift+C)")
            : unavailableForEditor(canExecute, activeZith, true,
                                   "Check current file (Ctrl+Shift+C)");
    state.formatTooltip =
        commandState.canFormat
            ? QStringLiteral("Format document (Ctrl+Alt+L)")
            : QStringLiteral(
                  "Formatting is unavailable with the active LSP server");
    state.runTooltip =
        canExecute && activeZith && taskRunning
            ? QStringLiteral("A task is already running")
            : unavailableForEditor(canExecute, activeZith, hasWorkspaceRoot,
                                   "Run project (Ctrl+Shift+R)");
    state.stopTooltip = state.canStop
                            ? QStringLiteral("Stop running task (Ctrl+Shift+Q)")
                            : unavailableForStop(canExecute, taskRunning);
    return state;
}

QString WorkspaceCommandAvailabilityController::unavailableForEditor(
    bool canExecute, bool activeZith, bool hasWorkspaceRoot,
    const QString &base) const
{
    if (!canExecute) {
        return QStringLiteral(
            "Requires a zith-lsp server with workspace/executeCommand support");
    }
    if (!activeZith)
        return QStringLiteral("Not a Zith file");
    if (!hasWorkspaceRoot)
        return QStringLiteral("No active project root");
    return base;
}

QString WorkspaceCommandAvailabilityController::unavailableForStop(
    bool canExecute, bool taskRunning) const
{
    if (!canExecute) {
        return QStringLiteral(
            "Requires a zith-lsp server with workspace/executeCommand support");
    }
    if (!taskRunning)
        return QStringLiteral("No task is running");
    return QStringLiteral("Stop running task (Ctrl+Shift+Q)");
}
