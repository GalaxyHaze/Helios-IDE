#include "LspRuntimeEventController.h"

#include "ClangdLifecycleCoordinator.h"
#include "LspLogPresenter.h"
#include "LspRuntimeController.h"
#include "LspRuntimePresentationController.h"
#include "StatusBarController.h"
#include "ZithRuntimeLifecycleCoordinator.h"
#include "LspEventSource.h"
#include "../editor/LspClient.h"
#include "LspEditorLifecycleController.h"

#include <utility>

LspRuntimeEventController::LspRuntimeEventController(
    Dependencies dependencies, Callbacks callbacks, QObject *parent)
    : QObject(parent),
      m_zithRuntime(dependencies.zithRuntime),
      m_zithClient(dependencies.zithClient),
      m_zithEvents(dependencies.zithEvents),
      m_clangdClient(dependencies.clangdClient),
      m_clangdEvents(dependencies.clangdEvents),
      m_clangdLifecycle(dependencies.clangdLifecycle),
      m_runtimeController(dependencies.runtimeController),
      m_presentation(dependencies.presentation),
      m_editorLifecycle(dependencies.editorLifecycle),
      m_logPresenter(dependencies.logPresenter),
      m_statusBar(dependencies.statusBar),
      m_callbacks(std::move(callbacks))
{
}

void LspRuntimeEventController::attach()
{
    if (m_zithRuntime) {
        connect(m_zithRuntime, &ZithRuntimeLifecycleCoordinator::connected,
                this, &LspRuntimeEventController::handleZithConnected);
        connect(m_zithRuntime, &ZithRuntimeLifecycleCoordinator::stopped, this,
                &LspRuntimeEventController::handleZithStopped);
        connect(m_zithRuntime,
                &ZithRuntimeLifecycleCoordinator::serverError, this,
                &LspRuntimeEventController::handleZithError);
        connect(m_zithRuntime, &ZithRuntimeLifecycleCoordinator::failed, this,
                &LspRuntimeEventController::handleZithError);
        connect(m_zithRuntime,
                &ZithRuntimeLifecycleCoordinator::frontendStatusChanged,
                m_presentation,
                &LspRuntimePresentationController::presentFrontendStatus);
        connect(m_zithRuntime, &ZithRuntimeLifecycleCoordinator::metricsReceived,
                m_presentation,
                &LspRuntimePresentationController::presentMetrics);
        connect(m_zithRuntime, &ZithRuntimeLifecycleCoordinator::stateChanged,
                this, &LspRuntimeEventController::refreshRuntime);
    }

    if (m_zithEvents) {
        connect(m_zithEvents, &LspEventSource::serverError, this,
                [this](const QString &message) {
                    handleClientError(m_zithClient, message);
                });
        connect(m_zithEvents, &LspEventSource::logMessage, this,
                [this](const QString &message) {
                    if (m_logPresenter)
                        m_logPresenter->append(message);
                });
    }

    if (m_clangdEvents) {
        connect(m_clangdEvents, &LspEventSource::serverError, this,
                [this](const QString &message) {
                    handleClientError(m_clangdClient, message);
                });
        connect(m_clangdEvents, &LspEventSource::logMessage, this,
                [this](const QString &message) {
                    if (m_logPresenter)
                        m_logPresenter->append(message);
                });
        connect(m_clangdEvents, &LspEventSource::initialized, this,
                &LspRuntimeEventController::handleClangdInitialized);
        connect(m_clangdEvents, &LspEventSource::serverStopped, this,
                &LspRuntimeEventController::handleClangdStopped);
    }
}

bool LspRuntimeEventController::isEnabled() const
{
    return m_callbacks.lspEnabled && m_callbacks.lspEnabled();
}

void LspRuntimeEventController::handleZithConnected()
{
    if (m_callbacks.refreshActions)
        m_callbacks.refreshActions();
    if (m_presentation)
        m_presentation->presentRuntimeStatus(
            LspRuntimePresentationController::RuntimeStatus::Connected);
    if (m_runtimeController)
        m_runtimeController->clearError();
    refreshRuntime();
    if (m_statusBar)
        m_statusBar->showMessage(QStringLiteral("LSP connected"), 3000);
    if (m_editorLifecycle && m_zithClient)
        m_editorLifecycle->openDocumentsFor(m_zithClient);
}

void LspRuntimeEventController::handleZithStopped()
{
    if (m_editorLifecycle && m_zithClient)
        m_editorLifecycle->detachDocumentsFor(m_zithClient);
    if (m_callbacks.refreshActions)
        m_callbacks.refreshActions();
    if (!isEnabled()) {
        if (m_presentation)
            m_presentation->presentRuntimeStatus(
                LspRuntimePresentationController::RuntimeStatus::Disabled);
    } else if (m_presentation) {
        m_presentation->presentRuntimeStatus(
            LspRuntimePresentationController::RuntimeStatus::Starting);
    }
    refreshRuntime();
}

void LspRuntimeEventController::handleZithError(const QString &message)
{
    if (!isEnabled())
        return;
    if (m_presentation)
        m_presentation->presentRuntimeStatus(
            LspRuntimePresentationController::RuntimeStatus::Error);
    if (m_runtimeController)
        m_runtimeController->recordError(message);
    if (m_logPresenter)
        m_logPresenter->append(QStringLiteral("[error] ") + message);
    if (m_statusBar)
        m_statusBar->showMessage(QStringLiteral("LSP: ") + message, 5000);
}

void LspRuntimeEventController::handleClientError(
    LspClient *client, const QString &message)
{
    if (!isEnabled())
        return;
    if (client == m_zithClient) {
        handleZithError(message);
        return;
    }

    if (m_clangdLifecycle)
        m_clangdLifecycle->recordError(message);
    if (m_presentation)
        m_presentation->refreshClangd();
    if (m_logPresenter)
        m_logPresenter->append(QStringLiteral("[error] ") + message);
    if (m_statusBar)
        m_statusBar->showMessage(QStringLiteral("LSP: ") + message, 5000);
}

void LspRuntimeEventController::handleClangdInitialized()
{
    if (m_editorLifecycle && m_clangdClient)
        m_editorLifecycle->openDocumentsFor(m_clangdClient);
    if (m_clangdLifecycle)
        m_clangdLifecycle->markInitialized();
    if (m_presentation)
        m_presentation->refreshClangd();
}

void LspRuntimeEventController::handleClangdStopped()
{
    if (m_editorLifecycle && m_clangdClient)
        m_editorLifecycle->detachDocumentsFor(m_clangdClient);
    if (m_clangdLifecycle)
        m_clangdLifecycle->markStopped();
    if (m_presentation)
        m_presentation->refreshClangd();
}

void LspRuntimeEventController::refreshRuntime()
{
    if (m_presentation)
        m_presentation->refreshRuntime(
            m_runtimeController ? m_runtimeController->lastError()
                                : QString());
}
