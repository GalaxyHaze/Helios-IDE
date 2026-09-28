#ifndef SHELLCOMMAND_H
#define SHELLCOMMAND_H

enum class ShellCommand
{
    NewFile,
    NewWindow,
    NewProject,
    OpenFile,
    SaveFile,
    Exit,
    Build,
    CheckFile,
    FormatDocument,
    Run,
    Stop,
    RestartLsp,
    Preferences,
    VimHelp,
    Shortcuts,
    LspManager,
    ToggleOutline,
    ToggleBottomPanel,
    GettingStarted,
    OpenFolder,
    Find,
    Replace,
    FindNext,
    FindPrevious,
    Explorer,
    WorkspaceSearch,
    Git,
    Settings,
    HideSidebar
};

#endif
