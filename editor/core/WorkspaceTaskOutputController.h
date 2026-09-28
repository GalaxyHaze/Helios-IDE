#ifndef WORKSPACETASKOUTPUTCONTROLLER_H
#define WORKSPACETASKOUTPUTCONTROLLER_H

#include <QObject>
#include <QString>

#include <functional>
#include <memory>

#include "RunOutputCollector.h"

class CompilerPanel;
class LspEventSource;

class WorkspaceTaskOutputController : public QObject
{
    Q_OBJECT

public:
    using StateChanged = std::function<void()>;

    WorkspaceTaskOutputController(LspEventSource *events,
                                  CompilerPanel *compilerPanel,
                                  StateChanged stateChanged,
                                  QObject *parent = nullptr);

    QString runningTaskId() const;
    void trackTaskStarted(const QString &taskId, const QString &programUri);
    void clearRunningTask();
    void beginCommand();

public slots:
    void handleProcessStopped(bool expected);
    void handleWorkDoneProgress(const QString &token, const QString &kind,
                                const QString &message);
    void handleProcessOutput(const QString &taskId, const QString &chunk);
    void handleProcessExit(const QString &taskId, int exitCode);

private:
    void notifyStateChanged();

    CompilerPanel *m_compilerPanel;
    StateChanged m_stateChanged;
    std::unique_ptr<RunOutputCollector> m_runOutput;
    QString m_runningTaskId;
    QString m_progressToken;
};

#endif
