#include "ShellCommandSurface.h"

#include "TranslationManager.h"

#include <QAction>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QShortcut>
#include <QWidget>

ShellCommandSurface::ShellCommandSurface(QMenuBar *menuBar, QObject *parent)
    : QObject(parent)
{
    if (!menuBar)
        return;

    m_fileMenu = menuBar->addMenu(QStringLiteral("&File"));
    m_newAction =
        addCommand(m_fileMenu, QStringLiteral("&New File"), QKeySequence::New,
                   Command::NewFile);
    addCommand(m_fileMenu, QStringLiteral("New &Window"),
               QKeySequence(QStringLiteral("Ctrl+Shift+N")), Command::NewWindow);
    m_newProjectAction =
        addCommand(m_fileMenu, QStringLiteral("New &Project..."),
                   QKeySequence(QStringLiteral("Ctrl+Alt+N")),
                   Command::NewProject);
    m_openAction =
        addCommand(m_fileMenu, QStringLiteral("&Open..."), QKeySequence::Open,
                   Command::OpenFile);
    m_saveAction =
        addCommand(m_fileMenu, QStringLiteral("&Save"), QKeySequence::Save,
                   Command::SaveFile);
    m_exitAction =
        addCommand(m_fileMenu, QStringLiteral("E&xit"), QKeySequence::Quit,
                   Command::Exit);

    m_toolsMenu = menuBar->addMenu(QStringLiteral("&Tools"));
    m_buildAction =
        addCommand(m_toolsMenu, QStringLiteral("&Build"),
                   QKeySequence(QStringLiteral("Ctrl+B")), Command::Build);
    m_checkAction =
        addCommand(m_toolsMenu, QStringLiteral("&Check File"),
                   QKeySequence(QStringLiteral("Ctrl+Shift+C")),
                   Command::CheckFile);
    m_formatAction =
        addCommand(m_toolsMenu, QStringLiteral("Format Document"),
                   QKeySequence(QStringLiteral("Ctrl+Alt+L")),
                   Command::FormatDocument);
    m_runAction =
        addCommand(m_toolsMenu, QStringLiteral("&Run"),
                   QKeySequence(QStringLiteral("Ctrl+Shift+R")), Command::Run);
    m_stopAction =
        addCommand(m_toolsMenu, QStringLiteral("&Stop"),
                   QKeySequence(QStringLiteral("Ctrl+Shift+Q")), Command::Stop);
    m_stopAction->setEnabled(false);
    m_restartLspAction =
        addCommand(m_toolsMenu, QStringLiteral("Restart &LSP"),
                   QKeySequence(QStringLiteral("Ctrl+Shift+L")),
                   Command::RestartLsp);

    m_viewMenu = menuBar->addMenu(QStringLiteral("&View"));
    m_preferencesAction =
        addCommand(m_viewMenu, QStringLiteral("&Preferences..."),
                   QKeySequence(QStringLiteral("Ctrl+,")), Command::Preferences);
    m_vimHelpAction =
        addCommand(m_viewMenu, QStringLiteral("Vim Motions..."),
                   QKeySequence(), Command::VimHelp);
    m_shortcutsAction =
        addCommand(m_viewMenu, QStringLiteral("Shortcuts..."), QKeySequence(),
                   Command::Shortcuts);
    m_lspManagerAction =
        addCommand(m_viewMenu, QStringLiteral("LSP Manager..."), QKeySequence(),
                   Command::LspManager);
    m_outlineAction =
        addCommand(m_viewMenu, QStringLiteral("Structure"), QKeySequence(),
                   Command::ToggleOutline);
    m_outlineAction->setCheckable(true);
    m_bottomPanelAction =
        addCommand(m_viewMenu, QStringLiteral("Toggle Bottom Panel"),
                   QKeySequence(), Command::ToggleBottomPanel);
    m_bottomPanelAction->setCheckable(true);

    m_helpMenu = menuBar->addMenu(QStringLiteral("&Help"));
    m_gettingStartedAction =
        addCommand(m_helpMenu, QStringLiteral("&Getting Started"),
                   QKeySequence(QStringLiteral("F1")), Command::GettingStarted);
}

