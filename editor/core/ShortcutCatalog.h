#ifndef SHORTCUTCATALOG_H
#define SHORTCUTCATALOG_H

#include <QList>
#include <QString>

struct ShortcutEntry
{
    QString translationKey;
    QString keySequence;
};

struct ShortcutCategory
{
    QString translationKey;
    QList<ShortcutEntry> entries;
};

class ShortcutCatalog
{
public:
    static QList<ShortcutCategory> categories();
};

#endif
