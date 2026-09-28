#include "ZithRuntimeLifecycleCoordinator.h"

#include <QDateTime>
#include <QTimer>

#include "../editor/LspClient.h"
#include "LspRestartPolicy.h"
#include "ZithToolchainManager.h"

namespace {
constexpr int kMillisecondsPerSecond = 1000;
}

ZithRuntimeLifecycleCoordinator::ZithRuntimeLifecycleCoordinator(
    QObject *parent)
    : QObject(parent), m_client(new LspClient(this)),
      m_toolchain(new ZithToolchainManager(this)) {
  connect(m_toolchain, &ZithToolchainManager::statusChanged, this,
          [this](const QString &message) { setStatusText(message); });
  connect(m_toolchain, &ZithToolchainManager::failed, this,
          [this](const QString &message) {
            setStatusText(message);
            emit failed(message);
          });
  connect(m_toolchain, &ZithToolchainManager::ready, this,
          [this](const QString &lspPath, const QString &stdlibPath,
                 const QString &tag) {
            startResolvedRuntime(lspPath, stdlibPath, tag);
          });

  connect(m_client, &LspClient::initialized, this, [this] {
    if (!m_enabled) {
      m_client->stop();
      return;
    }

    setStatusText(m_state.tag().isEmpty()
                      ? QStringLiteral("LSP connected.")
                      : QString("Connected to runtime %1.").arg(m_state.tag()));
    emit connected();
  });
  connect(m_client, &LspClient::serverStopped, this, [this] {
    setStatusText(m_enabled ? QStringLiteral("LSP server stopped.")
                            : QStringLiteral("Disabled"));
    emit stopped();
  });
  connect(m_client, &LspClient::processStopped, this,
          &ZithRuntimeLifecycleCoordinator::handleProcessStopped);
  connect(m_client, &LspClient::serverError, this,
          [this](const QString &message) {
            if (!m_enabled) {
              return;
            }
            setStatusText("LSP error: " + message);
            emit serverError(message);
          });
  connect(m_client, &LspClient::frontendStatusReceived, this,
          &ZithRuntimeLifecycleCoordinator::handleFrontendStatus);
  connect(m_client, &LspClient::metricsReceived, this,
          &ZithRuntimeLifecycleCoordinator::metricsReceived);
}

LspClient *ZithRuntimeLifecycleCoordinator::client() const { return m_client; }

const ZithRuntimeState &ZithRuntimeLifecycleCoordinator::state() const {
  return m_state;
}

QString ZithRuntimeLifecycleCoordinator::runtimeCacheRootPath() const {
  return m_toolchain->runtimeCacheRootPath();
}

void ZithRuntimeLifecycleCoordinator::setWorkspaceRoot(
    const QString &workspaceRoot) {
  m_workspaceRoot = workspaceRoot;
}

void ZithRuntimeLifecycleCoordinator::setEnabled(bool enabled) {
  m_enabled = enabled;
  if (m_enabled) {
    return;
  }

  m_toolchain->cancel();
  m_client->stop();
  m_state.clearRuntime();
  setStatusText(QStringLiteral("Disabled"));
}

bool ZithRuntimeLifecycleCoordinator::isEnabled() const { return m_enabled; }

void ZithRuntimeLifecycleCoordinator::ensureLatest(bool preferCached) {
  if (!m_enabled) {
    return;
  }

  setStatusText(preferCached
                    ? QStringLiteral("Resolving latest Zith runtime...")
                    : QStringLiteral("Refreshing Zith runtime..."));
  m_toolchain->ensureLatest(preferCached);
}

void ZithRuntimeLifecycleCoordinator::setPreferOnline(bool preferOnline) {
  m_toolchain->setPreferOnline(preferOnline);
}

bool ZithRuntimeLifecycleCoordinator::clearCachedRuntime(
    QString *errorMessage) {
  if (!m_enabled) {
    return true;
  }

  m_toolchain->cancel();
  m_client->stop();
  if (!m_toolchain->clearCachedRuntime(errorMessage)) {
    setStatusText(errorMessage != nullptr
                      ? *errorMessage
                      : QStringLiteral("Failed to clear runtime cache."));
    emit failed(m_state.statusText());
    return false;
  }

  m_state.clearRuntime();
  setStatusText(QStringLiteral(
      "Runtime cache cleared. Resolving latest Zith runtime..."));
  return true;
}

#ifdef HELIOS_UNIT_TESTING
void ZithRuntimeLifecycleCoordinator::setCacheRootForTesting(
    const QString &path) {
  m_toolchain->setCacheRootForTesting(path);
}
#endif

void ZithRuntimeLifecycleCoordinator::startResolvedRuntime(
    const QString &lspPath, const QString &stdlibPath, const QString &tag) {
  if (!m_enabled) {
    return;
  }

  if (m_client->isRunning() &&
      m_state.matches(lspPath, stdlibPath, m_workspaceRoot)) {
    m_state.activate(lspPath, stdlibPath, tag, m_workspaceRoot);
    setStatusText(QString("Runtime %1 already active.").arg(tag));
    return;
  }

  m_toolchain->cancel();
  if (m_client->isRunning()) {
    m_client->stop();
  }

  m_state.activate(lspPath, stdlibPath, tag, m_workspaceRoot);
  setStatusText(QString("Starting runtime %1...").arg(tag));
  if (!m_client->start(
          LspStartOptions{lspPath, stdlibPath, m_workspaceRoot, {}})) {
    setStatusText(QStringLiteral("Failed to start the resolved Zith runtime."));
    emit failed(QStringLiteral("Failed to start the resolved Zith runtime."));
  }
}

void ZithRuntimeLifecycleCoordinator::setStatusText(const QString &text) {
  m_state.setStatusText(text);
  emit stateChanged();
}

void ZithRuntimeLifecycleCoordinator::handleProcessStopped(bool expected) {
  const LspRestartPolicy::Decision decision =
      m_restartPolicy.decide(QDateTime::currentMSecsSinceEpoch(), expected,
                             m_enabled, !m_state.lspPath().isEmpty());
  if (decision.action == LspRestartPolicy::Action::Ignore) {
    return;
  }

  if (decision.action == LspRestartPolicy::Action::GiveUp) {
    setStatusText(
        QStringLiteral("LSP crashed repeatedly; automatic restart stopped."));
    emit failed(m_state.statusText());
    return;
  }

  setStatusText(QString("LSP stopped; restarting in %1 second(s)...")
                    .arg(decision.delaySeconds));
  QTimer::singleShot(
      decision.delaySeconds * kMillisecondsPerSecond, this, [this] {
        if (m_enabled && !m_client->isRunning()) {
          m_client->start(LspStartOptions{m_state.lspPath(),
                                          m_state.stdlibPath(),
                                          m_state.workspaceRoot(),
                                          {}});
        }
      });
}

void ZithRuntimeLifecycleCoordinator::handleFrontendStatus(
    const QJsonObject &status) {
  const QString state = status.value(QStringLiteral("state")).toString();
  const QString message = status.value(QStringLiteral("message")).toString();
  if (state == QStringLiteral("warming")) {
    setStatusText(QStringLiteral("Frontend warming up..."));
  } else if (state == QStringLiteral("ready")) {
    setStatusText(QStringLiteral("Frontend ready"));
  } else if (state == QStringLiteral("error")) {
    setStatusText("Frontend error: " + message);
  }

  emit frontendStatusChanged(status);
}