void ShellCommandSurface::installShortcuts(QWidget *parent)
{
    if (!parent)
        return;

    addShortcut(parent, QKeySequence(QStringLiteral("Ctrl+Alt+O")),
                Command::OpenFolder);
    addShortcut(parent, QKeySequence::Find, Command::Find);
    addShortcut(parent, QKeySequence(QStringLiteral("Ctrl+H")),
                Command::Replace);
    addShortcut(parent, QKeySequence::FindNext, Command::FindNext);
    addShortcut(parent, QKeySequence::FindPrevious, Command::FindPrevious);
    addShortcut(parent, QKeySequence(QStringLiteral("Ctrl+Shift+E")),
                Command::Explorer);
    addShortcut(parent, QKeySequence(QStringLiteral("Ctrl+Shift+F")),
                Command::WorkspaceSearch);
    addShortcut(parent, QKeySequence(QStringLiteral("Ctrl+Shift+G")),
                Command::Git);
    addShortcut(parent, QKeySequence(QStringLiteral("Ctrl+,")),
                Command::Settings);
    addShortcut(parent, QKeySequence(QStringLiteral("Ctrl+Shift+X")),
                Command::HideSidebar);
}

void ShellCommandSurface::applyTranslations()
{
    const auto &translation = TranslationManager::instance();
    if (!m_fileMenu)
        return;

    m_fileMenu->setTitle(translation.translate(QStringLiteral("menu.file")));
    m_newAction->setText(
        translation.translate(QStringLiteral("menu.new_file")));
    m_newProjectAction->setText(
        translation.translate(QStringLiteral("menu.new_project")));
    m_openAction->setText(
        translation.translate(QStringLiteral("menu.open_file")));
    m_saveAction->setText(translation.translate(QStringLiteral("menu.save")));
    m_exitAction->setText(translation.translate(QStringLiteral("menu.exit")));

    m_toolsMenu->setTitle(translation.translate(QStringLiteral("menu.tools")));
    m_buildAction->setText(translation.translate(QStringLiteral("menu.build")));
    m_checkAction->setText(translation.translate(QStringLiteral("menu.check")));
    m_formatAction->setText(
        translation.translate(QStringLiteral("menu.format_doc")));
    m_runAction->setText(translation.translate(QStringLiteral("menu.run")));
    m_stopAction->setText(translation.translate(QStringLiteral("menu.stop")));
    m_restartLspAction->setText(
        translation.translate(QStringLiteral("menu.restart_lsp")));

    m_viewMenu->setTitle(translation.translate(QStringLiteral("menu.view")));
    m_preferencesAction->setText(
        translation.translate(QStringLiteral("menu.preferences")));
    m_vimHelpAction->setText(
        translation.translate(QStringLiteral("menu.vim_motions")));
    m_shortcutsAction->setText(
        translation.translate(QStringLiteral("menu.shortcuts")));
    m_lspManagerAction->setText(
        translation.translate(QStringLiteral("menu.lsp_manager")));
    m_outlineAction->setText(
        translation.translate(QStringLiteral("menu.structure")));
    m_helpMenu->setTitle(translation.translate(QStringLiteral("menu.help")));
    m_gettingStartedAction->setText(
        translation.translate(QStringLiteral("welcome.getting_started")));
}

void ShellCommandSurface::setWorkspaceActionState(
    const WorkspaceActionState &state)
{
    if (!m_buildAction)
        return;

    m_buildAction->setEnabled(state.canBuild);
    m_checkAction->setEnabled(state.canCheck);
    m_formatAction->setEnabled(state.canFormat);
    m_runAction->setEnabled(state.canRun);
    m_stopAction->setEnabled(state.canStop);
    m_buildAction->setToolTip(state.buildTooltip);
    m_checkAction->setToolTip(state.checkTooltip);
    m_formatAction->setToolTip(state.formatTooltip);
    m_runAction->setToolTip(state.runTooltip);
    m_stopAction->setToolTip(state.stopTooltip);
}

void ShellCommandSurface::setRestartLspEnabled(bool enabled)
{
    if (m_restartLspAction)
        m_restartLspAction->setEnabled(enabled);
}

void ShellCommandSurface::setOutlineVisible(bool visible)
{
    if (m_outlineAction)
        m_outlineAction->setChecked(visible);
}

void ShellCommandSurface::setBottomPanelVisible(bool visible)
{
    if (m_bottomPanelAction)
        m_bottomPanelAction->setChecked(visible);
}

void ShellCommandSurface::addShortcut(QWidget *parent,
                                       const QKeySequence &shortcut,
                                       Command command)
{
    auto *action = new QShortcut(shortcut, parent);
    connect(action, &QShortcut::activated, this,
            [this, command]() { emit commandRequested(command); });
}

QAction *ShellCommandSurface::addCommand(QMenu *menu, const QString &text,
                                         const QKeySequence &shortcut,
                                         Command command)
{
    auto *action = menu->addAction(text, shortcut);
    connect(action, &QAction::triggered, this,
            [this, command]() { emit commandRequested(command); });
    return action;
}
