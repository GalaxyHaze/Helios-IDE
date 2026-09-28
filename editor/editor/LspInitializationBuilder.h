#ifndef LSPINITIALIZATIONBUILDER_H
#define LSPINITIALIZATIONBUILDER_H

#include <QJsonObject>
#include <QString>

struct LspInitializationOptions
{
    QString workspaceRoot;
    QString stdlibPath;
    QString initMode;
};

class LspInitializationBuilder
{
public:
    static QJsonObject build(const LspInitializationOptions &options);
};

#endif
