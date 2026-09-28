#ifndef LSPEVENTSOURCE_H
#define LSPEVENTSOURCE_H

#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QObject>
#include <QString>

#include "../editor/LspTypes.h"

class LspEventSource : public QObject
{
    Q_OBJECT

public:
    explicit LspEventSource(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

signals:
    void initialized();
    void serverError(const QString &message);
    void serverStopped();
    void processStopped(bool expected);
    void diagnosticsReceived(const QString &uri, int version,
                             const QList<LspDiagnostic> &diagnostics);
    void saveAllRequested();
    void logMessage(const QString &message);
    void showMessage(const QString &message);
    void processOutputReceived(const QString &taskId, const QString &chunk);
    void processExitReceived(const QString &taskId, int exitCode);
    void workDoneProgressReceived(const QString &token, const QString &kind,
                                  const QString &message);
    void commandResult(const QString &command, bool success,
                       const QJsonValue &result);
    void renameResult(const QString &uri, int version,
                      const QJsonObject &edit);
};

#endif
