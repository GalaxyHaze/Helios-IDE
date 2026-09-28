#include "GitPanelPresentationModel.h"

GitPanelPresentationState GitPanelPresentationModel::fromRepositoryState(
    const GitRepositoryState &state)
{
    GitPanelPresentationState presentation;
    presentation.busy = state.busy;
    presentation.showInitializeButton = !state.repositoryAvailable;
    presentation.showConnectButton =
        state.repositoryAvailable && !state.remoteAvailable;
    presentation.showStatusList = state.repositoryAvailable;

    if (!state.repositoryAvailable) {
        presentation.branchLabel = QStringLiteral("No Repo");
        presentation.summaryMessage =
            QStringLiteral("Open a project inside a Git repository.");
        return presentation;
    }

    presentation.branchLabel = state.branch.isEmpty()
        ? QStringLiteral("No Branch")
        : state.branch;
    presentation.summaryMessage = state.entries.isEmpty()
        ? QStringLiteral("Repository is clean.")
        : QStringLiteral("%1 changed file(s). Select files to stage.")
              .arg(state.entries.size());
    return presentation;
}
