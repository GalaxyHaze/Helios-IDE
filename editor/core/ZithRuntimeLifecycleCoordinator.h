#ifndef ZITHRUNTIMELIFECYCLECOORDINATOR_H
#define ZITHRUNTIMELIFECYCLECOORDINATOR_H

#include <QJsonObject>
#include <QObject>
#include <QString>

#include "ZithRuntimeState.h"
#include "LspRestartPolicy.h"

class LspClient;
class ZithToolchainManager;

class ZithRuntimeLifecycleCoordinator : public QObject
{
    Q_OBJECT

public:
    explicit ZithRuntimeLifecycleCoordinator(QObject *parent = nullptr);

    LspClient *client() const;
    const ZithRuntimeState &state() const;
    QString runtimeCacheRootPath() const;

    void setWorkspaceRoot(const QString &workspaceRoot);
    void setEnabled(bool enabled);
    bool isEnabled() const;
    void ensureLatest(bool preferCached);
    void setPreferOnline(bool preferOnline);
    bool clearCachedRuntime(QString *errorMessage = nullptr);

#ifdef HELIOS_UNIT_TESTING
    void setCacheRootForTesting(const QString &path);
#endif

signals:
    void stateChanged();
    void connected();
    void stopped();
    void failed(const QString &message);
    void serverError(const QString &message);
    void frontendStatusChanged(const QJsonObject &status);
    void metricsReceived(const QJsonObject &metrics);

private:
    void startResolvedRuntime(const QString &lspPath,
                              const QString &stdlibPath,
                              const QString &tag);
    void setStatusText(const QString &text);
    void handleProcessStopped(bool expected);
    void handleFrontendStatus(const QJsonObject &status);

    LspClient *m_client = nullptr;
    ZithToolchainManager *m_toolchain = nullptr;
    ZithRuntimeState m_state;
    LspRestartPolicy m_restartPolicy;
    QString m_workspaceRoot;
    bool m_enabled = false;
};

#endif
