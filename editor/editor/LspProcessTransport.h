#ifndef LSPPROCESSTRANSPORT_H
#define LSPPROCESSTRANSPORT_H

#include "LspProtocolCodec.h"

#include <QByteArray>
#include <QJsonObject>
#include <QObject>
#include <QString>

class QProcess;

struct LspProcessResult
{
    int exitCode = -1;
    bool crashed = false;
};

class LspProcessTransport : public QObject
{
    Q_OBJECT

public:
    explicit LspProcessTransport(QObject *parent = nullptr);
    ~LspProcessTransport() override;

#ifdef HELIOS_UNIT_TESTING
    void waitForFinishedForTesting(int timeoutMs);
#endif

    bool start(const QString &program);
    bool writeMessage(const QJsonObject &message);
    bool hasProcess() const { return m_process != nullptr; }
    bool isRunning() const;
    void terminate();
    void kill();

signals:
    void started();
    void messageReceived(const QJsonObject &message);
    void decodeError(const QString &message);
    void inputTooLarge();
    void protocolError(const QString &message);
    void stderrChunk(const QByteArray &chunk);
    void processError(const QString &message);
    void finished(const LspProcessResult &result);

private slots:
    void onReadyRead();
    void onReadyReadError();
    void onProcessStarted();
    void onProcessError();
    void onProcessFinished(int exitCode, bool crashed);

private:
    void finish(int exitCode, bool crashed);

    QProcess *m_process = nullptr;
    LspProtocolCodec m_protocolCodec;
    bool m_finishEmitted = false;
};

Q_DECLARE_METATYPE(LspProcessResult)

#endif
