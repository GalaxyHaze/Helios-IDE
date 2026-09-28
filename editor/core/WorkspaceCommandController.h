#ifndef WORKSPACECOMMANDCONTROLLER_H
#define WORKSPACECOMMANDCONTROLLER_H

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QString>

#include <functional>
#include <memory>

#include "ShellCommand.h"
#include "WorkspaceCommandState.h"
#include "WorkspaceCommandResultDecoder.h"

class CodeEditor;
class CompilerPanel;
class LspClient;
class LspEventSource;
class WorkspaceTaskOutputController;

class WorkspaceCommandController : public QObject
{
    Q_OBJECT

public:
    struct Dependencies
    {
        LspClient *client = nullptr;
        LspEventSource *events = nullptr;
        CompilerPanel *compilerPanel = nullptr;
    };

    struct Callbacks
    {
        std::function<bool()> lspEnabled;
        std::function<QString()> workspaceRoot;
        std::function<CodeEditor *()> currentEditor;
        std::function<bool(CodeEditor *)> isZithEditor;
        std::function<void()> saveAll;
        std::function<void()> showCompiler;
        std::function<void()> appendPublishedDiagnostics;
        std::function<void(const QString &, int)> showStatus;
    };

    WorkspaceCommandController(Dependencies dependencies, Callbacks callbacks,
                               QObject *parent = nullptr);
    ~WorkspaceCommandController() override;

    bool canExecuteWorkspaceCommand() const;
    bool canExecute(ShellCommand command) const;
    WorkspaceCommandState state() const;

public slots:
    void runBuild();
    void runCheckFile();
    void runProject();
    void stopRunningTask();
    bool handleShellCommand(ShellCommand command);
    void handleCommandResult(const QString &command, bool success,
                             const QJsonValue &result);

signals:
    void stateChanged(const WorkspaceCommandState &state);

private:
    struct UnavailableCommand
    {
        QString title;
        QString message;
    };

    void showUnavailable(const UnavailableCommand &command);
    void execute(const QString &command, const QJsonArray &arguments,
                 const QString &title);
    void appendServerFailure(const QString &command,
                             const WorkspaceCommandResult &result);
    void notifyStateChanged();

    LspClient *m_client;
    CompilerPanel *m_compilerPanel;
    Callbacks m_callbacks;
    std::unique_ptr<WorkspaceTaskOutputController> m_taskOutput;
};

#endif
