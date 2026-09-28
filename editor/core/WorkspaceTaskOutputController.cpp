#include "WorkspaceTaskOutputController.h"

#include "../panels/CompilerPanel.h"
#include "LspEventSource.h"

#include <utility>

WorkspaceTaskOutputController::WorkspaceTaskOutputController(
    LspEventSource *events, CompilerPanel *compilerPanel,
    StateChanged stateChanged, QObject *parent)
    : QObject(parent),
      m_compilerPanel(compilerPanel),
      m_stateChanged(std::move(stateChanged)),
      m_runOutput(std::make_unique<RunOutputCollector>())
{
    if (!events)
        return;

    connect(events, &LspEventSource::processOutputReceived, this,
            &WorkspaceTaskOutputController::handleProcessOutput);
    connect(events, &LspEventSource::processExitReceived, this,
            &WorkspaceTaskOutputController::handleProcessExit);
    connect(events, &LspEventSource::workDoneProgressReceived, this,
            &WorkspaceTaskOutputController::handleWorkDoneProgress);
    connect(events, &LspEventSource::processStopped, this,
            &WorkspaceTaskOutputController::handleProcessStopped);
}

QString WorkspaceTaskOutputController::runningTaskId() const
{
    return m_runningTaskId;
}

void WorkspaceTaskOutputController::trackTaskStarted(
    const QString &taskId, const QString &programUri)
{
    if (!m_compilerPanel)
        return;

    m_runningTaskId = taskId;
    for (const QString &chunk : m_runOutput->takeFor(taskId))
        m_compilerPanel->appendRawOutput(chunk.toUtf8());
    if (const auto exitCode = m_runOutput->takeExitFor(taskId)) {
        m_compilerPanel->appendRawOutput(
            (*exitCode == 0
                 ? QStringLiteral("Process exited with code 0")
                 : QStringLiteral("Process exited with code %1")
                       .arg(*exitCode))
                .toUtf8());
        m_runningTaskId.clear();
        notifyStateChanged();
        return;
    }

    m_compilerPanel->appendOutput(
        QStringLiteral("Run started (task %1)").arg(taskId));
    m_compilerPanel->appendOutput(
        QStringLiteral("Program URI: %1").arg(programUri));
    notifyStateChanged();
}

void WorkspaceTaskOutputController::handleProcessStopped(bool)
{
    m_runOutput->clear();
    m_runningTaskId.clear();
    m_progressToken.clear();
    if (m_compilerPanel) {
        m_compilerPanel->appendOutput(
            QStringLiteral("LSP stopped; any active process is no longer "
                           "being tracked."));
    }
    notifyStateChanged();
}

void WorkspaceTaskOutputController::handleWorkDoneProgress(
    const QString &token, const QString &kind, const QString &message)
{
    if (token.isEmpty())
        return;
    if (kind == QLatin1String("begin") && m_progressToken.isEmpty())
        m_progressToken = token;
    if (!m_progressToken.isEmpty() && token != m_progressToken)
        return;
    if (!m_compilerPanel)
        return;

    if (kind == QLatin1String("begin"))
        m_compilerPanel->appendOutput(QStringLiteral("Compiling..."));
    else if (kind == QLatin1String("report"))
        m_compilerPanel->appendOutput(
            message.isEmpty() ? QStringLiteral("Working...") : message);
    else if (kind == QLatin1String("end")) {
        m_compilerPanel->appendOutput(
            message.isEmpty() ? QStringLiteral("Finished") : message);
        m_progressToken.clear();
    }
}

void WorkspaceTaskOutputController::handleProcessOutput(
    const QString &taskId, const QString &chunk)
{
    if (!m_compilerPanel)
        return;

    const QString activeTask = m_runningTaskId;
    if (activeTask.isEmpty()) {
        m_runOutput->buffer(taskId, chunk);
    } else if (activeTask == taskId) {
        m_compilerPanel->appendRawOutput(chunk.toUtf8());
    }
}

void WorkspaceTaskOutputController::handleProcessExit(const QString &taskId,
                                                      int exitCode)
{
    if (!m_compilerPanel)
        return;

    if (m_runningTaskId != taskId) {
        m_runOutput->bufferExit(taskId, exitCode);
        return;
    }

    m_runOutput->discardFor(taskId);
    m_compilerPanel->appendRawOutput(
        (exitCode == 0
             ? QStringLiteral("Process exited with code 0")
             : QStringLiteral("Process exited with code %1").arg(exitCode))
            .toUtf8());
    m_runningTaskId.clear();
    notifyStateChanged();
}

void WorkspaceTaskOutputController::clearRunningTask()
{
    if (m_runningTaskId.isEmpty())
        return;

    m_runOutput->discardFor(m_runningTaskId);
    m_runningTaskId.clear();
    notifyStateChanged();
}

void WorkspaceTaskOutputController::beginCommand()
{
    m_progressToken.clear();
}

void WorkspaceTaskOutputController::notifyStateChanged()
{
    if (m_stateChanged)
        m_stateChanged();
}
