#ifndef EDITORFILECONTROLLER_H
#define EDITORFILECONTROLLER_H

#include "EditorSessionController.h"
#include "ShellCommand.h"

#include <QObject>
#include <QString>

#include <functional>

class CodeEditor;
class QTabWidget;
class QWidget;

class EditorFileController : public QObject
{
    Q_OBJECT

public:
    struct Callbacks
    {
        std::function<QString()> workspaceRoot;
        std::function<void(const QString &, int)> showStatus;
        std::function<void()> refreshCommandAvailability;
    };

    EditorFileController(QWidget *dialogParent, QTabWidget *tabWidget,
                         EditorSessionController *session,
                         Callbacks callbacks,
                         QObject *parent = nullptr);

    bool handleShellCommand(ShellCommand command);
    void newFile();
    void openFile();
    bool saveEditor(CodeEditor *editor);

private:
    CodeEditor *currentEditor() const;
    void showStatus(const QString &message, int timeout) const;

    QWidget *m_dialogParent = nullptr;
    QTabWidget *m_tabWidget = nullptr;
    EditorSessionController *m_session = nullptr;
    Callbacks m_callbacks;
};

#endif
