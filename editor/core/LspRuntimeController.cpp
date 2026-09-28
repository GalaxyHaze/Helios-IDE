#include "LspRuntimeController.h"

#include "LspRuntimePresentationController.h"
#include "LspSettingsPersistence.h"
#include "StatusBarController.h"
#include "ZithRuntimeLifecycleCoordinator.h"
#include "../editor/LspClient.h"
#include "../editor/LspCompletionModel.h"
#include "../panels/DiagnosticsPanel.h"
#include "../panels/LspManagerDialog.h"
#include "../panels/OutlinePanel.h"
#include "../panels/SettingsPanel.h"

#include <utility>

LspRuntimeController::LspRuntimeController(
    Dependencies dependencies, Callbacks callbacks, QObject *parent)
    : QObject(parent),
      m_settingsPanel(dependencies.settingsPanel),
      m_lspManagerDialog(dependencies.lspManagerDialog),
      m_zithRuntime(dependencies.zithRuntime),
      m_clangdClient(dependencies.clangdClient),
      m_presentation(dependencies.presentation),
      m_completionModel(dependencies.completionModel),
      m_diagnosticsPanel(dependencies.diagnosticsPanel),
      m_outlinePanel(dependencies.outlinePanel),
      m_settingsPersistence(dependencies.settingsPersistence),
      m_callbacks(std::move(callbacks))
{
    if (m_settingsPanel) {
        connect(m_settingsPanel, &SettingsPanel::lspEnabledChanged, this,
                [this](bool enabled) { setEnabled(enabled); });
        connect(m_settingsPanel, &SettingsPanel::refreshRuntimeRequested, this,
                [this]() {
                    if (m_enabled)
                        ensureRuntime(false);
                });
        connect(m_settingsPanel, &SettingsPanel::clearRuntimeCacheRequested,
                this, [this]() { clearRuntimeCache(); });
    }

    if (m_lspManagerDialog) {
        connect(m_lspManagerDialog, &LspManagerDialog::lspEnabledChanged,
                this, [this](bool enabled) { setEnabled(enabled); });
        connect(m_lspManagerDialog,
                &LspManagerDialog::useOnlineZithLspChanged, this,
                [this](bool enabled) {
                    if (m_settingsPersistence)
                        m_settingsPersistence->setUseOnlineZithLsp(enabled);
                    if (m_zithRuntime)
                        m_zithRuntime->setPreferOnline(enabled);
                    if (m_enabled)
                        ensureRuntime(false);
                });
        connect(m_lspManagerDialog, &LspManagerDialog::cLspEnabledChanged,
                this, [this](bool enabled) {
                    if (m_settingsPersistence)
                        m_settingsPersistence->setCLspEnabled(enabled);
                    if (m_callbacks.reconcileClangd)
                        m_callbacks.reconcileClangd();
                });
        connect(m_lspManagerDialog, &LspManagerDialog::cLspPathChanged, this,
                [this](const QString &path) {
                    if (m_settingsPersistence)
                        m_settingsPersistence->setCLspPath(path);
                    if (m_callbacks.reconcileClangd)
                        m_callbacks.reconcileClangd();
                });
        connect(m_lspManagerDialog,
                &LspManagerDialog::refreshRuntimeRequested, this,
                [this]() {
                    if (m_enabled)
                        ensureRuntime(false);
                });
        connect(m_lspManagerDialog,
                &LspManagerDialog::clearRuntimeCacheRequested, this,
                [this]() { clearRuntimeCache(); });
    }
}

bool LspRuntimeController::isEnabled() const { return m_enabled; }

const QString &LspRuntimeController::lastError() const
{
    return m_lastError;
}

void LspRuntimeController::initialize(bool enabled)
{
    m_enabled = enabled;
    applyEnabledStateToSurfaces(enabled);

    if (enabled)
        ensureRuntime(true);
    else
        showDisabledState();

    notifyEnabledStateChanged(enabled);
}

void LspRuntimeController::setEnabled(bool enabled)
{
    m_enabled = enabled;
    if (m_settingsPersistence)
        m_settingsPersistence->setLspEnabled(enabled);
    applyEnabledStateToSurfaces(enabled);

    if (!enabled) {
        disableDependentServices();
        showDisabledState();
    } else {
        clearError();
        ensureRuntime(true);
    }

    notifyEnabledStateChanged(enabled);
}

void LspRuntimeController::applyEnabledStateToSurfaces(bool enabled)
{
    if (m_settingsPanel)
        m_settingsPanel->setLspEnabled(enabled);
    if (m_lspManagerDialog)
        m_lspManagerDialog->setLspEnabled(enabled);
    if (m_zithRuntime)
        m_zithRuntime->setEnabled(enabled);
}

void LspRuntimeController::disableDependentServices()
{
    if (m_clangdClient)
        m_clangdClient->stop();
    if (m_completionModel)
        m_completionModel->setItems({});
    if (m_diagnosticsPanel)
        m_diagnosticsPanel->clear();
    if (m_outlinePanel)
        m_outlinePanel->clear();
}

void LspRuntimeController::notifyEnabledStateChanged(bool enabled)
{
    if (m_callbacks.reconcileClangd)
        m_callbacks.reconcileClangd();
    if (m_callbacks.refreshActions)
        m_callbacks.refreshActions();
    if (m_callbacks.setRestartActionEnabled)
        m_callbacks.setRestartActionEnabled(enabled);
}

void LspRuntimeController::ensureRuntime(bool preferCached)
{
    if (!m_enabled || !m_zithRuntime)
        return;

    showStartingState();
    m_zithRuntime->ensureLatest(preferCached);
}

void LspRuntimeController::clearRuntimeCache()
{
    if (!m_enabled || !m_zithRuntime)
        return;
    if (m_callbacks.confirmCacheClear &&
        !m_callbacks.confirmCacheClear())
        return;

    QString errorMessage;
    if (!m_zithRuntime->clearCachedRuntime(&errorMessage)) {
        recordError(errorMessage);
        if (m_presentation)
            m_presentation->presentRuntimeStatus(
                LspRuntimePresentationController::RuntimeStatus::Error);
        refreshPresentation();
        if (m_callbacks.showStatus)
            m_callbacks.showStatus(errorMessage, 8000);
        return;
    }

    if (m_completionModel)
        m_completionModel->setItems({});
    showStartingState();
    if (m_callbacks.showStatus)
        m_callbacks.showStatus(QStringLiteral("Zith runtime cache cleared."),
                               4000);
    ensureRuntime(false);
}

bool LspRuntimeController::handleShellCommand(ShellCommand command)
{
    if (command != ShellCommand::RestartLsp)
        return false;

    if (m_enabled)
        ensureRuntime(false);
    else
        refreshPresentation();
    return true;
}

void LspRuntimeController::recordError(const QString &message)
{
    m_lastError = message;
    refreshPresentation();
}

void LspRuntimeController::clearError()
{
    m_lastError.clear();
}

void LspRuntimeController::refreshPresentation()
{
    if (m_presentation)
        m_presentation->refreshRuntime(m_lastError);
}

void LspRuntimeController::showDisabledState()
{
    if (m_presentation)
        m_presentation->presentRuntimeStatus(
            LspRuntimePresentationController::RuntimeStatus::Disabled);
    refreshPresentation();
}

void LspRuntimeController::showStartingState()
{
    if (m_presentation)
        m_presentation->presentRuntimeStatus(
            LspRuntimePresentationController::RuntimeStatus::Starting);
    refreshPresentation();
}
