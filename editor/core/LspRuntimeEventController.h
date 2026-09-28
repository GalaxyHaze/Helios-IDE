#ifndef LSPRUNTIMEEVENTCONTROLLER_H
#define LSPRUNTIMEEVENTCONTROLLER_H

#include <QObject>

#include <functional>

class ClangdLifecycleCoordinator;
class LspClient;
class LspEventSource;
class LspEditorLifecycleController;
class LspLogPresenter;
class LspRuntimeController;
class LspRuntimePresentationController;
class StatusBarController;
class ZithRuntimeLifecycleCoordinator;

class LspRuntimeEventController : public QObject
{
    Q_OBJECT

public:
    struct Dependencies
    {
        ZithRuntimeLifecycleCoordinator *zithRuntime = nullptr;
        LspClient *zithClient = nullptr;
        LspEventSource *zithEvents = nullptr;
        LspClient *clangdClient = nullptr;
        LspEventSource *clangdEvents = nullptr;
        ClangdLifecycleCoordinator *clangdLifecycle = nullptr;
        LspRuntimeController *runtimeController = nullptr;
        LspRuntimePresentationController *presentation = nullptr;
        LspEditorLifecycleController *editorLifecycle = nullptr;
        LspLogPresenter *logPresenter = nullptr;
        StatusBarController *statusBar = nullptr;
    };

    struct Callbacks
    {
        std::function<bool()> lspEnabled;
        std::function<void()> refreshActions;
    };

    LspRuntimeEventController(Dependencies dependencies, Callbacks callbacks,
        QObject *parent = nullptr);

    void attach();

private:
    bool isEnabled() const;
    void handleZithConnected();
    void handleZithStopped();
    void handleZithError(const QString &message);
    void handleClientError(LspClient *client, const QString &message);
    void handleClangdInitialized();
    void handleClangdStopped();
    void refreshRuntime();

    ZithRuntimeLifecycleCoordinator *m_zithRuntime = nullptr;
    LspClient *m_zithClient = nullptr;
    LspEventSource *m_zithEvents = nullptr;
    LspClient *m_clangdClient = nullptr;
    LspEventSource *m_clangdEvents = nullptr;
    ClangdLifecycleCoordinator *m_clangdLifecycle = nullptr;
    LspRuntimeController *m_runtimeController = nullptr;
    LspRuntimePresentationController *m_presentation = nullptr;
    LspEditorLifecycleController *m_editorLifecycle = nullptr;
    LspLogPresenter *m_logPresenter = nullptr;
    StatusBarController *m_statusBar = nullptr;
    Callbacks m_callbacks;
};

#endif
