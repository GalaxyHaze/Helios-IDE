#include "ZithRuntimeInstaller.h"

#include <QDir>
#include <QFile>
#include <QProcess>

ZithRuntimeInstaller::ZithRuntimeInstaller(
    const ZithRuntimeCatalog &runtimeCatalog)
    : m_runtimeCatalog(runtimeCatalog)
{
}

bool ZithRuntimeInstaller::install(
    const ZithRuntimeInstallRequest &request,
    QString *errorMessage) const
{
    if (request.sourcePath.isEmpty() || request.tag.isEmpty()) {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Zith runtime installation request was incomplete.");
        return false;
    }

    if (!ensureDirectory(m_runtimeCatalog.releaseRootPath(request.tag))) {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral(
                "Failed to create the Zith runtime cache directory.");
        return false;
    }

    if (request.kind == ZithRuntimeAssetKind::LspBinary)
        return installLspBinary(request, errorMessage);
    return installStdlibArchive(request, errorMessage);
}

bool ZithRuntimeInstaller::installLspBinary(
    const ZithRuntimeInstallRequest &request,
    QString *errorMessage) const
{
    const QString destination = m_runtimeCatalog.lspInstallPath(request.tag);
    QFile::remove(destination);
    if (!QFile::copy(request.sourcePath, destination)) {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral(
                "Failed to install the downloaded Zith LSP binary.");
        return false;
    }

    QFile::setPermissions(destination,
                          QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                              QFileDevice::ExeOwner | QFileDevice::ReadGroup |
                              QFileDevice::ExeGroup | QFileDevice::ReadOther |
                              QFileDevice::ExeOther);
    return true;
}

bool ZithRuntimeInstaller::installStdlibArchive(
    const ZithRuntimeInstallRequest &request,
    QString *errorMessage) const
{
    const QString destination = m_runtimeCatalog.stdlibInstallPath(request.tag);
    QDir(destination).removeRecursively();
    if (!ensureDirectory(destination)) {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Failed to create the stdlib cache directory.");
        return false;
    }

    if (!extractArchive(request.sourcePath, destination, errorMessage)) {
        QDir(destination).removeRecursively();
        return false;
    }
    return true;
}

bool ZithRuntimeInstaller::extractArchive(const QString &archivePath,
                                          const QString &destinationDir,
                                          QString *errorMessage) const
{
#ifdef Q_OS_WIN
    return runProcess(
        QStringLiteral("powershell"),
        {QStringLiteral("-NoProfile"), QStringLiteral("-NonInteractive"),
         QStringLiteral("-Command"),
         QStringLiteral("Expand-Archive -LiteralPath '%1' -DestinationPath '%2' -Force")
             .arg(QString(archivePath).replace('\'', "''"),
                  QString(destinationDir).replace('\'', "''"))},
        errorMessage);
#else
    return runProcess(QStringLiteral("tar"),
                      {QStringLiteral("-xzf"), archivePath,
                       QStringLiteral("-C"), destinationDir},
                      errorMessage);
#endif
}

bool ZithRuntimeInstaller::runProcess(const QString &program,
                                      const QStringList &arguments,
                                      QString *errorMessage) const
{
    QProcess process;
    process.start(program, arguments);

    if (!process.waitForStarted()) {
        if (errorMessage != nullptr) {
            *errorMessage =
                QString("Failed to start %1 while preparing the Zith runtime.")
                    .arg(program);
        }
        return false;
    }

    if (!process.waitForFinished()) {
        if (errorMessage != nullptr) {
            *errorMessage =
                QString("%1 did not finish while preparing the Zith runtime.")
                    .arg(program);
        }
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        if (errorMessage != nullptr) {
            const QString stdErr =
                QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
            *errorMessage = stdErr.isEmpty()
                ? QString("%1 failed while preparing the Zith runtime.").arg(program)
                : stdErr;
        }
        return false;
    }

    return true;
}

bool ZithRuntimeInstaller::ensureDirectory(const QString &path) const
{
    QDir dir;
    return dir.mkpath(path);
}
