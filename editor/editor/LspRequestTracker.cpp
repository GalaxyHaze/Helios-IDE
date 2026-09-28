#include "LspRequestTracker.h"

LspRequestTracker::LspRequestTracker(QObject *parent, int timeoutMs)
    : QObject(parent), m_timeoutMs(timeoutMs)
{
}

std::optional<qint64> LspRequestTracker::track(LspPendingRequest request)
{
    std::optional<qint64> replacedId;
    if (request.cancellable && !request.uri.isEmpty()) {
        const QString key = request.method + QLatin1Char('\x1f') + request.uri;
        if (m_replaceableRequests.contains(key)) {
            replacedId = m_replaceableRequests.value(key);
            cancel(*replacedId);
        }
        m_replaceableRequests.insert(key, request.id);
    }

    request.timeout = new QTimer(this);
    request.timeout->setSingleShot(true);
    const qint64 id = request.id;
    connect(request.timeout, &QTimer::timeout, this, [this, id]() {
        const auto it = m_requests.constFind(id);
        if (it == m_requests.constEnd())
            return;
        emit requestTimedOut(id, it->method, it->uri);
    });
    m_requests.insert(request.id, request);
    request.timeout->start(m_timeoutMs);
    return replacedId;
}

std::optional<LspPendingRequest> LspRequestTracker::take(qint64 id)
{
    auto it = m_requests.find(id);
    if (it == m_requests.end())
        return std::nullopt;

    LspPendingRequest request = it.value();
    disposeTimer(request);
    removeFromIndexes(request);
    m_requests.erase(it);
    return request;
}

std::optional<LspPendingRequest> LspRequestTracker::cancel(qint64 id)
{
    return take(id);
}

QList<qint64> LspRequestTracker::cancelForUri(const QString &uri)
{
    QList<qint64> ids;
    for (auto it = m_requests.cbegin(); it != m_requests.cend(); ++it) {
        if (it->uri == uri)
            ids.append(it.key());
    }
    for (const qint64 id : ids)
        cancel(id);
    return ids;
}

QList<qint64> LspRequestTracker::cancelCancellableForUri(
    const QString &uri)
{
    QList<qint64> ids;
    for (auto it = m_requests.cbegin(); it != m_requests.cend(); ++it) {
        if (it->uri == uri && it->cancellable)
            ids.append(it.key());
    }
    for (const qint64 id : ids)
        cancel(id);
    return ids;
}

void LspRequestTracker::clear()
{
    for (auto it = m_requests.begin(); it != m_requests.end(); ++it)
        disposeTimer(it.value());
    m_requests.clear();
    m_replaceableRequests.clear();
}

void LspRequestTracker::removeFromIndexes(
    const LspPendingRequest &request)
{
    if (!request.cancellable || request.uri.isEmpty())
        return;

    const QString key = request.method + QLatin1Char('\x1f') + request.uri;
    if (m_replaceableRequests.value(key) == request.id)
        m_replaceableRequests.remove(key);
}

void LspRequestTracker::disposeTimer(LspPendingRequest &request) const
{
    if (request.timeout != nullptr) {
        request.timeout->stop();
        request.timeout->deleteLater();
        request.timeout = nullptr;
    }
}
