#ifndef ZITHRUNTIMECATALOG_H
#define ZITHRUNTIMECATALOG_H

#include <QString>

#include <optional>

class ZithRuntimeCatalog
{
public:
    struct ResolvedRuntime
    {
        QString lspPath;
        QString stdlibPath;
    };

    struct InstalledRuntime
    {
        ResolvedRuntime paths;
        QString tag;
    };

    ZithRuntimeCatalog() = default;

    void setCacheRoot(const QString &path);
    QString cacheRootPath() const;

    std::optional<ResolvedRuntime> resolveInstalledRelease(
        const QString &tag) const;
    std::optional<InstalledRuntime> resolveNewestInstalledRelease() const;
    bool isReleaseDirectoryName(const QString &directoryName) const;
    bool removeStaleLocalRuntimeCache(QString *errorMessage = nullptr) const;

    QString releaseRootPath(const QString &tag) const;
    QString lspInstallPath(const QString &tag) const;
    QString stdlibInstallPath(const QString &tag) const;

private:
    QString m_cacheRootOverride;
};

#endif
