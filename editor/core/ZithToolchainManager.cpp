#include "ZithToolchainManager.h"

#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace
{
constexpr auto kLatestReleaseUrl =
    "https://api.github.com/repos/GalaxyHaze/Zith-Lang/releases/latest";
constexpr auto kLspOverrideEnv = "HELIOS_ZITH_LSP_PATH";
constexpr auto kStdlibOverrideEnv = "HELIOS_ZITH_STDLIB_PATH";
}

ZithToolchainManager::ZithToolchainManager(QObject *parent)
    : QObject(parent),
      m_runtimeInstaller(m_runtimeCatalog),
      m_assetDownloader(new ZithRuntimeAssetDownloader(this)),
      m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_assetDownloader, &ZithRuntimeAssetDownloader::finished, this,
            &ZithToolchainManager::onAssetDownloadFinished);
    connect(m_assetDownloader, &ZithRuntimeAssetDownloader::failed, this,
            &ZithToolchainManager::onAssetDownloadFailed);
}

void ZithToolchainManager::ensureLatest(bool preferCached)
{
    m_preferCached = preferCached;
    cancel();

    if (tryUseEnvironmentOverrides())
        return;

    const QFileInfo localCache(
        m_runtimeCatalog.releaseRootPath(QStringLiteral("local")));
    if (localCache.exists()) {
        emit statusChanged(QStringLiteral("Ignoring outdated local runtime cache."));
        QString staleError;
        if (!m_runtimeCatalog.removeStaleLocalRuntimeCache(&staleError) &&
            !staleError.isEmpty()) {
            emit statusChanged(staleError);
        }
    }

    if (preferCached) {
        QString lspPath;
        QString stdlibPath;
        QString tag;
        if (resolveNewestInstalledRelease(&lspPath, &stdlibPath, &tag)) {
            emit statusChanged(QString("Using cached Zith runtime %1 while checking for updates.")
                                   .arg(tag));
            finishWithResolvedRuntime(lspPath, stdlibPath, tag);
        }
    }

    emit statusChanged(QStringLiteral("Resolving latest Zith runtime..."));
    requestLatestRelease();
}

void ZithToolchainManager::cancel()
{
    m_hasResolvedRuntime = false;
    m_pendingDownloads.clear();
    m_pendingTag.clear();

    if (m_latestReleaseReply) {
        QObject::disconnect(m_latestReleaseReply, nullptr, this, nullptr);
        m_latestReleaseReply->abort();
        m_latestReleaseReply->deleteLater();
        m_latestReleaseReply = nullptr;
    }

    m_assetDownloader->cancel();
}

QString ZithToolchainManager::runtimeCacheRootPath() const
{
    return m_runtimeCatalog.cacheRootPath();
}

bool ZithToolchainManager::clearCachedRuntime(QString *errorMessage) const
{
    const QString root = m_runtimeCatalog.cacheRootPath();
    const QFileInfo rootInfo(root);
    if (!rootInfo.exists()) {
        return true;
    }

    QDir rootDir(root);
    if (!rootDir.removeRecursively()) {
        if (errorMessage != nullptr) {
            *errorMessage = "Failed to remove the cached Zith runtime directory.";
        }
        return false;
    }

    return true;
}

void ZithToolchainManager::onLatestReleaseFinished()
{
    QNetworkReply *reply = m_latestReleaseReply;
    m_latestReleaseReply = nullptr;

    if (!reply)
        return;

    const QByteArray payload = reply->readAll();
    const QString errorString = reply->error() == QNetworkReply::NoError
        ? QString()
        : reply->errorString();
    reply->deleteLater();

    if (!errorString.isEmpty()) {
        fallbackToInstalledRuntime(
            "Failed to query the latest Zith release: " + errorString);
        return;
    }

    QString parseError;
    const auto release = ZithReleaseCatalog::parse(payload, &parseError);
    if (!release) {
        fallbackToInstalledRuntime(parseError);
        return;
    }

    QString lspPath;
    QString stdlibPath;
    if (resolveInstalledRelease(release->tag, &lspPath, &stdlibPath)) {
        emit statusChanged(QString("Zith runtime %1 is already installed.").arg(release->tag));
        if (!m_hasResolvedRuntime || !m_preferCached)
            finishWithResolvedRuntime(lspPath, stdlibPath, release->tag);
        return;
    }

    const auto lspAsset = ZithReleaseCatalog::findLspAsset(*release);
    const auto stdlibAsset = ZithReleaseCatalog::findStdlibAsset(*release);
    if (!lspAsset || !stdlibAsset) {
        fallbackToInstalledRuntime(
            QString("Latest Zith release %1 is missing a compatible LSP or stdlib asset.")
                .arg(release->tag));
        return;
    }

    m_pendingTag = release->tag;
    queueDownload(ZithRuntimeAssetKind::LspBinary, *lspAsset);
    queueDownload(ZithRuntimeAssetKind::StdlibArchive, *stdlibAsset);

    emit statusChanged(QString("Downloading Zith runtime %1...").arg(release->tag));
    startNextDownload();
}

