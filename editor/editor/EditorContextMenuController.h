#ifndef EDITORCONTEXTMENUCONTROLLER_H
#define EDITORCONTEXTMENUCONTROLLER_H

#include <QObject>
#include <QPoint>

#include <memory>

class CodeEditor;
class EditorLanguageFeatureController;
class QMenu;

class EditorContextMenuController : public QObject
{
    Q_OBJECT

public:
    EditorContextMenuController(CodeEditor *editor,
                                EditorLanguageFeatureController *features,
                                QObject *parent = nullptr);

    std::unique_ptr<QMenu> createMenu();
    void show(const QPoint &globalPosition);

private:
    CodeEditor *m_editor = nullptr;
    EditorLanguageFeatureController *m_features = nullptr;
};

#endif
