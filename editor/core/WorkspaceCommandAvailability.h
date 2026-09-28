#ifndef WORKSPACECOMMANDAVAILABILITY_H
#define WORKSPACECOMMANDAVAILABILITY_H

#include <QString>

struct WorkspaceCommandAvailability
{
    bool canBuild = false;
    bool canCheck = false;
    bool canFormat = false;
    bool canRun = false;
    bool canStop = false;
    QString buildTooltip;
    QString checkTooltip;
    QString formatTooltip;
    QString runTooltip;
    QString stopTooltip;
};

#endif
