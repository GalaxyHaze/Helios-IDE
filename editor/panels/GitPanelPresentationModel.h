#ifndef GITPANELPRESENTATIONMODEL_H
#define GITPANELPRESENTATIONMODEL_H

#include "../core/GitRepositorySession.h"

#include <QString>

struct GitPanelPresentationState
{
    QString branchLabel;
    QString summaryMessage;
    bool showInitializeButton = false;
    bool showConnectButton = false;
    bool showStatusList = false;
    bool busy = false;
};

class GitPanelPresentationModel
{
public:
    static GitPanelPresentationState fromRepositoryState(
        const GitRepositoryState &state);
};

#endif
