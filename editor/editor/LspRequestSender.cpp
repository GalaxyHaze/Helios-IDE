#include "LspRequestSender.h"

#include <utility>

LspRequestSender::LspRequestSender(LspRequestSenderCallbacks callbacks,
                                   int requestTimeoutMs)
    : m_callbacks(std::move(callbacks)),
      m_tracker(nullptr, requestTimeoutMs)
{
    QObject::connect(
        &m_tracker, &LspRequestTracker::requestTimedOut, &m_tracker,
        [this](qint64 id, const QString &method, const QString &uri) {
            if (m_callbacks.logMessage) {
                QString message =
                    QStringLiteral("LSP request timed out: %1 (id %2)")
                        .arg(method)
                        .arg(id);
                if (!uri.isEmpty())
                    message += QStringLiteral(" [") + uri +
                               QStringLiteral("]");
                m_callbacks.logMessage(message);
            }
            cancel(id);
        });
}

qint64 LspRequestSender::nextId()
{
    return ++m_nextId;
}

qint64 LspRequestSender::send(
    const QString &method, const QJsonObject &params, const QString &uri,
    int version, bool cancellable, bool ready,
    std::function<void(const QJsonObject &)> callback)
{
    if (method != QLatin1String("initialize") && !ready)
        return -1;

    const qint64 id = nextId();
    const LspPendingRequest request{id, method, uri, version, cancellable,
                                    std::move(callback)};
    const std::optional<qint64> replacedId = m_tracker.track(request);
    if (replacedId)
        sendCancellation(*replacedId);

    const QJsonObject message{
        {"jsonrpc", "2.0"}, {"id", id}, {"method", method}, {"params", params}};
    if (!m_callbacks.writeMessage || !m_callbacks.writeMessage(message)) {
        cancel(id);
        return -1;
    }
    return id;
}

void LspRequestSender::handleResponse(
    const QJsonObject &message,
    const std::function<bool(const QString &, int)> &isCurrentDocument)
{
    const qint64 id = message.value("id").toVariant().toLongLong();
    const std::optional<LspPendingRequest> request = m_tracker.take(id);
    if (!request)
        return;

    if (!request->uri.isEmpty() && isCurrentDocument &&
        !isCurrentDocument(request->uri, request->version))
        return;

    const QJsonObject error = message.value("error").toObject();
    if (!error.isEmpty()) {
        const int code = error.value("code").toInt();
        if (code != -32800 && code != -32801 && m_callbacks.logMessage) {
            m_callbacks.logMessage(
                QString("LSP %1 failed: %2")
                    .arg(request->method, error.value("message").toString()));
        }
        return;
    }

    if (request->callback)
        request->callback(message);
}

void LspRequestSender::cancel(qint64 id)
{
    if (!m_tracker.cancel(id))
        return;
    sendCancellation(id);
}

void LspRequestSender::cancelForUri(const QString &uri)
{
    const QList<qint64> ids = m_tracker.cancelForUri(uri);
    for (const qint64 id : ids)
        sendCancellation(id);
}

void LspRequestSender::cancelCancellableForUri(const QString &uri)
{
    const QList<qint64> ids = m_tracker.cancelCancellableForUri(uri);
    for (const qint64 id : ids)
        sendCancellation(id);
}

void LspRequestSender::clear()
{
    m_tracker.clear();
}

void LspRequestSender::sendCancellation(qint64 id)
{
    if (m_callbacks.isRunning && m_callbacks.isRunning() &&
        m_callbacks.writeMessage) {
        m_callbacks.writeMessage(
            {{"jsonrpc", "2.0"},
             {"method", "$/cancelRequest"},
             {"params", QJsonObject{{"id", id}}}});
    }
}
