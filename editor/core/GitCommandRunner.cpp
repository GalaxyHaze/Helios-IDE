#include "GitCommandRunner.h"

#include <QProcess>
#include <QTimer>

namespace
{
constexpr int kGitCommandTimeoutMs = 10000;
}

GitCommandRunner::GitCommandRunner(QObject *parent)
    : QObject(parent)
{
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &GitCommandRunner::handleFinished);
    connect(m_process, &QProcess::errorOccurred, this,
            &GitCommandRunner::handleError);

    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(kGitCommandTimeoutMs);
    connect(m_timeoutTimer, &QTimer::timeout, this,
            &GitCommandRunner::handleTimeout);
}

void GitCommandRunner::setProgram(const QString &program)
{
    m_program = program;
}

void GitCommandRunner::setWorkingDirectory(const QString &path)
{
    m_workingDirectory = path;
}

bool GitCommandRunner::isRunning() const
{
    return m_process->state() != QProcess::NotRunning;
}

bool GitCommandRunner::run(const QStringList &arguments)
{
    if (isRunning())
        return false;

    m_arguments = arguments;
    m_resultEmitted = false;
    m_timedOut = false;
    m_process->setWorkingDirectory(m_workingDirectory);
    m_process->start(m_program, m_arguments);
    m_timeoutTimer->start();
    return true;
}

void GitCommandRunner::handleFinished(int exitCode,
                                      QProcess::ExitStatus exitStatus)
{
    if (m_resultEmitted)
        return;

    const bool succeeded =
        !m_timedOut && exitStatus == QProcess::NormalExit && exitCode == 0;
    emitResult(succeeded, false, m_timedOut);
}

void GitCommandRunner::handleError(QProcess::ProcessError error)
{
    if (m_resultEmitted || error != QProcess::FailedToStart)
        return;

    emitResult(false, true);
}

void GitCommandRunner::handleTimeout()
{
    if (m_resultEmitted || !isRunning())
        return;

    m_timedOut = true;
    m_process->kill();
}

void GitCommandRunner::emitResult(bool succeeded,
                                  bool failedToStart,
                                  bool timedOut)
{
    if (m_resultEmitted)
        return;

    m_resultEmitted = true;
    m_timeoutTimer->stop();
    GitCommandResult result;
    result.arguments = m_arguments;
    result.standardOutput =
        QString::fromUtf8(m_process->readAllStandardOutput()).trimmed();
    result.standardError =
        QString::fromUtf8(m_process->readAllStandardError()).trimmed();
    result.succeeded = succeeded;
    result.failedToStart = failedToStart;
    result.timedOut = timedOut;
    emit finished(result);
}
