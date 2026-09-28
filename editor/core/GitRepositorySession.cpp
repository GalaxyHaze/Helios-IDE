#include "GitRepositorySession.h"

#include <QTimer>

GitRepositorySession::GitRepositorySession(QObject *parent)
    : QObject(parent),
      m_runner(new GitCommandRunner(this))
{
    connect(m_runner, &GitCommandRunner::finished,
            this, &GitRepositorySession::handleCommandFinished);
}

void GitRepositorySession::setProgram(const QString &program)
{
    m_runner->setProgram(program);
}

void GitRepositorySession::setRootPath(const QString &path)
{
    m_rootPath = path;
    m_state = {};
    m_state.rootPath = path;
    m_state.busy = m_busy;
    publishState();
    refresh();
}

QString GitRepositorySession::rootPath() const
{
    return m_rootPath;
}

GitRepositoryState GitRepositorySession::state() const
{
    return m_state;
}

bool GitRepositorySession::isBusy() const
{
    return m_activeOperation != Operation::None;
}

void GitRepositorySession::refresh()
{
    if (isBusy()) {
        m_refreshPending = true;
        return;
    }

    if (m_rootPath.isEmpty()) {
        m_state.branch.clear();
        m_state.entries.clear();
        m_state.repositoryAvailable = false;
        m_state.remoteAvailable = false;
        publishState();
        emit messageChanged(QStringLiteral("Open a project inside a Git repository."),
                            false);
        return;
    }

    run(Operation::Status, {QStringLiteral("status"),
                            QStringLiteral("--short"),
                            QStringLiteral("--branch")});
}

void GitRepositorySession::stageAll()
{
    m_pendingFileCount = m_state.entries.size();
    run(Operation::Stage, {QStringLiteral("add"), QStringLiteral("--all")});
}

void GitRepositorySession::stage(const QStringList &relativePaths)
{
    if (relativePaths.isEmpty())
        return;

    m_pendingFileCount = relativePaths.size();
    QStringList arguments = {QStringLiteral("add"), QStringLiteral("--")};
    arguments.append(relativePaths);
    run(Operation::Stage, arguments);
}

void GitRepositorySession::unstage(const QStringList &relativePaths)
{
    if (relativePaths.isEmpty())
        return;

    m_pendingFileCount = relativePaths.size();
    QStringList arguments = {QStringLiteral("restore"),
                             QStringLiteral("--staged"),
                             QStringLiteral("--")};
    arguments.append(relativePaths);
    run(Operation::Unstage, arguments);
}

void GitRepositorySession::commit(const QString &message)
{
    const QString normalizedMessage = message.trimmed();
    if (normalizedMessage.isEmpty()) {
        emit messageChanged(QStringLiteral("Enter a commit message before committing."),
                            true);
        return;
    }

    run(Operation::Commit, {QStringLiteral("commit"),
                            QStringLiteral("-m"),
                            normalizedMessage});
}

void GitRepositorySession::initializeRepository()
{
    run(Operation::Init, {QStringLiteral("init"), QStringLiteral("-b"),
                          QStringLiteral("main")});
}

void GitRepositorySession::connectToRemote(const QString &url)
{
    const QString normalizedUrl = url.trimmed();
    if (normalizedUrl.isEmpty())
        return;

    run(Operation::ConnectToRemote, {QStringLiteral("remote"),
                                     QStringLiteral("add"),
                                     QStringLiteral("origin"),
                                     normalizedUrl});
}

void GitRepositorySession::handleCommandFinished(const GitCommandResult &result)
{
    if (!isBusy())
        return;

    const Operation operation = m_activeOperation;
    m_activeOperation = Operation::None;

    if (result.failedToStart) {
        emit messageChanged(QStringLiteral("Failed to start Git."), true);
        requestRefreshAfterOperation();
    } else if (result.timedOut) {
        emit messageChanged(QStringLiteral("Timed out while running Git."), true);
        m_refreshPending = true;
    } else {
        handleFinished(operation, result.standardOutput, result.standardError,
                       result.succeeded);
    }

    if (isBusy())
        return;

    if (m_refreshPending) {
        m_refreshPending = false;
        refresh();
    } else if (operation != Operation::Status
               && operation != Operation::Remote) {
        QTimer::singleShot(0, this, &GitRepositorySession::refresh);
    } else {
        setBusy(false);
    }
}

