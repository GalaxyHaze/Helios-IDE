#include "LspRuntimePresentationController.h"

#include "ClangdLifecycleCoordinator.h"
#include "LspLogPresenter.h"
#include "StatusBarController.h"
#include "ThemeManager.h"
#include "ZithRuntimeLifecycleCoordinator.h"
#include "../editor/LspClient.h"
#include "../panels/LspManagerDialog.h"
#include "../panels/SettingsPanel.h"

#include <QCoreApplication>
#include <QJsonDocument>

#include <utility>

LspRuntimePresentationController::LspRuntimePresentationController(
    Dependencies dependencies, LspEnabled lspEnabled,
    CFamilyEnabled cFamilyEnabled, ResolvedClangdPath resolvedClangdPath,
    QObject *parent)
    : QObject(parent),
      m_settingsPanel(dependencies.settingsPanel),
      m_lspManagerDialog(dependencies.lspManagerDialog),
      m_logPresenter(dependencies.logPresenter),
      m_statusBar(dependencies.statusBar),
      m_zithRuntime(dependencies.zithRuntime),
      m_zithLspClient(dependencies.zithLspClient),
      m_clangdLifecycle(dependencies.clangdLifecycle),
      m_lspEnabled(std::move(lspEnabled)),
      m_cFamilyEnabled(std::move(cFamilyEnabled)),
      m_resolvedClangdPath(std::move(resolvedClangdPath))
{
}

void LspRuntimePresentationController::setLspStatus(const QString &text,
                                                    const QString &color)
{
    if (m_statusBar)
        m_statusBar->setLspStatus(text, color);
}

void LspRuntimePresentationController::presentRuntimeStatus(
    RuntimeStatus status)
{
    QString text;
    ThemeManager::SemanticRole role = ThemeManager::SemanticRole::Info;
    switch (status) {
    case RuntimeStatus::Disabled:
        text = QStringLiteral("LSP Disabled");
        role = ThemeManager::SemanticRole::TextFaint;
        break;
    case RuntimeStatus::Starting:
        text = QStringLiteral("LSP ○");
        role = ThemeManager::SemanticRole::Warning;
        break;
    case RuntimeStatus::Warming:
        text = QStringLiteral("LSP ◐");
        role = ThemeManager::SemanticRole::Info;
        break;
    case RuntimeStatus::Connected:
        text = QStringLiteral("LSP ⬤");
        role = ThemeManager::SemanticRole::Success;
        break;
    case RuntimeStatus::Error:
        text = QStringLiteral("LSP !");
        role = ThemeManager::SemanticRole::Error;
        break;
    }

    setLspStatus(text, ThemeManager::instance().semanticColor(role).name());
}

void LspRuntimePresentationController::presentFrontendStatus(
    const QJsonObject &status)
{
    const QString state = status.value(QStringLiteral("state")).toString();
    const QString message =
        status.value(QStringLiteral("message")).toString();

    if (state == QStringLiteral("warming"))
        presentRuntimeStatus(RuntimeStatus::Warming);
    else if (state == QStringLiteral("ready"))
        presentRuntimeStatus(RuntimeStatus::Connected);
    else if (state == QStringLiteral("error"))
        presentRuntimeStatus(RuntimeStatus::Error);

    appendLog(QStringLiteral("Frontend status: ") + state +
              (message.isEmpty() ? QString() : QStringLiteral(" - ") + message));
}

void LspRuntimePresentationController::presentMetrics(
    const QJsonObject &metrics)
{
    appendLog(QStringLiteral("Metrics: ") +
              QString::fromUtf8(
                  QJsonDocument(metrics).toJson(QJsonDocument::Compact)));
}

void LspRuntimePresentationController::appendLog(const QString &line)
{
    if (m_logPresenter)
        m_logPresenter->append(line);
}

