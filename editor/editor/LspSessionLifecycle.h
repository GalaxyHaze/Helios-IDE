#ifndef LSPSESSIONLIFECYCLE_H
#define LSPSESSIONLIFECYCLE_H

#include "LspProcessTransport.h"
#include "LspShutdownEscalation.h"
#include "LspTypes.h"

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTimer>

#include <functional>

struct LspSessionLifecycleCallbacks {
  std::function<bool(const QJsonObject &)> sendMessage;
  std::function<qint64(const QString &, const QJsonObject &, const QString &,
                       int, bool, std::function<void(const QJsonObject &)>)>
      sendRequest;
  std::function<bool()> hasProcess;
  std::function<bool()> isRunning;
  std::function<bool(const QString &)> startProcess;
  std::function<void()> terminateProcess;
  std::function<void()> killProcess;
  std::function<void()> clearSessionData;
  std::function<void(const QJsonObject &)> parseCapabilities;
};

class LspSessionLifecycle : public QObject {
  Q_OBJECT

public:
  explicit LspSessionLifecycle(LspSessionLifecycleCallbacks callbacks,
                               QObject *parent = nullptr);

  bool start(const LspStartOptions &options, bool processAlreadyExists);
  void stop();
  void processStarted();
  void processFinished(const LspProcessResult &result,
                       const QString &unexpectedError);

  bool isReady() const;

#ifdef HELIOS_UNIT_TESTING
  void setReadyForTesting(bool ready);
#endif

signals:
  void initialized();
  void serverError(const QString &message);
  void serverStopped();
  void processStopped(bool expected);

private:
  void launch();
  void resetSession();
  void beginShutdown();
  void scheduleShutdownEscalation();

  LspSessionLifecycleCallbacks m_callbacks;
  LspStartOptions m_options;
  bool m_initialized = false;
  bool m_shutdownRequested = false;
  bool m_exitSent = false;
  bool m_stopping = false;
  bool m_startPending = false;
  QTimer *m_shutdownTimer = nullptr;
  LspShutdownEscalation m_shutdownEscalation;
};

#endif