void GitRepositorySession::run(Operation operation, const QStringList &arguments)
{
    if (m_rootPath.isEmpty()) {
        emit messageChanged(QStringLiteral("Open a project inside a Git repository."),
                            true);
        return;
    }

    if (isBusy()) {
        m_refreshPending = true;
        return;
    }

    m_activeOperation = operation;
    m_runner->setWorkingDirectory(m_rootPath);
    if (!m_runner->run(arguments)) {
        m_activeOperation = Operation::None;
        emit messageChanged(QStringLiteral("Git is already running."), true);
        return;
    }

    setBusy(true);
}

void GitRepositorySession::handleFinished(Operation operation,
                                          const QString &standardOutput,
                                          const QString &standardError,
                                          bool succeeded)
{
    switch (operation) {
    case Operation::Status: {
        if (!succeeded) {
            m_state.branch.clear();
            m_state.entries.clear();
            m_state.repositoryAvailable = false;
            m_state.remoteAvailable = false;
            publishState();
            emitErrorOrMessage(standardError,
                               QStringLiteral("Current workspace is not a Git repository."),
                               false);
            return;
        }

        const GitStatusSnapshot snapshot =
            GitStatusParser::parse(standardOutput);
        m_state.branch = snapshot.branch;
        m_state.entries = snapshot.entries;
        m_state.repositoryAvailable = true;
        m_state.remoteAvailable = false;
        publishState();
        run(Operation::Remote, {QStringLiteral("remote")});
        return;
    }

    case Operation::Remote:
        m_state.remoteAvailable =
            succeeded && standardOutput.contains(QStringLiteral("origin"));
        publishState();
        return;

    case Operation::Stage: {
        const qsizetype count = m_pendingFileCount > 0
                                    ? m_pendingFileCount
                                    : qsizetype(0);
        emitErrorOrMessage(standardError,
                           count > 0
                               ? QStringLiteral("Staged %1 file(s).")
                                     .arg(QString::number(count))
                               : QStringLiteral("Staged all files."),
                           succeeded);
        m_pendingFileCount = 0;
        return;
    }

    case Operation::Unstage:
        emitErrorOrMessage(standardError,
                           QStringLiteral("Unstaged %1 file(s).")
                               .arg(m_pendingFileCount),
                           succeeded);
        m_pendingFileCount = 0;
        return;

    case Operation::Commit:
        if (succeeded)
            emit commitSucceeded();
        emitErrorOrMessage(standardError,
                           standardOutput.simplified().isEmpty()
                               ? QStringLiteral("Commit created successfully.")
                               : standardOutput.simplified(),
                           succeeded);
        return;

    case Operation::Init:
        emitErrorOrMessage(standardError,
                           QStringLiteral("Repository initialized."),
                           succeeded);
        return;

    case Operation::ConnectToRemote:
        emitErrorOrMessage(standardError,
                           QStringLiteral("Remote 'origin' added."),
                           succeeded);
        return;

    case Operation::None:
        return;
    }
}

void GitRepositorySession::requestRefreshAfterOperation()
{
    if (m_rootPath.isEmpty())
        return;

    m_refreshPending = true;
}

void GitRepositorySession::setBusy(bool busy)
{
    if (busy == m_busy)
        return;

    m_busy = busy;
    m_state.busy = busy;
    publishState();
}

void GitRepositorySession::publishState()
{
    m_state.rootPath = m_rootPath;
    emit stateChanged(m_state);
}

void GitRepositorySession::emitErrorOrMessage(const QString &error,
                                              const QString &fallback,
                                              bool success)
{
    if (success)
        emit messageChanged(fallback, false);
    else
        emit messageChanged(error.isEmpty() ? fallback : error, true);
}
