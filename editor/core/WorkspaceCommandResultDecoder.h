#ifndef WORKSPACECOMMANDRESULTDECODER_H
#define WORKSPACECOMMANDRESULTDECODER_H

#include <QJsonValue>
#include <QString>

struct WorkspaceCommandResult
{
    bool success = false;
    QString programUri;
    QString taskId;
    bool hasCodegenAvailable = false;
    bool codegenAvailable = false;
    QString text;
    bool hasServerDetails = false;
    QString serverMessage;
};

class WorkspaceCommandResultDecoder
{
public:
    static WorkspaceCommandResult decode(bool transportSuccess,
                                         const QJsonValue &result);
};

#endif
