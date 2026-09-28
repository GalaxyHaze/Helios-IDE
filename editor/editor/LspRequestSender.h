#ifndef LSPREQUESTSENDER_H
#define LSPREQUESTSENDER_H

#include "LspRequestTracker.h"

#include <QJsonObject>

#include <functional>
#include <optional>

struct LspRequestSenderCallbacks
{
    std::function<bool(const QJsonObject &)> writeMessage;
    std::function<bool()> isRunning;
    std::function<void(const QString &)> logMessage;
};

class LspRequestSender
{
public:
    explicit LspRequestSender(LspRequestSenderCallbacks callbacks,
                              int requestTimeoutMs = 8000);

    qint64 send(const QString &method, const QJsonObject &params,
                const QString &uri, int version, bool cancellable,
                bool ready,
                std::function<void(const QJsonObject &)> callback);
    void handleResponse(
        const QJsonObject &message,
        const std::function<bool(const QString &, int)> &isCurrentDocument);
    void cancel(qint64 id);
    void cancelForUri(const QString &uri);
    void cancelCancellableForUri(const QString &uri);
    void clear();

private:
    qint64 nextId();
    void sendCancellation(qint64 id);

    LspRequestSenderCallbacks m_callbacks;
    LspRequestTracker m_tracker;
    qint64 m_nextId = 0;
};

#endif
