#ifndef LSPREQUESTTRACKER_H
#define LSPREQUESTTRACKER_H

#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTimer>

#include <functional>
#include <optional>

struct LspPendingRequest
{
    qint64 id = 0;
    QString method;
    QString uri;
    int version = -1;
    bool cancellable = false;
    std::function<void(const QJsonObject &)> callback;
    QTimer *timeout = nullptr;
};

class LspRequestTracker : public QObject
{
    Q_OBJECT

public:
    explicit LspRequestTracker(QObject *parent = nullptr,
                               int timeoutMs = 8000);

    // Returns the id of a replaced request, if the new request superseded one.
    std::optional<qint64> track(LspPendingRequest request);
    std::optional<LspPendingRequest> take(qint64 id);
    std::optional<LspPendingRequest> cancel(qint64 id);
    QList<qint64> cancelForUri(const QString &uri);
    QList<qint64> cancelCancellableForUri(const QString &uri);
    void clear();

signals:
    void requestTimedOut(qint64 id, const QString &method,
                         const QString &uri);

private:
    void removeFromIndexes(const LspPendingRequest &request);
    void disposeTimer(LspPendingRequest &request) const;

    QHash<qint64, LspPendingRequest> m_requests;
    QHash<QString, qint64> m_replaceableRequests;
    int m_timeoutMs = 8000;
};

#endif
