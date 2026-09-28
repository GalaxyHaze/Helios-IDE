#ifndef GITREPOSITORYSESSION_H
#define GITREPOSITORYSESSION_H

#include "GitCommandRunner.h"
#include "GitStatusParser.h"

#include <QObject>

struct GitRepositoryState
{
    QString rootPath;
    QString branch;
    QList<GitStatusEntry> entries;
    bool repositoryAvailable = false;
    bool remoteAvailable = false;
    bool busy = false;
};

class GitRepositorySession : public QObject
{
    Q_OBJECT

public:
    explicit GitRepositorySession(QObject *parent = nullptr);

    void setProgram(const QString &program);
    void setRootPath(const QString &path);
    QString rootPath() const;
    GitRepositoryState state() const;
    bool isBusy() const;

public slots:
    void refresh();
    void stageAll();
    void stage(const QStringList &relativePaths);
    void unstage(const QStringList &relativePaths);
    void commit(const QString &message);
    void initializeRepository();
    void connectToRemote(const QString &url);

signals:
    void stateChanged(const GitRepositoryState &state);
    void messageChanged(const QString &message, bool isError);
    void commitSucceeded();

private slots:
    void handleCommandFinished(const GitCommandResult &result);

private:
    enum class Operation {
        None,
        Status,
        Remote,
        Stage,
        Unstage,
        Commit,
        Init,
        ConnectToRemote
    };

    void run(Operation operation, const QStringList &arguments);
    void handleFinished(Operation operation,
                        const QString &standardOutput,
                        const QString &standardError,
                        bool succeeded);
    void requestRefreshAfterOperation();
    void setBusy(bool busy);
    void publishState();
    void emitErrorOrMessage(const QString &error,
                            const QString &fallback,
                            bool success);

    GitCommandRunner *m_runner = nullptr;
    QString m_rootPath;
    Operation m_activeOperation = Operation::None;
    bool m_refreshPending = false;
    bool m_busy = false;
    qsizetype m_pendingFileCount = 0;
    GitRepositoryState m_state;
};

Q_DECLARE_METATYPE(GitStatusSnapshot)
Q_DECLARE_METATYPE(GitRepositoryState)

#endif
