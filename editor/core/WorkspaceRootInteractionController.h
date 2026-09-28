#ifndef WORKSPACEROOTINTERACTIONCONTROLLER_H
#define WORKSPACEROOTINTERACTIONCONTROLLER_H

#include "WorkspaceRootController.h"

#include <QObject>
#include <QString>

#include <cstdint>
#include <functional>

class QWidget;

class WorkspaceRootInteractionController : public QObject
{
    Q_OBJECT

public:
    using RequestDirectory =
        std::function<QString(const QString &, const QString &)>;
    using CurrentRoot = std::function<QString()>;

    WorkspaceRootInteractionController(
        QWidget *dialogParent, WorkspaceRootController *rootController,
        RequestDirectory requestDirectory = {},
        CurrentRoot currentRoot = {}, QObject *parent = nullptr);

    WorkspaceRootController::ActivationResult openFolder();
    WorkspaceRootController::ActivationResult newProject();
    WorkspaceRootController::ActivationResult createContext();

private:
    enum class Intent : std::uint8_t
    {
        ReplaceCurrentRoot,
        CreateContext,
    };

    WorkspaceRootController::ActivationResult activate(Intent intent);
    QString requestDirectory(const QString &title,
                             const QString &initialPath) const;

    QWidget *m_dialogParent = nullptr;
    WorkspaceRootController *m_rootController = nullptr;
    RequestDirectory m_requestDirectory;
    CurrentRoot m_currentRoot;
};

#endif
