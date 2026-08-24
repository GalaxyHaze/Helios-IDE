#ifndef FILEICONS_H
#define FILEICONS_H

#include <QIcon>
#include <QString>

QString fileIconResourceForSuffix(const QString &suffix);
QIcon fileIconForSuffix(const QString &suffix);
QIcon fileIconForPath(const QString &path);

#endif // FILEICONS_H
