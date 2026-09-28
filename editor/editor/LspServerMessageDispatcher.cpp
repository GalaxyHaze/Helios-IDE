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
    const auto sendResult = [this, &id](const QJsonValue &result) {
        if (m_dependencies.isRunning && m_dependencies.isRunning() &&
            m_dependencies.sendMessage)
            m_dependencies.sendMessage(
                {{"jsonrpc", "2.0"}, {"id", id}, {"result", result}});
    };
    const auto sendError = [this, &id](int code, const QString &text) {
        if (m_dependencies.isRunning && m_dependencies.isRunning() &&
            m_dependencies.sendMessage)
            m_dependencies.sendMessage(
                {{"jsonrpc", "2.0"},
                 {"id", id},
                 {"error", QJsonObject{{"code", code}, {"message", text}}}});
    };

    if (method == QLatin1String("window/workDoneProgress/create")) {
        sendResult(QJsonValue::Null);
        return;
    }

    if (method == QLatin1String("workspace/applyEdit")) {
        if (m_dependencies.applyWorkspaceEdit) {
            sendResult(m_dependencies.applyWorkspaceEdit(
                message.value("params").toObject()));
        }
        else {
            sendResult(QJsonObject{
                {"applied", false},
                {"failureReason",
                 QStringLiteral("Workspace edit handling is unavailable.")}});
        }
        return;
    }

    if (method == QLatin1String("window/showMessageRequest")) {
        const QJsonValue result = m_dependencies.showMessageRequest
                                      ? m_dependencies.showMessageRequest(
                                            message.value("params").toObject())
                                      : QJsonValue::Null;
        sendResult(result);
        return;
    }

    if (method == QLatin1String("workspace/configuration")) {
        const QJsonArray items =
            message.value("params").toObject().value("items").toArray();
        QJsonArray values;
        if (m_dependencies.configuration)
            values = m_dependencies.configuration(items);
        else
            for (int index = 0; index < items.size(); ++index)
                values.append(QJsonValue::Null);
        sendResult(values);
        return;
    }

    if (method == QLatin1String("client/registerCapability")) {
        if (m_dependencies.registerCapability)
            m_dependencies.registerCapability(
                message.value("params")
                    .toObject()
                    .value("registrations")
                    .toArray());
        sendResult(QJsonValue::Null);
        return;
    }

    if (method == QLatin1String("client/unregisterCapability")) {
        if (m_dependencies.unregisterCapability)
            m_dependencies.unregisterCapability(
                message.value("params")
                    .toObject()
                    .value("unregisterations")
                    .toArray());
        sendResult(QJsonValue::Null);
        return;
    }

    sendError(-32601, QStringLiteral("Method not found: ") + method);
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
