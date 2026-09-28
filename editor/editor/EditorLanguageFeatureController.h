#ifndef EDITORLANGUAGEFEATURECONTROLLER_H
#define EDITORLANGUAGEFEATURECONTROLLER_H

#include "LspClient.h"

#include <QObject>

class CodeEditor;
class EditorLanguageFeedbackPresenter;
class LspCompleter;
class QKeyEvent;
class QMouseEvent;
class QTimer;

class EditorLanguageFeatureController : public QObject
{
    Q_OBJECT

public:
    enum class Feature
    {
        Completion,
        Hover,
        Definition,
        Implementation,
        Declaration,
        References,
        DocumentHighlight,
        SignatureHelp,
        Formatting,
        Rename,
        CodeActions
    };

    explicit EditorLanguageFeatureController(CodeEditor *editor,
                                             QObject *parent = nullptr);

    void setClient(LspClient *client);
    void detachClient();
    bool isAvailable(Feature feature) const;
    bool handleKeyPress(QKeyEvent *event);
    bool handleMousePress(QMouseEvent *event);
    void handleMouseMove(QMouseEvent *event);
    void handleCursorPositionChanged();

    void triggerCompletion();
    void triggerSignatureHelp();
    void requestDefinition();
    void requestImplementation();
    void requestDeclaration();
    void requestReferences();
    void requestFormatting();
    void requestRename(const QString &newName);
    void requestCodeActions();

private slots:
    void requestHover();
    void requestDocumentHighlight();

private:
    CodeEditor *m_editor = nullptr;
    LspClient *m_client = nullptr;
    EditorLanguageFeedbackPresenter *m_feedbackPresenter = nullptr;
    QTimer *m_hoverTimer = nullptr;
    QTimer *m_documentHighlightTimer = nullptr;
    int m_hoverLine = -1;
    int m_hoverCharacter = -1;
};

#endif
