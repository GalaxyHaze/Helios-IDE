#ifndef CLANGDLIFECYCLECOORDINATOR_H
#define CLANGDLIFECYCLECOORDINATOR_H

#include <QString>

class LspClient;

class ClangdLifecycleCoordinator
{
public:
    enum class State
    {
        Disabled,
        MissingPath,
        Starting,
        Ready,
        Stopped,
        Error,
    };

    struct Configuration
    {
        bool lspEnabled = false;
        bool cFamilyEnabled = false;
        bool hasCFamilyDocuments = false;
        QString serverPath;
        QString workspaceRoot;
    };

    void setClient(LspClient *client);
    void reconcile(const Configuration &configuration);
    void markInitialized();
    void markStopped();
    void recordError(const QString &message);

    State state() const;
    const QString &activePath() const;
    const QString &activeWorkspaceRoot() const;
    const QString &error() const;

private:
    LspClient *m_client = nullptr;
    State m_state = State::Disabled;
    QString m_activePath;
    QString m_activeWorkspaceRoot;
    QString m_error;
};

#endif