void ZithToolchainManager::onAssetDownloadFinished()
{
    if (m_pendingDownloads.isEmpty())
        return;

    PendingDownload current = m_pendingDownloads.takeFirst();

    QString installError;
    if (!m_runtimeInstaller.install(
            {current.kind, current.temporaryPath, m_pendingTag},
            &installError)) {
        QFile::remove(current.temporaryPath);
        fallbackToInstalledRuntime(installError);
        return;
    }

    QFile::remove(current.temporaryPath);

    if (!m_pendingDownloads.isEmpty()) {
        startNextDownload();
        return;
    }

    QString lspPath;
    QString stdlibPath;
    if (!resolveInstalledRelease(m_pendingTag, &lspPath, &stdlibPath)) {
        fallbackToInstalledRuntime("Downloaded Zith runtime could not be resolved from cache.");
        return;
    }

    emit statusChanged(QString("Installed Zith runtime %1.").arg(m_pendingTag));
    if (!m_hasResolvedRuntime || !m_preferCached) {
        finishWithResolvedRuntime(lspPath, stdlibPath, m_pendingTag);
    } else {
        emit statusChanged(
            QString("Zith runtime %1 was downloaded. Use Restart LSP to switch to it.")
                .arg(m_pendingTag));
    }
}

void ZithToolchainManager::onAssetDownloadFailed(const QString &message)
{
    if (m_pendingDownloads.isEmpty())
        return;

    QFile::remove(m_pendingDownloads.first().temporaryPath);
    m_pendingDownloads.clear();
    fallbackToInstalledRuntime(message);
}

bool ZithToolchainManager::tryUseEnvironmentOverrides()
{
    const auto result = ZithRuntimeOverrideResolver::resolve(
        QString::fromLocal8Bit(qgetenv(kLspOverrideEnv)),
        QString::fromLocal8Bit(qgetenv(kStdlibOverrideEnv)));

    if (result.action == ZithRuntimeOverrideResolver::Action::NotConfigured)
        return false;

    if (result.action ==
        ZithRuntimeOverrideResolver::Action::IgnoreAndContinue) {
        emit statusChanged(result.message);
        return false;
    }

    if (result.action == ZithRuntimeOverrideResolver::Action::Fail) {
        emit failed(result.message);
        return true;
    }

    emit statusChanged(result.message);
    finishWithResolvedRuntime(result.runtime.lspPath,
                              result.runtime.stdlibPath, "environment");
    return true;
}

void ZithToolchainManager::requestLatestRelease()
{
    QNetworkRequest request(QUrl(QString::fromLatin1(kLatestReleaseUrl)));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Helios"));
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    request.setTransferTimeout(5000);

    m_latestReleaseReply = m_networkManager->get(request);
    connect(m_latestReleaseReply, &QNetworkReply::finished,
            this, &ZithToolchainManager::onLatestReleaseFinished);
}

void ZithToolchainManager::startNextDownload()
{
    if (m_pendingDownloads.isEmpty())
        return;

    const PendingDownload &current = m_pendingDownloads.first();

    m_assetDownloader->start({current.asset.downloadUrl, current.asset.name,
                              current.temporaryPath});
}

void ZithToolchainManager::queueDownload(ZithRuntimeAssetKind kind,
                                         const ReleaseAsset &asset)
{
    const QString tempRoot =
        QDir(m_runtimeCatalog.cacheRootPath()).filePath("downloads");
    QDir().mkpath(tempRoot);

    PendingDownload download;
    download.kind = kind;
    download.asset = asset;
    download.temporaryPath = QDir(tempRoot).filePath(asset.name);
    m_pendingDownloads.append(download);
}

void ZithToolchainManager::fallbackToInstalledRuntime(const QString &reason)
{
    QString lspPath;
    QString stdlibPath;
    QString tag;
    if (resolveNewestInstalledRelease(&lspPath, &stdlibPath, &tag)) {
        emit statusChanged(reason + QString(" Falling back to cached runtime %1.").arg(tag));
        if (!m_hasResolvedRuntime)
            finishWithResolvedRuntime(lspPath, stdlibPath, tag);
        return;
    }

    emit failed(reason);
}

void ZithToolchainManager::finishWithResolvedRuntime(const QString &lspPath,
                                                     const QString &stdlibPath,
                                                     const QString &tag)
{
    m_hasResolvedRuntime = true;
    emit ready(lspPath, stdlibPath, tag);
}

bool ZithToolchainManager::resolveInstalledRelease(
    const QString &tag,
    QString *lspPath,
    QString *stdlibPath) const
{
    const auto resolved = m_runtimeCatalog.resolveInstalledRelease(tag);
    if (!resolved)
        return false;
    if (lspPath)
        *lspPath = resolved->lspPath;
    if (stdlibPath)
        *stdlibPath = resolved->stdlibPath;
    return true;
}

bool ZithToolchainManager::resolveNewestInstalledRelease(
    QString *lspPath,
    QString *stdlibPath,
    QString *tag) const
{
    const auto resolved = m_runtimeCatalog.resolveNewestInstalledRelease();
    if (!resolved)
        return false;
    if (lspPath)
        *lspPath = resolved->paths.lspPath;
    if (stdlibPath)
        *stdlibPath = resolved->paths.stdlibPath;
    if (tag)
        *tag = resolved->tag;
    return true;
}
