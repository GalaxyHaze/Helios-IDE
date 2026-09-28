#include "WorkspaceCommandResultDecoder.h"

#include <QJsonObject>

WorkspaceCommandResult WorkspaceCommandResultDecoder::decode(
    bool transportSuccess, const QJsonValue &result)
{
    WorkspaceCommandResult decoded;
    decoded.success = transportSuccess;

    if (result.isString()) {
        decoded.text = result.toString();
        return decoded;
    }

    const QJsonObject object = result.toObject();
    if (object.isEmpty()) {
        return decoded;
    }

    decoded.hasServerDetails = true;
    decoded.success =
        transportSuccess &&
        object.value(QStringLiteral("success")).toBool(true);
    decoded.programUri =
        object.value(QStringLiteral("programUri")).toString();
    decoded.taskId = object.value(QStringLiteral("taskId")).toString();
    decoded.hasCodegenAvailable =
        object.contains(QStringLiteral("codegenAvailable"));
    decoded.codegenAvailable =
        object.value(QStringLiteral("codegenAvailable")).toBool(false);
    decoded.serverMessage =
        object.value(QStringLiteral("message")).toString();
    return decoded;
}
