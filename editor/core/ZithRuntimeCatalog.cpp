#include "ZithRuntimeCatalog.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QVersionNumber>

namespace
{
QVersionNumber releaseVersion(const QString &tag)
{
    QString normalized = tag.trimmed();
    if (normalized.startsWith('v'))
        normalized.remove(0, 1);
    return QVersionNumber::fromString(normalized);
}
}

void ZithRuntimeCatalog::setCacheRoot(const QString &path)
{
    m_cacheRootOverride = path;
}

QString ZithRuntimeCatalog::cacheRootPath() const
{
    if (!m_cacheRootOverride.isEmpty())
        return m_cacheRootOverride;

    QString root =
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (root.isEmpty())
        root = QDir::homePath() + "/.helios";
    return QDir(root).filePath("zith-runtime");
}

QString ZithRuntimeCatalog::releaseRootPath(const QString &tag) const
{
    return QDir(cacheRootPath()).filePath(tag);
}

QString ZithRuntimeCatalog::lspInstallPath(const QString &tag) const
{
#ifdef Q_OS_WIN
    return QDir(releaseRootPath(tag)).filePath("zith-lsp.exe");
#else
    return QDir(releaseRootPath(tag)).filePath("zith-lsp");
#endif
}

QString ZithRuntimeCatalog::stdlibInstallPath(const QString &tag) const
{
    return QDir(releaseRootPath(tag)).filePath("stdlib");
}

bool ZithRuntimeCatalog::isReleaseDirectoryName(
    const QString &directoryName) const
{
    return directoryName.startsWith(QLatin1Char('v')) &&
        !releaseVersion(directoryName).isNull();
}

std::optional<ZithRuntimeCatalog::ResolvedRuntime>
ZithRuntimeCatalog::resolveInstalledRelease(const QString &tag) const
{
    if (!isReleaseDirectoryName(tag))
        return std::nullopt;

    const QString resolvedLspPath = lspInstallPath(tag);
    const QString resolvedStdlibPath = stdlibInstallPath(tag);
    const QFileInfo lspInfo(resolvedLspPath);
    const QFileInfo stdlibInfo(resolvedStdlibPath);

    if (!lspInfo.exists() || !lspInfo.isFile() || !lspInfo.isExecutable())
        return std::nullopt;
    if (!stdlibInfo.exists() || !stdlibInfo.isDir())
        return std::nullopt;

    const QDir stdlibDir(resolvedStdlibPath);
    const QStringList entries =
        stdlibDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    if (entries.isEmpty())
        return std::nullopt;

    return ResolvedRuntime{resolvedLspPath, resolvedStdlibPath};
}

std::optional<ZithRuntimeCatalog::InstalledRuntime>
ZithRuntimeCatalog::resolveNewestInstalledRelease() const
{
    const QDir cacheDir(cacheRootPath());
    const QFileInfoList entries =
        cacheDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    QString bestTag;
    QVersionNumber bestVersion;
    bool found = false;

    for (const QFileInfo &entry : entries) {
        const auto resolved = resolveInstalledRelease(entry.fileName());
        if (!resolved)
            continue;

        const QVersionNumber version = releaseVersion(entry.fileName());
        if (!found || version > bestVersion) {
            found = true;
            bestVersion = version;
            bestTag = entry.fileName();
        }
    }

    if (!found)
        return std::nullopt;

    const auto resolved = resolveInstalledRelease(bestTag);
    if (!resolved)
        return std::nullopt;
    return InstalledRuntime{*resolved, bestTag};
}

bool ZithRuntimeCatalog::removeStaleLocalRuntimeCache(
    QString *errorMessage) const
{
    const QString localCache = releaseRootPath(QStringLiteral("local"));
    const QFileInfo localCacheInfo(localCache);
    if (!localCacheInfo.exists())
        return true;

    if (QDir(localCache).removeRecursively())
        return true;

    if (errorMessage) {
        *errorMessage = QString(
            "Failed to remove the stale local Zith runtime cache at %1.")
                            .arg(localCache);
    }
    return false;
}
