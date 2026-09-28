#include "ZithRuntimeOverrideResolver.h"

#include <QFileInfo>

ZithRuntimeOverrideResolver::Result
ZithRuntimeOverrideResolver::resolve(const QString &lspPath,
                                     const QString &stdlibPath)
{
    const QString resolvedLspPath = lspPath.trimmed();
    const QString resolvedStdlibPath = stdlibPath.trimmed();

    if (resolvedLspPath.isEmpty() && resolvedStdlibPath.isEmpty())
        return {};

    if (resolvedLspPath.isEmpty() || resolvedStdlibPath.isEmpty()) {
        return {
            Action::IgnoreAndContinue,
            {},
            QStringLiteral(
                "Ignoring partial Zith overrides. Set both "
                "HELIOS_ZITH_LSP_PATH and HELIOS_ZITH_STDLIB_PATH to override "
                "the managed runtime.")};
    }

    const QFileInfo lspInfo(resolvedLspPath);
    if (!lspInfo.exists() || !lspInfo.isExecutable()) {
        return {
            Action::Fail,
            {},
            QStringLiteral(
                "Configured HELIOS_ZITH_LSP_PATH does not point to an "
                "executable file.")};
    }

    if (!QFileInfo(resolvedStdlibPath).isDir()) {
        return {
            Action::Fail,
            {},
            QStringLiteral(
                "Configured HELIOS_ZITH_STDLIB_PATH does not point to a "
                "directory.")};
    }

    return {
        Action::Use,
        {resolvedLspPath, resolvedStdlibPath},
        QStringLiteral("Using Zith runtime from environment overrides.")};
}
