#include "ClangdExecutableResolver.h"

#include <QDir>
#include <QFileInfo>

QString ClangdExecutableResolver::resolve(const QString &configuredPath,
                                          const QString &pathEnvironment)
{
    const QString configured = configuredPath.trimmed();
    if (!configured.isEmpty())
        return configured;

    const QStringList directories =
        pathEnvironment.split(QLatin1Char(':'), Qt::SkipEmptyParts);
    for (const QString &directory : directories) {
        const QFileInfo candidate(QDir(directory), QStringLiteral("clangd"));
        if (candidate.isFile() && candidate.isExecutable())
            return candidate.absoluteFilePath();
    }
    return {};
}
