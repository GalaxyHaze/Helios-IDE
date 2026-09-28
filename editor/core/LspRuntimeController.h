#ifndef LSPRUNTIMECONTROLLER_H
#define LSPRUNTIMECONTROLLER_H

#include <QObject>
#include <QString>

#include <functional>

#include "ShellCommand.h"

class ClangdLifecycleCoordinator;
class DiagnosticsPanel;
class LspClient;
class LspCompletionModel;
class LspManagerDialog;
class LspRuntimePresentationController;
class LspSettingsPersistence;
class OutlinePanel;
class SettingsPanel;
class StatusBarController;
class ZithRuntimeLifecycleCoordinator;

class LspRuntimeController : public QObject
{
    Q_OBJECT

public:
    struct Dependencies
    {
        SettingsPanel *settingsPanel = nullptr;
        LspManagerDialog *lspManagerDialog = nullptr;
        ZithRuntimeLifecycleCoordinator *zithRuntime = nullptr;
        LspClient *clangdClient = nullptr;
        LspRuntimePresentationController *presentation = nullptr;
        LspCompletionModel *completionModel = nullptr;
        DiagnosticsPanel *diagnosticsPanel = nullptr;
        OutlinePanel *outlinePanel = nullptr;
        LspSettingsPersistence *settingsPersistence = nullptr;
    };

    struct Callbacks
    {
        std::function<void()> reconcileClangd;
        std::function<void()> refreshActions;
        std::function<void(bool)> setRestartActionEnabled;
        std::function<bool()> confirmCacheClear;
        std::function<void(const QString &, int)> showStatus;
    };

    LspRuntimeController(Dependencies dependencies, Callbacks callbacks,
        QObject *parent = nullptr);

    bool isEnabled() const;
    const QString &lastError() const;

    void initialize(bool enabled);
    void setEnabled(bool enabled);
    void ensureRuntime(bool preferCached);
    void clearRuntimeCache();
    bool handleShellCommand(ShellCommand command);

    void recordError(const QString &message);
    void clearError();

signals:
    void configurationChanged();

private:
    void applyEnabledStateToSurfaces(bool enabled);
    void disableDependentServices();
    void notifyEnabledStateChanged(bool enabled);
    void refreshPresentation();
    void showDisabledState();
    void showStartingState();

    SettingsPanel *m_settingsPanel = nullptr;
    LspManagerDialog *m_lspManagerDialog = nullptr;
    ZithRuntimeLifecycleCoordinator *m_zithRuntime = nullptr;
    LspClient *m_clangdClient = nullptr;
    LspRuntimePresentationController *m_presentation = nullptr;
    LspCompletionModel *m_completionModel = nullptr;
    DiagnosticsPanel *m_diagnosticsPanel = nullptr;
    OutlinePanel *m_outlinePanel = nullptr;
    LspSettingsPersistence *m_settingsPersistence = nullptr;
    Callbacks m_callbacks;
    bool m_enabled = false;
    QString m_lastError;
};

#endif
