#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QIcon>
#include "ActivityBar.h"

class QTabWidget;
class QLabel;
class QSplitter;
class QStackedWidget;
class CodeEditor;
class SyntaxHighlighter;
class LspClient;
class LspCompletionModel;
class LspCompleter;
class DiagnosticsPanel;
class CompilerPanel;
class BreadcrumbsBar;
class FindReplaceBar;
class FileTreePanel;
class ContextManager;
class SnippetManager;
class SearchPanel;
class GitPanel;
class SettingsPanel;
struct Context;

QIcon createHeliosIcon();

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void openFilePath(const QString &path);

private slots:
    CodeEditor *createTab(bool makeCurrent = true);
    void connectEditorSignals(CodeEditor *editor);
    void newFile();
    void newProject();
    void openFile();
    void saveFile();
    void toggleFileTree(bool show);
    void saveContextState();
    void restoreContextState(const Context &ctx);
    CodeEditor *currentEditor() const;
    void setSidebarMode(ActivityBar::Mode mode);
    void applyEditorPreferences(CodeEditor *editor);
    void setEditorFontSize(int pointSize);
    void setWordWrapEnabled(bool enabled);
    void loadUiPreferences();
    void saveUiPreferences() const;
    void updateEditorChrome(CodeEditor *editor);
    void releaseEditor(CodeEditor *editor);

private:
    QTabWidget *m_tabWidget = nullptr;
    QMap<CodeEditor*, SyntaxHighlighter*> m_highlighters;
    LspClient *m_lspClient = nullptr;
    LspCompletionModel *m_completionModel = nullptr;
    LspCompleter *m_completer = nullptr;
    DiagnosticsPanel *m_diagnosticsPanel = nullptr;
    CompilerPanel *m_compilerPanel = nullptr;
    BreadcrumbsBar *m_breadcrumbs = nullptr;
    FindReplaceBar *m_findReplaceBar = nullptr;
    ActivityBar *m_activityBar = nullptr;
    FileTreePanel *m_fileTree = nullptr;
    QStackedWidget *m_sidePanel = nullptr;
    SearchPanel *m_searchPanel = nullptr;
    GitPanel *m_gitPanel = nullptr;
    SettingsPanel *m_settingsPanel = nullptr;
    ContextManager *m_contextManager = nullptr;
    SnippetManager *m_snippetManager = nullptr;
    bool m_initialContextSetup;
    QSplitter *m_splitter;
    int m_fileTreeWidth = 220;
    bool m_fileTreeAnimating = false;
    int m_editorFontSize = 12;
    bool m_wordWrapEnabled = false;
    QLabel *m_lspLabel = nullptr;
    QLabel *m_contextLabel = nullptr;
    QLabel *m_errorLabel = nullptr;
    QLabel *m_posLabel = nullptr;
    QLabel *m_indentLabel = nullptr;
    QLabel *m_encodingLabel = nullptr;
    QLabel *m_langLabel = nullptr;
};

#endif
