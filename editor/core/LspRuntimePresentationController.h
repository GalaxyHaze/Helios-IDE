#ifndef LSPRUNTIMEPRESENTATIONCONTROLLER_H
#define LSPRUNTIMEPRESENTATIONCONTROLLER_H

#include <QObject>
#include <QJsonObject>
#include <QString>

#include <functional>

class ClangdLifecycleCoordinator;
class LspClient;
class LspLogPresenter;
class LspManagerDialog;
class SettingsPanel;
class StatusBarController;
class ZithRuntimeLifecycleCoordinator;

class LspRuntimePresentationController : public QObject
{
public:
    enum class RuntimeStatus
    {
        Disabled,
        Starting,
        Warming,
        Connected,
        Error
    };

    using LspEnabled = std::function<bool()>;
    using CFamilyEnabled = std::function<bool()>;
    using ResolvedClangdPath = std::function<QString()>;

    struct Dependencies
    {
        SettingsPanel *settingsPanel = nullptr;
        LspManagerDialog *lspManagerDialog = nullptr;
        LspLogPresenter *logPresenter = nullptr;
        StatusBarController *statusBar = nullptr;
        ZithRuntimeLifecycleCoordinator *zithRuntime = nullptr;
        LspClient *zithLspClient = nullptr;
        ClangdLifecycleCoordinator *clangdLifecycle = nullptr;
    };

    LspRuntimePresentationController(Dependencies dependencies,
                                     LspEnabled lspEnabled,
                                     CFamilyEnabled cFamilyEnabled,
                                     ResolvedClangdPath resolvedClangdPath,
                                     QObject *parent = nullptr);

    void setLspStatus(const QString &text, const QString &color);
    void presentRuntimeStatus(RuntimeStatus status);
    void presentFrontendStatus(const QJsonObject &status);
    void presentMetrics(const QJsonObject &metrics);
    void refreshRuntime(const QString &lastLspError);
    void refreshClangd();
    void refreshDiagnostics(const QString &lastLspError);

private:
    void appendLog(const QString &line);

    SettingsPanel *m_settingsPanel = nullptr;
    LspManagerDialog *m_lspManagerDialog = nullptr;
    LspLogPresenter *m_logPresenter = nullptr;
    StatusBarController *m_statusBar = nullptr;
    ZithRuntimeLifecycleCoordinator *m_zithRuntime = nullptr;
    LspClient *m_zithLspClient = nullptr;
    ClangdLifecycleCoordinator *m_clangdLifecycle = nullptr;
    LspEnabled m_lspEnabled;
    CFamilyEnabled m_cFamilyEnabled;
    ResolvedClangdPath m_resolvedClangdPath;
};

#endif
