#ifndef CONTEXTNAVIGATIONCONTROLLER_H
#define CONTEXTNAVIGATIONCONTROLLER_H

#include <QObject>
#include <QString>

#include <functional>

class ContextManager;
class WorkspaceRootInteractionController;
class QWidget;

class ContextNavigationController : public QObject
{
public:
    using SaveContext = std::function<void()>;

    ContextNavigationController(ContextManager *contextManager,
                                QWidget *shortcutParent,
                                SaveContext saveContext,
                                WorkspaceRootInteractionController
                                    *rootInteraction,
                                QObject *parent = nullptr);

    void navigateLeft();
    void navigateRight();

private:
    ContextManager *m_contextManager = nullptr;
    SaveContext m_saveContext;
    WorkspaceRootInteractionController *m_rootInteraction = nullptr;
};

#endif
