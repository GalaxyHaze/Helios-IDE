#ifndef ZITHRUNTIMEINSTALLER_H
#define ZITHRUNTIMEINSTALLER_H

#include <QString>

#include "ZithRuntimeCatalog.h"

enum class ZithRuntimeAssetKind {
    LspBinary,
    StdlibArchive
};

struct ZithRuntimeInstallRequest
{
    ZithRuntimeAssetKind kind = ZithRuntimeAssetKind::LspBinary;
    QString sourcePath;
    QString tag;
};

class ZithRuntimeInstaller
{
public:
    explicit ZithRuntimeInstaller(const ZithRuntimeCatalog &runtimeCatalog);

    bool install(const ZithRuntimeInstallRequest &request,
                 QString *errorMessage = nullptr) const;

private:
    bool installLspBinary(const ZithRuntimeInstallRequest &request,
                          QString *errorMessage) const;
    bool installStdlibArchive(const ZithRuntimeInstallRequest &request,
                              QString *errorMessage) const;
    bool extractArchive(const QString &archivePath,
                        const QString &destinationDir,
                        QString *errorMessage) const;
    bool runProcess(const QString &program,
                    const QStringList &arguments,
                    QString *errorMessage) const;
    bool ensureDirectory(const QString &path) const;

    const ZithRuntimeCatalog &m_runtimeCatalog;
};

#endif
