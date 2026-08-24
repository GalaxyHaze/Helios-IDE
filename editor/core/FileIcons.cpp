#include "FileIcons.h"

#include <QFileInfo>

QString fileIconResourceForSuffix(const QString &suffix)
{
    const QString normalized = suffix.toLower();
    if (normalized == "zith")
        return QStringLiteral(":/icons/file-zith.svg");
    if (normalized == "c")
        return QStringLiteral(":/icons/file-c.svg");
    if (normalized == "h")
        return QStringLiteral(":/icons/file-h.svg");
    return {};
}

QIcon fileIconForSuffix(const QString &suffix)
{
    const QString resource = fileIconResourceForSuffix(suffix);
    if (!resource.isEmpty())
        return QIcon(resource);
    return QIcon::fromTheme(QStringLiteral("text-x-generic"));
}

QIcon fileIconForPath(const QString &path)
{
    return fileIconForSuffix(QFileInfo(path).suffix());
}
