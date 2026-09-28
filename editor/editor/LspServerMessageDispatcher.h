#ifndef LSPSERVERMESSAGEDISPATCHER_H
#define LSPSERVERMESSAGEDISPATCHER_H

#include <QJsonObject>
#include <QObject>
#include <QString>

#include <functional>

#include "LspTypes.h"

struct LspServerMessageDispatcherDependencies
{
    std::function<bool(const QJsonObject &)> sendMessage;
    std::function<bool()> isRunning;
    std::function<bool(const QString &, int)> isCurrentDocument;
};

class LspServerMessageDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit LspServerMessageDispatcher(
        LspServerMessageDispatcherDependencies dependencies,
        QObject *parent = nullptr);

    void dispatch(const QJsonObject &message);

signals:
    void diagnosticsReceived(const QString &uri, int version,
                             const QList<LspDiagnostic> &diagnostics);
    void logMessage(const QString &message);
    void showMessage(const QString &message);
    void saveAllRequested();
    void processOutputReceived(const QString &taskId, const QString &chunk);
    void processExitReceived(const QString &taskId, int exitCode);
    void workDoneProgressReceived(const QString &token, const QString &kind,
                                  const QString &message);
    void frontendStatusReceived(const QJsonObject &status);
    void metricsReceived(const QJsonObject &metrics);

private:
    void handleServerRequest(const QJsonObject &message);
    void handleNotification(const QJsonObject &message);

    LspServerMessageDispatcherDependencies m_dependencies;
};

#endif
