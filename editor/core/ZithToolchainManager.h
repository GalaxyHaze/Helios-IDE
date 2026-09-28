#ifndef ZITHTOOLCHAINMANAGER_H
#define ZITHTOOLCHAINMANAGER_H

#include <QObject>

#include "ZithRuntimeCatalog.h"
#include "ZithRuntimeAssetDownloader.h"
#include "ZithRuntimeInstaller.h"
#include "ZithReleaseCatalog.h"
#include "ZithRuntimeOverrideResolver.h"

class QNetworkAccessManager;
class QNetworkReply;

class ZithToolchainManager : public QObject
{
    Q_OBJECT

public:
    explicit ZithToolchainManager(QObject *parent = nullptr);

    void ensureLatest(bool preferCached = true);
    void cancel();
    QString runtimeCacheRootPath() const;
    bool clearCachedRuntime(QString *errorMessage = nullptr) const;
    void setPreferOnline(bool preferOnline)
    {
        m_preferOnline = preferOnline;
    }
    void setCacheRootForTesting(const QString &path)
    {
        m_runtimeCatalog.setCacheRoot(path);
    }

signals:
    void statusChanged(const QString &message);
    void ready(const QString &lspPath, const QString &stdlibPath, const QString &tag);
    void failed(const QString &message);

private slots:
    void onLatestReleaseFinished();
    void onAssetDownloadFinished();
    void onAssetDownloadFailed(const QString &message);

private:
#ifdef HELIOS_UNIT_TESTING
    friend class TestHelios;
#endif

    using ReleaseAsset = ZithReleaseCatalog::Asset;
    using ReleaseInfo = ZithReleaseCatalog::Release;

    struct PendingDownload {
        ZithRuntimeAssetKind kind;
        ReleaseAsset asset;
        QString temporaryPath;
    };

    bool tryUseEnvironmentOverrides();
    void requestLatestRelease();
    void startNextDownload();
    void queueDownload(ZithRuntimeAssetKind kind, const ReleaseAsset &asset);
    void fallbackToInstalledRuntime(const QString &reason);
    void finishWithResolvedRuntime(const QString &lspPath,
                                   const QString &stdlibPath,
                                   const QString &tag);

    bool resolveInstalledRelease(const QString &tag,
                                 QString *lspPath,
                                 QString *stdlibPath) const;
    bool resolveNewestInstalledRelease(QString *lspPath,
                                       QString *stdlibPath,
                                       QString *tag) const;

    ZithRuntimeCatalog m_runtimeCatalog;
    ZithRuntimeInstaller m_runtimeInstaller;
    ZithRuntimeAssetDownloader *m_assetDownloader = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_latestReleaseReply = nullptr;
    QList<PendingDownload> m_pendingDownloads;
    QString m_pendingTag;
    bool m_preferCached = true;
    bool m_preferOnline = false;
    bool m_hasResolvedRuntime = false;
};

#endif
