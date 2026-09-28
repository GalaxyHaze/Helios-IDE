#include "WorkspaceCommandController.h"

#include "../editor/Code.h"
#include "../editor/LspClient.h"
#include "../panels/CompilerPanel.h"
#include "LspEventSource.h"
#include "WorkspaceTaskOutputController.h"

#include <QUrl>

namespace {
QString failurePrefixFor(const QString &command)
{
    if (command == QLatin1String("zith.build"))
        return QStringLiteral("Build failed: ");
    if (command == QLatin1String("zith.check"))
        return QStringLiteral("Check failed: ");
    if (command == QLatin1String("zith.run"))
        return QStringLiteral("Run failed: ");
    return {};
}
} // namespace

WorkspaceCommandController::WorkspaceCommandController(
    Dependencies dependencies, Callbacks callbacks, QObject *parent)
    : QObject(parent),
      m_client(dependencies.client),
      m_compilerPanel(dependencies.compilerPanel),
      m_callbacks(std::move(callbacks)),
      m_taskOutput(std::make_unique<WorkspaceTaskOutputController>(
          dependencies.events, dependencies.compilerPanel,
          [this]() { notifyStateChanged(); }, this))
{
    if (!m_client)
        return;

    if (dependencies.events) {
        connect(dependencies.events, &LspEventSource::commandResult, this,
                &WorkspaceCommandController::handleCommandResult);
        connect(dependencies.events, &LspEventSource::saveAllRequested, this,
                [this]() {
                    if (m_callbacks.saveAll)
                        m_callbacks.saveAll();
                });
    }
}

WorkspaceCommandController::~WorkspaceCommandController() = default;

bool WorkspaceCommandController::canExecuteWorkspaceCommand() const
{
    return state().canExecuteWorkspaceCommand;
}

bool WorkspaceCommandController::canExecute(ShellCommand command) const
{
    const WorkspaceCommandState commandState = state();
    if (!commandState.canExecuteWorkspaceCommand)
        return false;

    switch (command) {
    case ShellCommand::Build:
    case ShellCommand::Run:
        return commandState.activeZithEditor &&
               commandState.hasWorkspaceRoot &&
               (command != ShellCommand::Run || !commandState.taskRunning);
    case ShellCommand::CheckFile:
        return commandState.activeZithEditor &&
               commandState.hasCurrentFile;
    case ShellCommand::Stop:
        return commandState.taskRunning;
    default:
        return false;
    }
}

WorkspaceCommandState WorkspaceCommandController::state() const
{
    WorkspaceCommandState commandState;
    commandState.canExecuteWorkspaceCommand =
        m_callbacks.lspEnabled && m_callbacks.lspEnabled() && m_client &&
        m_client->isReady() &&
        m_client->supports(LspClient::Capability::ExecuteCommand);

    CodeEditor *editor =
        m_callbacks.currentEditor ? m_callbacks.currentEditor() : nullptr;
    commandState.activeZithEditor =
        editor && m_callbacks.isZithEditor &&
        m_callbacks.isZithEditor(editor);
    commandState.hasCurrentFile = editor && !editor->filePath().isEmpty();
    commandState.workspaceRoot =
        m_callbacks.workspaceRoot ? m_callbacks.workspaceRoot() : QString();
    commandState.hasWorkspaceRoot =
        !commandState.workspaceRoot.trimmed().isEmpty();
    commandState.runningTaskId =
        m_taskOutput ? m_taskOutput->runningTaskId() : QString();
    commandState.taskRunning = !commandState.runningTaskId.isEmpty();
    commandState.canFormat =
        editor && !editor->fileUri().isEmpty() && editor->lspClient() &&
        editor->lspClient()->isReady() &&
        editor->lspClient()->supports(LspClient::Capability::Formatting);
    return commandState;
}

