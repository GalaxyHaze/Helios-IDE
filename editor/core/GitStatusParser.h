#ifndef GITSTATUSPARSER_H
#define GITSTATUSPARSER_H

#include <QList>
#include <QString>

struct GitStatusEntry
{
    QString status;
    QString relativePath;
};

struct GitStatusSnapshot
{
    QString branch;
    QList<GitStatusEntry> entries;
};

class GitStatusParser
{
public:
    static GitStatusSnapshot parse(const QString &output);
};

#endif
