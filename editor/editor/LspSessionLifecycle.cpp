#include "LspSessionLifecycle.h"

#include "LspInitializationBuilder.h"

#include <QTimer>

#include <utility>

LspSessionLifecycle::LspSessionLifecycle(LspSessionLifecycleCallbacks callbacks,
                                         QObject *parent)
    : QObject(parent), m_callbacks(std::move(callbacks)) {}

bool LspSessionLifecycle::start(const LspStartOptions &options,
                                bool processAlreadyExists) {
  m_options = options;
  if (processAlreadyExists) {
    m_startPending = true;
    stop();
    return true;
  }

  launch();
  return true;
}

void LspSessionLifecycle::launch() {
  resetSession();
  if (!m_callbacks.startProcess ||
      !m_callbacks.startProcess(m_options.serverPath)) {
    emit serverError(QStringLiteral("failed to start LSP process"));
  }
}

void LspSessionLifecycle::resetSession() {
  m_initialized = false;
  m_shutdownRequested = false;
  m_exitSent = false;
  m_stopping = false;
  if (m_callbacks.clearSessionData)
    m_callbacks.clearSessionData();
  m_shutdownEscalation.reset();
}

void LspSessionLifecycle::stop() {
  if (!m_callbacks.hasProcess || !m_callbacks.hasProcess() || m_stopping)
    return;

  m_stopping = true;
  if (!m_callbacks.isRunning || !m_callbacks.isRunning())
    return;

  if (m_initialized)
    beginShutdown();
  else {
    if (m_callbacks.sendMessage)
      m_callbacks.sendMessage({{"jsonrpc", "2.0"}, {"method", "exit"}});
    scheduleShutdownEscalation();
  }
}

void LspSessionLifecycle::beginShutdown() {
  if (m_shutdownRequested)
    return;

  m_shutdownRequested = true;
  if (!m_callbacks.sendRequest)
    return;

  m_callbacks.sendRequest(
      QStringLiteral("shutdown"), QJsonObject{}, QString(), -1, false,
      [this](const QJsonObject &) {
        if (!m_callbacks.hasProcess || !m_callbacks.hasProcess() || m_exitSent)
          return;

        m_exitSent = true;
        if (m_callbacks.sendMessage)
          m_callbacks.sendMessage({{"jsonrpc", "2.0"}, {"method", "exit"}});
        scheduleShutdownEscalation();
      });
}

void LspSessionLifecycle::scheduleShutdownEscalation() {
  if (!m_callbacks.hasProcess || !m_callbacks.hasProcess())
    return;

  if (!m_shutdownTimer) {
    m_shutdownTimer = new QTimer(this);
    m_shutdownTimer->setSingleShot(true);
    connect(m_shutdownTimer, &QTimer::timeout, this, [this]() {
      if (!m_callbacks.isRunning || !m_callbacks.isRunning())
        return;

      switch (m_shutdownEscalation.nextAction(true)) {
      case LspShutdownEscalation::Action::Terminate:
        if (m_callbacks.terminateProcess)
          m_callbacks.terminateProcess();
        m_shutdownTimer->start(500);
        break;
      case LspShutdownEscalation::Action::Kill:
        if (m_callbacks.killProcess)
          m_callbacks.killProcess();
        break;
      case LspShutdownEscalation::Action::None:
        break;
      }
    });
  }
  m_shutdownTimer->start(1000);
}

void LspSessionLifecycle::processStarted() {
  const QString initMode = m_options.initMode == QLatin1String("clangd")
                               ? QStringLiteral("clangd")
                               : QStringLiteral("zith");
  const QJsonObject params = LspInitializationBuilder::build(
      {m_options.workspaceRoot, m_options.stdlibPath, initMode});

  if (!m_callbacks.sendRequest)
    return;
  m_callbacks.sendRequest(
      QStringLiteral("initialize"), params, QString(), -1, false,
      [this](const QJsonObject &response) {
        if (response.contains("error")) {
          const QJsonObject error = response.value("error").toObject();
          emit serverError(error.value("message").toString(
              QStringLiteral("initialize failed")));
          return;
        }

        if (m_callbacks.parseCapabilities)
          m_callbacks.parseCapabilities(response.value("result")
                                            .toObject()
                                            .value("capabilities")
                                            .toObject());
        if (!m_callbacks.sendMessage ||
            !m_callbacks.sendMessage({{"jsonrpc", "2.0"},
                                      {"method", "initialized"},
                                      {"params", QJsonObject{}}})) {
          emit serverError(
              QStringLiteral("failed to send LSP initialized notification"));
          return;
        }

        m_initialized = true;
        emit initialized();
      });
}

void LspSessionLifecycle::processFinished(const LspProcessResult &,
                                          const QString &unexpectedError) {
  const bool expected = m_stopping;
  const bool wasInitialized = m_initialized;
  if (m_shutdownTimer)
    m_shutdownTimer->stop();

  if (!expected && !unexpectedError.isEmpty())
    emit serverError(unexpectedError);

  if (m_callbacks.clearSessionData)
    m_callbacks.clearSessionData();
  m_initialized = false;
  m_shutdownRequested = false;
  m_exitSent = false;
  m_stopping = false;
  m_shutdownEscalation.reset();

  if (wasInitialized)
    emit serverStopped();
  emit processStopped(expected);

  if (m_startPending) {
    m_startPending = false;
    launch();
  }
}

bool LspSessionLifecycle::isReady() const {
  return m_initialized && m_callbacks.isRunning && m_callbacks.isRunning();
}

#ifdef HELIOS_UNIT_TESTING
void LspSessionLifecycle::setReadyForTesting(bool ready) {
  m_initialized = ready;
}
#endif