void WorkspaceCommandController::showUnavailable(
    const UnavailableCommand &command)
{
    if (m_callbacks.showCompiler)
        m_callbacks.showCompiler();
    if (m_compilerPanel) {
        m_compilerPanel->startBuild(command.title);
        m_compilerPanel->appendOutput(command.message);
    }
    if (m_callbacks.showStatus)
        m_callbacks.showStatus(command.message, 6000);
}

void WorkspaceCommandController::execute(const QString &command,
                                         const QJsonArray &arguments,
                                         const QString &title)
{
    if (m_callbacks.showCompiler)
        m_callbacks.showCompiler();
    if (m_compilerPanel)
        m_compilerPanel->startBuild(title);
    if (m_taskOutput)
        m_taskOutput->beginCommand();
    m_client->executeWorkspaceCommand(command, arguments, {});
    notifyStateChanged();
}

void WorkspaceCommandController::runBuild()
{
    const WorkspaceCommandState commandState = state();
    if (!commandState.activeZithEditor) {
        showUnavailable({QStringLiteral("Build unavailable"),
                         QStringLiteral("Open an active Zith file to build "
                                        "the project.")});
        return;
    }
    if (!commandState.canExecuteWorkspaceCommand) {
        showUnavailable({QStringLiteral("Build unavailable"),
                         QStringLiteral("zith-lsp must advertise "
                      "workspace/executeCommand to run Build.")});
        return;
    }

    if (!commandState.hasWorkspaceRoot) {
        if (m_callbacks.showStatus)
            m_callbacks.showStatus(
                QStringLiteral("No active project root for Build"), 5000);
        return;
    }

    if (m_callbacks.saveAll)
        m_callbacks.saveAll();
    execute(QStringLiteral("zith.build"),
            QJsonArray{QUrl::fromLocalFile(commandState.workspaceRoot).toString()},
            QStringLiteral("Build project %1").arg(commandState.workspaceRoot));
}

bool WorkspaceCommandController::handleShellCommand(ShellCommand command)
{
    switch (command) {
    case ShellCommand::Build:
        runBuild();
        return true;
    case ShellCommand::CheckFile:
        runCheckFile();
        return true;
    case ShellCommand::Run:
        runProject();
        return true;
    case ShellCommand::Stop:
        stopRunningTask();
        return true;
    default:
        return false;
    }
}

void WorkspaceCommandController::runCheckFile()
{
    const WorkspaceCommandState commandState = state();
    auto *editor = m_callbacks.currentEditor ? m_callbacks.currentEditor()
                                              : nullptr;
    if (!commandState.hasCurrentFile || !editor) {
        if (m_callbacks.showStatus)
            m_callbacks.showStatus(QStringLiteral("No open file to check"), 4000);
        return;
    }
    if (!commandState.activeZithEditor) {
        showUnavailable({QStringLiteral("Check unavailable"),
                         QStringLiteral("Check File requires an active Zith "
                                        "file.")});
        return;
    }
    if (!commandState.canExecuteWorkspaceCommand) {
        showUnavailable({QStringLiteral("Check unavailable"),
                         QStringLiteral("zith-lsp must advertise "
                                        "workspace/executeCommand to run "
                                        "Check File.")});
        return;
    }

    editor->flushPendingLspChanges();
    execute(QStringLiteral("zith.check"), QJsonArray{editor->fileUri()},
            QStringLiteral("Check %1").arg(editor->filePath()));
}

void WorkspaceCommandController::runProject()
{
    const WorkspaceCommandState commandState = state();
    if (!commandState.activeZithEditor) {
        showUnavailable({QStringLiteral("Run unavailable"),
                         QStringLiteral("Open an active Zith file to run "
                                        "the project.")});
        return;
    }
    if (!commandState.canExecuteWorkspaceCommand) {
        showUnavailable({QStringLiteral("Run unavailable"),
                         QStringLiteral("zith-lsp must advertise "
                                        "workspace/executeCommand to run the "
                                        "project.")});
        return;
    }

    if (!commandState.hasWorkspaceRoot) {
        if (m_callbacks.showStatus)
            m_callbacks.showStatus(
                QStringLiteral("No active project root for Run"), 5000);
        return;
    }

    if (m_callbacks.saveAll)
        m_callbacks.saveAll();
    execute(QStringLiteral("zith.run"),
            QJsonArray{QUrl::fromLocalFile(commandState.workspaceRoot).toString()},
            QStringLiteral("Run project %1").arg(commandState.workspaceRoot));
}

