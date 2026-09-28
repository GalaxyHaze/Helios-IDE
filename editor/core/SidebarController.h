#ifndef SIDEBARCONTROLLER_H
#define SIDEBARCONTROLLER_H

#include "../widgets/ActivityBar.h"
#include "ShellCommand.h"

#include <QObject>

#include <functional>

class GitPanel;
class QStackedWidget;

class SidebarController : public QObject
{
public:
    using PersistVisibility = std::function<void(bool)>;
    using PrepareSettings = std::function<void()>;
    using FocusExplorer = std::function<void()>;

    SidebarController(ActivityBar *activityBar,
                      QStackedWidget *sidePanel,
                      GitPanel *gitPanel,
                      PersistVisibility persistVisibility,
                      PrepareSettings prepareSettings,
                      FocusExplorer focusExplorer,
                      QObject *parent = nullptr);

    void setVisible(bool visible);
    void selectMode(ActivityBar::Mode mode);
    void showSettings();
    void synchronizeActivityBar();
    bool handleShellCommand(ShellCommand command);

private:
    ActivityBar *m_activityBar = nullptr;
    QStackedWidget *m_sidePanel = nullptr;
    GitPanel *m_gitPanel = nullptr;
    PersistVisibility m_persistVisibility;
    PrepareSettings m_prepareSettings;
    FocusExplorer m_focusExplorer;
};

#endif
