#include "ZithToolchainManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QStandardPaths>
#include <QSysInfo>
#include <QUrl>
#include <QVersionNumber>

namespace
{
constexpr int kNetworkTimeoutMs = 5000;
constexpr auto kLatestReleaseUrl =
    "https://api.github.com/repos/GalaxyHaze/Zith-Lang/releases/latest";
constexpr auto kLspOverrideEnv = "HELIOS_ZITH_LSP_PATH";
constexpr auto kStdlibOverrideEnv = "HELIOS_ZITH_STDLIB_PATH";

QVersionNumber releaseVersion(const QString &tag)
{
    QString normalized = tag.trimmed();
    if (normalized.startsWith('v'))
        normalized.remove(0, 1);
    return QVersionNumber::fromString(normalized);
}
}

ZithToolchainManager::ZithToolchainManager(QObject *parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
}

void ZithToolchainManager::ensureLatest(bool preferCached)
{
    m_preferCached = preferCached;
    cancel();

    if (tryUseEnvironmentOverrides())
        return;

    if (tryUseLocalDevelopmentOverrides())
        return;

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

    if (m_assetReply) {
        QObject::disconnect(m_assetReply, nullptr, this, nullptr);
        m_assetReply->abort();
        m_assetReply->deleteLater();
        m_assetReply = nullptr;
    }
}

QString ZithToolchainManager::runtimeCacheRootPath() const
{
    return cacheRootPath();
}

bool ZithToolchainManager::clearCachedRuntime(QString *errorMessage) const
{
    const QString root = cacheRootPath();
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

    const QJsonDocument document = QJsonDocument::fromJson(payload);
    if (!document.isObject()) {
        fallbackToInstalledRuntime("Latest Zith release response was not valid JSON.");
        return;
    }

    const QJsonObject root = document.object();
    ReleaseInfo release;
    release.tag = root.value("tag_name").toString().trimmed();
    const QJsonArray assets = root.value("assets").toArray();
    for (const QJsonValue &assetValue : assets) {
        const QJsonObject assetObject = assetValue.toObject();
        const QString name = assetObject.value("name").toString();
        const QString downloadUrl = assetObject.value("browser_download_url").toString();
        if (name.isEmpty() || downloadUrl.isEmpty())
            continue;
        release.assets.append({name, QUrl(downloadUrl)});
    }

    if (release.tag.isEmpty()) {
        fallbackToInstalledRuntime("Latest Zith release did not expose a tag.");
        return;
    }

    QString lspPath;
    QString stdlibPath;
    if (resolveInstalledRelease(release.tag, &lspPath, &stdlibPath)) {
        emit statusChanged(QString("Zith runtime %1 is already installed.").arg(release.tag));
        if (!m_hasResolvedRuntime || !m_preferCached)
            finishWithResolvedRuntime(lspPath, stdlibPath, release.tag);
        return;
    }

    const ReleaseAsset lspAsset = findLspAsset(release);
    const ReleaseAsset stdlibAsset = findStdlibAsset(release);
    if (lspAsset.name.isEmpty() || stdlibAsset.name.isEmpty()) {
        fallbackToInstalledRuntime(
            QString("Latest Zith release %1 is missing a compatible LSP or stdlib asset.")
                .arg(release.tag));
        return;
    }

    m_pendingTag = release.tag;
    queueDownload(DownloadKind::LspBinary, lspAsset);
    queueDownload(DownloadKind::StdlibArchive, stdlibAsset);

    emit statusChanged(QString("Downloading Zith runtime %1...").arg(release.tag));
    startNextDownload();
}

