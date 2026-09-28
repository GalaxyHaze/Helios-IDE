#ifndef EDITORLSPACTIONCONTROLLER_H
#define EDITORLSPACTIONCONTROLLER_H

#include "../editor/LspClient.h"
#include "ShellCommand.h"

#include <QObject>

class QTabWidget;
class CodeEditor;

class EditorLspActionController : public QObject
{
public:
    explicit EditorLspActionController(QTabWidget *tabWidget,
                                       QObject *parent = nullptr);

    void attach(CodeEditor *editor);
    bool requestRename(CodeEditor *editor, const QString &uri, int version,
                       const LspPosition &position, const QString &newName);
    bool requestCodeActions(CodeEditor *editor, const QString &uri,
                            int version, const LspRange &range);
    bool handleShellCommand(ShellCommand command);

private:
    CodeEditor *currentEditor() const;
    void flushPendingChanges() const;

    QTabWidget *m_tabWidget;
};

#endif
