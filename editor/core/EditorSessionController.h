#ifndef EDITORSESSIONCONTROLLER_H
#define EDITORSESSIONCONTROLLER_H

#include "ContextManager.h"

#include <QFont>
#include <QObject>
#include <QPlainTextEdit>
#include <QString>

#include <functional>

class QTabWidget;
class CodeEditor;
class DiagnosticsPanel;
class LspCompletionModel;
class LspCompleter;
class LspLogPresenter;
class SnippetManager;
class EditorSyntaxController;
class LspDocumentCoordinator;
class EditorChromeController;

class EditorSessionController : public QObject
{
    Q_OBJECT

public:
    enum class SaveResult
    {
        Saved,
        MissingPath,
        WriteFailed
    };

    struct EditorPreferences
    {
        QFont editorFont;
        bool wordWrapEnabled = false;
        bool vimMotionsEnabled = false;
    };

    struct Dependencies
    {
        QTabWidget *tabWidget = nullptr;
        SnippetManager *snippetManager = nullptr;
        LspCompleter *completer = nullptr;
        LspCompletionModel *completionModel = nullptr;
        DiagnosticsPanel *diagnosticsPanel = nullptr;
        EditorSyntaxController *syntaxController = nullptr;
        LspDocumentCoordinator *documentCoordinator = nullptr;
        LspLogPresenter *logPresenter = nullptr;
        EditorChromeController *editorChrome = nullptr;
    };

    struct Callbacks
    {
        std::function<void(CodeEditor *)> connectEditorSignals;
        std::function<void()> updateCentralWidgetState;
        std::function<void()> refreshLspRouting;
    };

    EditorSessionController(Dependencies dependencies,
                            Callbacks callbacks,
                            QObject *parent = nullptr);

    CodeEditor *createTab(bool makeCurrent = true);
    void setEditorPreferences(const EditorPreferences &preferences);
    bool openFilePath(const QString &path);
    bool assignPath(CodeEditor *editor, const QString &path);
    SaveResult saveEditor(CodeEditor *editor, const QString &path = {});
    void releaseEditor(CodeEditor *editor);
    void saveAllForLsp();

    EditorSessionState captureState() const;
    void restoreState(const EditorSessionState &state);

signals:
    void documentSetChanged();

private:
    void updateTabMetadata(CodeEditor *editor, const QString &path);
    void openDocument(CodeEditor *editor);
    void saveDocument(CodeEditor *editor);
    void publishDocumentSetChanged();
    void appendLspLog(const QString &message) const;

    QTabWidget *m_tabWidget;
    SnippetManager *m_snippetManager;
    LspCompleter *m_completer;
    LspCompletionModel *m_completionModel;
    DiagnosticsPanel *m_diagnosticsPanel;
    EditorSyntaxController *m_syntaxController;
    LspDocumentCoordinator *m_documentCoordinator;
    LspLogPresenter *m_logPresenter;
    EditorChromeController *m_editorChrome;
    EditorPreferences m_editorPreferences;
    Callbacks m_callbacks;
    bool m_batchingDocumentSetChanges = false;
    bool m_documentSetChangedWhileBatching = false;
};

#endif