void ZithToolchainManager::onAssetDownloadFinished()
{
    QNetworkReply *reply = m_assetReply;
    m_assetReply = nullptr;

    if (!reply || m_pendingDownloads.isEmpty())
        return;

    PendingDownload current = m_pendingDownloads.takeFirst();
    const QByteArray payload = reply->readAll();
    const QString errorString = reply->error() == QNetworkReply::NoError
        ? QString()
        : reply->errorString();
    reply->deleteLater();

    if (!errorString.isEmpty()) {
        QFile::remove(current.temporaryPath);
        fallbackToInstalledRuntime("Failed to download " + current.asset.name + ": " + errorString);
        return;
    }

    QFile file(current.temporaryPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        fallbackToInstalledRuntime("Failed to write " + current.asset.name + " to cache.");
        return;
    }

    if (file.write(payload) != payload.size()) {
        file.close();
        QFile::remove(current.temporaryPath);
        fallbackToInstalledRuntime("Incomplete write while caching " + current.asset.name + ".");
        return;
    }
    file.close();

    QString installError;
    if (!installDownloadedAsset(current, m_pendingTag, &installError)) {
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

bool ZithToolchainManager::tryUseEnvironmentOverrides()
{
    const QString lspPath = QString::fromLocal8Bit(qgetenv(kLspOverrideEnv)).trimmed();
    const QString stdlibPath = QString::fromLocal8Bit(qgetenv(kStdlibOverrideEnv)).trimmed();

    if (lspPath.isEmpty() && stdlibPath.isEmpty())
        return false;

    if (lspPath.isEmpty() || stdlibPath.isEmpty()) {
        emit statusChanged(
            "Ignoring partial Zith overrides. Set both HELIOS_ZITH_LSP_PATH and "
            "HELIOS_ZITH_STDLIB_PATH to override the managed runtime.");
        return false;
    }

    const QFileInfo lspInfo(lspPath);
    if (!lspInfo.exists() || !lspInfo.isExecutable()) {
        emit failed("Configured HELIOS_ZITH_LSP_PATH does not point to an executable file.");
        return true;
    }

    if (!QFileInfo(stdlibPath).isDir()) {
        emit failed("Configured HELIOS_ZITH_STDLIB_PATH does not point to a directory.");
        return true;
    }

    emit statusChanged("Using Zith runtime from environment overrides.");
    finishWithResolvedRuntime(lspPath, stdlibPath, "environment");
    return true;
}

bool ZithToolchainManager::tryUseLocalDevelopmentOverrides()
{
    QStringList lspCandidates = {
        "../zith-lsp/build/zith-lsp",
        "../zith-lsp/out/build/dev/zith-lsp"
    };
    QString stdlibCandidate = "../Zith/stdlib";

    QDir base(QDir::currentPath());
    QString resolvedLsp;
    for (const QString &candidate : lspCandidates) {
        QString absPath = QDir::cleanPath(base.absoluteFilePath(candidate));
        QFileInfo info(absPath);
        if (info.exists() && info.isFile() && info.isExecutable()) {
            resolvedLsp = absPath;
            break;
        }
    }

    if (resolvedLsp.isEmpty())
        return false;

    QString resolvedStdlib = QDir::cleanPath(base.absoluteFilePath(stdlibCandidate));
    QFileInfo stdlibInfo(resolvedStdlib);
    if (!stdlibInfo.exists() || !stdlibInfo.isDir()) {
        resolvedStdlib = "";
    }

    emit statusChanged("Using local development Zith runtime from ../zith-lsp.");
    finishWithResolvedRuntime(resolvedLsp, resolvedStdlib, "local-dev");
    return true;
}

void ZithToolchainManager::requestLatestRelease()
{
    QNetworkRequest request(QUrl(QString::fromLatin1(kLatestReleaseUrl)));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Helios"));
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    request.setTransferTimeout(kNetworkTimeoutMs);

    m_latestReleaseReply = m_networkManager->get(request);
    connect(m_latestReleaseReply, &QNetworkReply::finished,
            this, &ZithToolchainManager::onLatestReleaseFinished);
}

void ZithToolchainManager::startNextDownload()
{
    if (m_pendingDownloads.isEmpty())
        return;

    const PendingDownload &current = m_pendingDownloads.first();

    QNetworkRequest request(current.asset.downloadUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Helios"));
    request.setTransferTimeout(kNetworkTimeoutMs);

    m_assetReply = m_networkManager->get(request);
    connect(m_assetReply, &QNetworkReply::finished,
            this, &ZithToolchainManager::onAssetDownloadFinished);
}

void ZithToolchainManager::queueDownload(DownloadKind kind, const ReleaseAsset &asset)
{
    const QString tempRoot = QDir(cacheRootPath()).filePath("downloads");
    ensureDirectory(tempRoot);

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

bool ZithToolchainManager::resolveInstalledRelease(const QString &tag,
                                                   QString *lspPath,
                                                   QString *stdlibPath) const
{
    const QString resolvedLspPath = lspInstallPath(tag);
    const QString resolvedStdlibPath = stdlibInstallPath(tag);
    const QFileInfo lspInfo(resolvedLspPath);
    const QFileInfo stdlibInfo(resolvedStdlibPath);

    if (!lspInfo.exists() || !lspInfo.isFile() || !lspInfo.isExecutable())
        return false;

    if (!stdlibInfo.exists() || !stdlibInfo.isDir())
        return false;

    const QDir stdlibDir(resolvedStdlibPath);
    const QStringList entries = stdlibDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    if (entries.isEmpty())
        return false;

    if (lspPath)
        *lspPath = resolvedLspPath;
    if (stdlibPath)
        *stdlibPath = resolvedStdlibPath;
    return true;
}

bool ZithToolchainManager::resolveNewestInstalledRelease(QString *lspPath,
                                                         QString *stdlibPath,
                                                         QString *tag) const
{
    const QDir cacheDir(cacheRootPath());
    const QFileInfoList entries = cacheDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    QString bestTag;
    QVersionNumber bestVersion;
    bool found = false;

    for (const QFileInfo &entry : entries) {
        QString candidateLsp;
        QString candidateStdlib;
        if (!resolveInstalledRelease(entry.fileName(), &candidateLsp, &candidateStdlib))
            continue;

        const QVersionNumber version = releaseVersion(entry.fileName());
        if (!found || version > bestVersion) {
            found = true;
            bestVersion = version;
            bestTag = entry.fileName();
            if (lspPath)
                *lspPath = candidateLsp;
            if (stdlibPath)
                *stdlibPath = candidateStdlib;
        }
    }

    if (tag)
        *tag = bestTag;
    return found;
}

bool ZithToolchainManager::installDownloadedAsset(const PendingDownload &download,
                                                  const QString &tag,
                                                  QString *errorMessage) const
{
    const QString releaseRoot = releaseRootPath(tag);
    if (!ensureDirectory(releaseRoot)) {
        if (errorMessage)
            *errorMessage = "Failed to create the Zith runtime cache directory.";
        return false;
    }

    if (download.kind == DownloadKind::LspBinary) {
        const QString destination = lspInstallPath(tag);
        QFile::remove(destination);
        if (!QFile::copy(download.temporaryPath, destination)) {
            if (errorMessage)
                *errorMessage = "Failed to install the downloaded Zith LSP binary.";
            return false;
        }

        QFile::setPermissions(destination,
                              QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                  QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                                  QFileDevice::ExeGroup | QFileDevice::ReadOther |
                                  QFileDevice::ExeOther);
        return true;
    }

    const QString destination = stdlibInstallPath(tag);
    QDir(destination).removeRecursively();
    if (!ensureDirectory(destination)) {
        if (errorMessage)
            *errorMessage = "Failed to create the stdlib cache directory.";
        return false;
    }

    if (!extractArchive(download.temporaryPath, destination, errorMessage)) {
        QDir(destination).removeRecursively();
        return false;
    }

    return true;
}

bool ZithToolchainManager::extractArchive(const QString &archivePath,
                                          const QString &destinationDir,
                                          QString *errorMessage) const
{
#ifdef Q_OS_WIN
    return runProcess(
        "powershell",
        {"-NoProfile", "-NonInteractive", "-Command",
         QString("Expand-Archive -LiteralPath '%1' -DestinationPath '%2' -Force")
             .arg(QString(archivePath).replace('\'', "''"),
                  QString(destinationDir).replace('\'', "''"))},
        errorMessage);
#else
    return runProcess("tar", {"-xzf", archivePath, "-C", destinationDir}, errorMessage);
#endif
}

bool ZithToolchainManager::runProcess(const QString &program,
                                      const QStringList &arguments,
                                      QString *errorMessage) const
{
    QProcess process;
    process.start(program, arguments);

    if (!process.waitForStarted()) {
        if (errorMessage)
            *errorMessage = QString("Failed to start %1 while preparing the Zith runtime.")
                                .arg(program);
        return false;
    }

    if (!process.waitForFinished()) {
        if (errorMessage)
            *errorMessage = QString("%1 did not finish while preparing the Zith runtime.")
                                .arg(program);
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        if (errorMessage) {
            const QString stdErr = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
            *errorMessage = stdErr.isEmpty()
                ? QString("%1 failed while preparing the Zith runtime.").arg(program)
                : stdErr;
        }
        return false;
    }

    return true;
}

bool ZithToolchainManager::ensureDirectory(const QString &path) const
{
    QDir dir;
    return dir.mkpath(path);
}

QString ZithToolchainManager::cacheRootPath() const
{
    QString root = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (root.isEmpty())
        root = QDir::homePath() + "/.helios";
    return QDir(root).filePath("zith-runtime");
}

QString ZithToolchainManager::releaseRootPath(const QString &tag) const
{
    return QDir(cacheRootPath()).filePath(tag);
}

QString ZithToolchainManager::lspInstallPath(const QString &tag) const
{
#ifdef Q_OS_WIN
    return QDir(releaseRootPath(tag)).filePath("zith-lsp.exe");
#else
    return QDir(releaseRootPath(tag)).filePath("zith-lsp");
#endif
}

QString ZithToolchainManager::stdlibInstallPath(const QString &tag) const
{
    return QDir(releaseRootPath(tag)).filePath("stdlib");
}

QString ZithToolchainManager::lspAssetNameForCurrentPlatform() const
{
#ifdef Q_OS_WIN
    return isArm64Architecture() ? "zith-lsp-windows-arm64.exe"
                                 : "zith-lsp-windows-amd64.exe";
#elif defined(Q_OS_MACOS)
    return "zith-lsp-macos-universal";
#else
    return isArm64Architecture() ? "zith-lsp-linux-arm64"
                                 : "zith-lsp-linux-amd64";
#endif
}

QString ZithToolchainManager::stdlibAssetSuffixForCurrentPlatform() const
{
#ifdef Q_OS_WIN
    return ".zip";
#else
    return ".tar.gz";
#endif
}

ZithToolchainManager::ReleaseAsset ZithToolchainManager::findLspAsset(
    const ReleaseInfo &release) const
{
    const QString expectedName = lspAssetNameForCurrentPlatform();
    for (const ReleaseAsset &asset : release.assets) {
        if (asset.name == expectedName)
            return asset;
    }
    return {};
}

ZithToolchainManager::ReleaseAsset ZithToolchainManager::findStdlibAsset(
    const ReleaseInfo &release) const
{
    const QString expectedSuffix = stdlibAssetSuffixForCurrentPlatform();
    for (const ReleaseAsset &asset : release.assets) {
        if (asset.name.startsWith("zithc-stdlib-") && asset.name.endsWith(expectedSuffix))
            return asset;
    }
    return {};
}

bool ZithToolchainManager::isArm64Architecture()
{
    const QString arch = QSysInfo::currentCpuArchitecture().toLower();
    return arch.contains("arm64") || arch.contains("aarch64");
}
