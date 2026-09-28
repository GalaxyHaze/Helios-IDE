#ifndef EDITORINTERACTIONCONTROLLER_H
#define EDITORINTERACTIONCONTROLLER_H

#include <QObject>
#include <QString>

#include <functional>

class CodeEditor;
class EditorChromeController;
class EditorLspActionController;
class LocationNavigator;

class EditorInteractionController : public QObject
{
    Q_OBJECT

public:
    struct Dependencies
    {
        EditorLspActionController *lspActions = nullptr;
        EditorChromeController *editorChrome = nullptr;
        LocationNavigator *locationNavigator = nullptr;
    };

    struct Callbacks
    {
        std::function<CodeEditor *()> currentEditor;
        std::function<bool()> saveCurrent;
        std::function<void(CodeEditor *)> closeEditor;
        std::function<void()> updateCentralWidgetState;
        std::function<void(const QString &, int)> showStatus;
        std::function<void(const QString &)> setVimModeLabel;
    };

    EditorInteractionController(Dependencies dependencies, Callbacks callbacks,
                                QObject *parent = nullptr);

    void attach(CodeEditor *editor);
    void handleVimCommand(const QString &command);

private:
    void closeCurrentEditor(CodeEditor *editor);
    void showStatus(const QString &message, int timeout) const;

    EditorLspActionController *m_lspActions;
    EditorChromeController *m_editorChrome;
    LocationNavigator *m_locationNavigator;
    Callbacks m_callbacks;
};

#endif
