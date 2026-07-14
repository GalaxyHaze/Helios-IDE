#ifndef ZITHTOOLCHAINMANAGER_H
#define ZITHTOOLCHAINMANAGER_H

#include <QObject>
#include <QList>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

class ZithToolchainManager : public QObject
{
    Q_OBJECT

public:
    explicit ZithToolchainManager(QObject *parent = nullptr);

    void ensureLatest(bool preferCached = true);
    QString runtimeCacheRootPath() const;
    bool clearCachedRuntime(QString *errorMessage = nullptr) const;

signals:
    void statusChanged(const QString &message);
    void ready(const QString &lspPath, const QString &stdlibPath, const QString &tag);
    void failed(const QString &message);

private slots:
    void onLatestReleaseFinished();
    void onAssetDownloadFinished();

private:
    enum class DownloadKind {
        LspBinary,
        StdlibArchive
    };

    struct ReleaseAsset {
        QString name;
        QUrl downloadUrl;
    };

    struct ReleaseInfo {
        QString tag;
        QList<ReleaseAsset> assets;
    };

    struct PendingDownload {
        DownloadKind kind;
        ReleaseAsset asset;
        QString temporaryPath;
    };

    bool tryUseEnvironmentOverrides();
    bool tryUseLocalDevelopmentOverrides();
    void requestLatestRelease();
    void startNextDownload();
    void queueDownload(DownloadKind kind, const ReleaseAsset &asset);
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
    bool installDownloadedAsset(const PendingDownload &download,
                                const QString &tag,
                                QString *errorMessage) const;
    bool extractArchive(const QString &archivePath,
                        const QString &destinationDir,
                        QString *errorMessage) const;
    bool runProcess(const QString &program,
                    const QStringList &arguments,
                    QString *errorMessage) const;
    bool ensureDirectory(const QString &path) const;

    QString cacheRootPath() const;
    QString releaseRootPath(const QString &tag) const;
    QString lspInstallPath(const QString &tag) const;
    QString stdlibInstallPath(const QString &tag) const;
    QString lspAssetNameForCurrentPlatform() const;
    QString stdlibAssetSuffixForCurrentPlatform() const;

    ReleaseAsset findLspAsset(const ReleaseInfo &release) const;
    ReleaseAsset findStdlibAsset(const ReleaseInfo &release) const;

    static bool isArm64Architecture();

    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_latestReleaseReply = nullptr;
    QNetworkReply *m_assetReply = nullptr;
    QList<PendingDownload> m_pendingDownloads;
    QString m_pendingTag;
    bool m_preferCached = true;
    bool m_hasResolvedRuntime = false;
};

#endif
