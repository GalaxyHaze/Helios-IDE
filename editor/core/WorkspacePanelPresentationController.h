#ifndef WORKSPACEPANELPRESENTATIONCONTROLLER_H
#define WORKSPACEPANELPRESENTATIONCONTROLLER_H

#include <QObject>

#include <functional>

#include "ShellCommand.h"

class BottomPanel;
class CodeEditor;
class EditorChromeController;
class OutlinePanel;
class ShellCommandSurface;

class WorkspacePanelPresentationController : public QObject
{
    Q_OBJECT

public:
    struct Dependencies
    {
        OutlinePanel *outlinePanel = nullptr;
        BottomPanel *bottomPanel = nullptr;
        ShellCommandSurface *commandSurface = nullptr;
        EditorChromeController *editorChrome = nullptr;
    };

    using CurrentEditor = std::function<CodeEditor *()>;
    using PersistOutlineVisibility = std::function<void(bool)>;

    WorkspacePanelPresentationController(Dependencies dependencies,
                                         CurrentEditor currentEditor,
                                         PersistOutlineVisibility
                                             persistOutlineVisibility,
                                         QObject *parent = nullptr);

    void setOutlineVisible(bool visible);
    void toggleOutline();
    void setBottomPanelVisible(bool visible);
    void toggleBottomPanel();
    bool handleShellCommand(ShellCommand command);
    void synchronize();

private:
    OutlinePanel *m_outlinePanel = nullptr;
    BottomPanel *m_bottomPanel = nullptr;
    ShellCommandSurface *m_commandSurface = nullptr;
    EditorChromeController *m_editorChrome = nullptr;
    CurrentEditor m_currentEditor;
    PersistOutlineVisibility m_persistOutlineVisibility;
};

#endif