void LspRuntimePresentationController::refreshRuntime(
    const QString &lastLspError)
{
    if (!m_zithRuntime)
        return;

    const ZithRuntimeState &state = m_zithRuntime->state();
    const QString cachePath = m_zithRuntime->runtimeCacheRootPath();
    const LspRuntimeInfo info{state.statusText(), state.tag(), state.lspPath(),
                              state.stdlibPath(), cachePath};
    if (m_settingsPanel) {
        m_settingsPanel->setRuntimeInfo(info);
    }
    if (m_lspManagerDialog) {
        m_lspManagerDialog->setRuntimeInfo(info);
    }

    refreshDiagnostics(lastLspError);
}

void LspRuntimePresentationController::refreshClangd()
{
    if (!m_clangdLifecycle)
        return;

    const bool enabled = m_lspEnabled && m_lspEnabled();
    const bool cFamilyEnabled =
        m_cFamilyEnabled && m_cFamilyEnabled();
    const QString path =
        m_resolvedClangdPath ? m_resolvedClangdPath() : QString();
    QPair<QString, QString> info;
    if (!enabled || !cFamilyEnabled ||
        m_clangdLifecycle->state() ==
                        ClangdLifecycleCoordinator::State::Disabled)
        info = {QCoreApplication::translate("MainWindow", "Disabled"),
                QString()};
    else if (m_clangdLifecycle->state() ==
             ClangdLifecycleCoordinator::State::Ready)
        info = {
            QCoreApplication::translate("MainWindow", "Connected"),
            m_clangdLifecycle->activePath().isEmpty()
                ? QString()
                : QStringLiteral(
                      "clangd is active. Results improve with "
                      "compile_commands.json.")};
    else if (m_clangdLifecycle->state() ==
             ClangdLifecycleCoordinator::State::MissingPath)
        info = {
            QCoreApplication::translate("MainWindow", "Not found"),
            QCoreApplication::translate(
                "MainWindow",
                "clangd was not found in PATH. Install clangd or set its "
                "path in the LSP Manager.")};
    else if (m_clangdLifecycle->state() ==
             ClangdLifecycleCoordinator::State::Starting)
        info = {
            QCoreApplication::translate("MainWindow", "Starting"),
            QCoreApplication::translate(
                "MainWindow",
                "clangd is starting; it may need compile_commands.json for "
                "full precision.")};
    else
        info = {
            m_clangdLifecycle->error().isEmpty()
                ? QCoreApplication::translate("MainWindow", "Stopped")
                : QCoreApplication::translate("MainWindow", "Error"),
            m_clangdLifecycle->error().isEmpty()
                ? QCoreApplication::translate(
                      "MainWindow",
                      "clangd is configured but not running with an open "
                      "C-family file.")
                : m_clangdLifecycle->error()};

    const ClangdInfo clangdInfo{info.first, path, info.second};
    if (m_settingsPanel)
        m_settingsPanel->setCLspInfo(clangdInfo);
    if (m_lspManagerDialog)
        m_lspManagerDialog->setCLspInfo(clangdInfo);
}

void LspRuntimePresentationController::refreshDiagnostics(
    const QString &lastLspError)
{
    if (!m_settingsPanel && !m_lspManagerDialog)
        return;

    QString connection = QStringLiteral("Not started");
    if (m_zithLspClient) {
        if (m_zithLspClient->isReady())
            connection = QStringLiteral("Connected");
        else if (m_zithLspClient->isRunning())
            connection = QStringLiteral("Starting (waiting for initialize)");
        else
            connection = QStringLiteral("Stopped");
    }

    QString syncMode = QStringLiteral("Unknown");
    if (m_zithLspClient && m_zithLspClient->isReady()) {
        switch (m_zithLspClient->documentSyncKind()) {
        case 2:
            syncMode = QStringLiteral("Incremental");
            break;
        case 1:
            syncMode = QStringLiteral("Full");
            break;
        case 0:
            syncMode = QStringLiteral("None");
            break;
        default:
            break;
        }
    }

    const LspDiagnosticsInfo diagnostics{connection, syncMode, lastLspError};
    if (m_settingsPanel)
        m_settingsPanel->setLspDiagnostics(diagnostics);
    if (m_lspManagerDialog)
        m_lspManagerDialog->setLspDiagnostics(diagnostics);
}
