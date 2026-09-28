#ifndef GITCOMMANDRUNNER_H
#define GITCOMMANDRUNNER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class QTimer;

struct GitCommandResult
{
    QStringList arguments;
    QString standardOutput;
    QString standardError;
    bool succeeded = false;
    bool failedToStart = false;
    bool timedOut = false;
};

class GitCommandRunner : public QObject
{
    Q_OBJECT

public:
    explicit GitCommandRunner(QObject *parent = nullptr);

    void setProgram(const QString &program);
    void setWorkingDirectory(const QString &path);
    bool isRunning() const;
    bool run(const QStringList &arguments);

signals:
    void finished(const GitCommandResult &result);

private slots:
    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleError(QProcess::ProcessError error);
    void handleTimeout();

private:
    void emitResult(bool succeeded,
                    bool failedToStart = false,
                    bool timedOut = false);

    QProcess *m_process = nullptr;
    QTimer *m_timeoutTimer = nullptr;
    QString m_program = QStringLiteral("git");
    QString m_workingDirectory;
    QStringList m_arguments;
    bool m_resultEmitted = false;
    bool m_timedOut = false;
};

#endif
