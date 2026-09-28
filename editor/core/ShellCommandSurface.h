#ifndef SHELLCOMMANDSURFACE_H
#define SHELLCOMMANDSURFACE_H

#include <QObject>
#include <QString>

#include "ShellCommand.h"
#include "WorkspaceCommandAvailability.h"

class QAction;
class QMenu;
class QMenuBar;
class QWidget;

class ShellCommandSurface : public QObject
{
    Q_OBJECT

public:
    using Command = ShellCommand;

    using WorkspaceActionState = WorkspaceCommandAvailability;

    explicit ShellCommandSurface(QMenuBar *menuBar,
                                 QObject *parent = nullptr);

    void installShortcuts(QWidget *parent);
    void applyTranslations();
    void setWorkspaceActionState(const WorkspaceActionState &state);
    void setRestartLspEnabled(bool enabled);
    void setOutlineVisible(bool visible);
    void setBottomPanelVisible(bool visible);

signals:
    void commandRequested(ShellCommand command);

private:
    void addShortcut(QWidget *parent, const QKeySequence &shortcut,
                     Command command);
    QAction *addCommand(QMenu *menu, const QString &text,
                        const QKeySequence &shortcut, Command command);

    QMenu *m_fileMenu = nullptr;
    QMenu *m_toolsMenu = nullptr;
    QMenu *m_viewMenu = nullptr;
    QMenu *m_helpMenu = nullptr;

    QAction *m_newAction = nullptr;
    QAction *m_newProjectAction = nullptr;
    QAction *m_openAction = nullptr;
    QAction *m_saveAction = nullptr;
    QAction *m_exitAction = nullptr;
    QAction *m_buildAction = nullptr;
    QAction *m_checkAction = nullptr;
    QAction *m_formatAction = nullptr;
    QAction *m_runAction = nullptr;
    QAction *m_stopAction = nullptr;
    QAction *m_restartLspAction = nullptr;
    QAction *m_preferencesAction = nullptr;
    QAction *m_vimHelpAction = nullptr;
    QAction *m_shortcutsAction = nullptr;
    QAction *m_lspManagerAction = nullptr;
    QAction *m_outlineAction = nullptr;
    QAction *m_bottomPanelAction = nullptr;
    QAction *m_gettingStartedAction = nullptr;
};

#endif