void WorkspaceCommandController::stopRunningTask()
{
    const WorkspaceCommandState commandState = state();
    if (!commandState.taskRunning ||
        !commandState.canExecuteWorkspaceCommand)
        return;

    m_client->executeWorkspaceCommand(QStringLiteral("zith.stop"),
                                      QJsonArray{commandState.runningTaskId},
                                      {});
    if (m_compilerPanel)
        m_compilerPanel->appendOutput(QStringLiteral("Stopping task %1")
                                          .arg(commandState.runningTaskId));
}

void WorkspaceCommandController::appendServerFailure(
    const QString &command, const WorkspaceCommandResult &result)
{
    if (!m_compilerPanel)
        return;

    const QString prefix = failurePrefixFor(command);
    if (!result.text.isEmpty()) {
        m_compilerPanel->appendOutput(prefix + result.text);
        return;
    }
    if (!result.hasServerDetails)
        return;

    m_compilerPanel->appendOutput(
        QStringLiteral("Server reported success=false"));
    if (!result.programUri.isEmpty())
        m_compilerPanel->appendOutput(
            QStringLiteral("Program URI: %1").arg(result.programUri));
    if (result.hasCodegenAvailable)
        m_compilerPanel->appendOutput(
            QStringLiteral("codegenAvailable: %1")
                .arg(result.codegenAvailable ? QStringLiteral("true")
                                             : QStringLiteral("false")));
    if (!result.serverMessage.isEmpty())
        m_compilerPanel->appendOutput(result.serverMessage);
}

void WorkspaceCommandController::handleCommandResult(const QString &command,
                                                      bool success,
                                                      const QJsonValue &result)
{
    if (!m_compilerPanel)
        return;

    const WorkspaceCommandResult decoded =
        WorkspaceCommandResultDecoder::decode(success, result);

    if (command == QLatin1String("zith.build")) {
        m_compilerPanel->showBuildResult(decoded.success, decoded.programUri);
        if (!decoded.success) {
            appendServerFailure(command, decoded);
            if (m_callbacks.appendPublishedDiagnostics)
                m_callbacks.appendPublishedDiagnostics();
        }
    } else if (command == QLatin1String("zith.check")) {
        m_compilerPanel->showBuildResult(decoded.success);
        if (!decoded.success) {
            appendServerFailure(command, decoded);
            if (m_callbacks.appendPublishedDiagnostics)
                m_callbacks.appendPublishedDiagnostics();
        }
        if (auto *editor = m_callbacks.currentEditor ? m_callbacks.currentEditor()
                                                     : nullptr)
            editor->viewport()->update();
    } else if (command == QLatin1String("zith.run")) {
        if (!decoded.success) {
            m_compilerPanel->showBuildResult(false);
            appendServerFailure(command, decoded);
            if (m_taskOutput)
                m_taskOutput->clearRunningTask();
            notifyStateChanged();
            return;
        }

        const QString taskId = decoded.taskId;
        if (taskId.isEmpty()) {
            m_compilerPanel->appendOutput(
                QStringLiteral("Server started run without a taskId; cannot "
                               "track or stop it."));
            m_compilerPanel->showBuildResult(false);
            if (m_taskOutput)
                m_taskOutput->clearRunningTask();
            notifyStateChanged();
            return;
        }

        if (m_taskOutput)
            m_taskOutput->trackTaskStarted(taskId, decoded.programUri);
    }

    notifyStateChanged();
}

void WorkspaceCommandController::notifyStateChanged()
{
    emit stateChanged(state());
}
