#ifndef WORKSPACEROOTCONTROLLER_H
#define WORKSPACEROOTCONTROLLER_H

#include <QObject>
#include <QString>

#include <functional>

class ContextManager;

class WorkspaceRootController : public QObject
{
public:
    enum class ActivationResult
    {
        Rejected,
        ReplacedCurrentContext,
        CreatedContext,
        RestoredCurrentContext,
    };

    struct Callbacks
    {
        std::function<void()> saveCurrentContext;
        std::function<void(const QString &)> persistRecentProject;
        std::function<void()> refreshRecentProjects;
    };

    WorkspaceRootController(ContextManager *contextManager,
                            Callbacks callbacks,
                            QObject *parent = nullptr);

    ActivationResult replaceCurrentRoot(const QString &path);
    ActivationResult createContext(const QString &path);
    ActivationResult restoreCurrentRoot(const QString &path);

private:
    static QString normalizeDirectory(const QString &path);
    ActivationResult activate(const QString &path, bool createContext,
                             bool persistActivation);

    ContextManager *m_contextManager = nullptr;
    Callbacks m_callbacks;
};

#endif
