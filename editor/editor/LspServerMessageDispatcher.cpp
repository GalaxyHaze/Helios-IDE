#include "LspServerMessageDispatcher.h"

#include "LspResultDecoder.h"

#include <utility>

LspServerMessageDispatcher::LspServerMessageDispatcher(
    LspServerMessageDispatcherDependencies dependencies, QObject *parent)
    : QObject(parent), m_dependencies(std::move(dependencies))
{
}

void LspServerMessageDispatcher::dispatch(const QJsonObject &message)
{
    if (!message.contains("method"))
        return;

    if (message.contains("id"))
        handleServerRequest(message);
    else
        handleNotification(message);
}

void LspServerMessageDispatcher::handleServerRequest(
    const QJsonObject &message)
{
    const QString method = message.value("method").toString();
    const QJsonValue id = message.value("id");
    if (method == QLatin1String("window/workDoneProgress/create")) {
        if (m_dependencies.isRunning && m_dependencies.isRunning() &&
            m_dependencies.sendMessage)
            m_dependencies.sendMessage(
                {{"jsonrpc", "2.0"}, {"id", id}, {"result", QJsonValue::Null}});
        return;
    }

    if (m_dependencies.isRunning && m_dependencies.isRunning() &&
        m_dependencies.sendMessage)
        m_dependencies.sendMessage(
            {{"jsonrpc", "2.0"},
             {"id", id},
             {"error",
              QJsonObject{{"code", -32601},
                          {"message", QStringLiteral("Method not found: ") +
                                          method}}}});
}

void LspServerMessageDispatcher::handleNotification(
    const QJsonObject &message)
{
    const QString method = message.value("method").toString();
    const QJsonObject params = message.value("params").toObject();
    if (method == QLatin1String("textDocument/publishDiagnostics")) {
        const QString uri = params.value("uri").toString();
        const int version =
            params.contains("version") ? params.value("version").toInt() : -1;
        if (m_dependencies.isCurrentDocument &&
            !m_dependencies.isCurrentDocument(uri, version))
            return;
        emit diagnosticsReceived(
            uri, version,
            LspResultDecoder::diagnostics(
                params.value("diagnostics").toArray()));
    } else if (method == QLatin1String("window/logMessage")) {
        emit logMessage(params.value("message").toString());
    } else if (method == QLatin1String("window/showMessage")) {
        emit logMessage(params.value("message").toString());
        emit showMessage(params.value("message").toString());
    } else if (method == QLatin1String("zith/requestSaveAll")) {
        emit saveAllRequested();
    } else if (method == QLatin1String("zith/frontendStatus")) {
        emit frontendStatusReceived(params);
    } else if (method == QLatin1String("zith/metrics")) {
        emit metricsReceived(params);
    } else if (method == QLatin1String("zith/processOutput")) {
        emit processOutputReceived(params.value("taskId").toString(),
                                   params.value("chunk").toString());
    } else if (method == QLatin1String("zith/processExit")) {
        emit processExitReceived(params.value("taskId").toString(),
                                 params.value("exitCode").toInt(-1));
    } else if (method == QLatin1String("$/progress")) {
        const QJsonObject value = params.value("value").toObject();
        emit workDoneProgressReceived(
            params.value("token").toString(), value.value("kind").toString(),
            value.value("message").toString());
    }
}
