#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QList>
#include <QIcon>
#include <QJsonObject>
#include "../widgets/ActivityBar.h"

class QTabWidget;
class QLabel;
class QSplitter;
class QStackedWidget;
class QMenu;
class QAction;
class CodeEditor;
class SyntaxHighlighter;
class LspClient;
class LspCompletionModel;
class LspCompleter;
class DiagnosticsPanel;
class ReferencesPanel;
class CompilerPanel;
class BreadcrumbsBar;
class FindReplaceBar;
class FileTreePanel;
class ContextManager;
class SnippetManager;
class SearchPanel;
class GitPanel;
class SettingsPanel;
class ShortcutsDialog;
class PreferencesDialog;
class VimHelpDialog;
class LspManagerDialog;
class ZithToolchainManager;
class WelcomeWidget;
class OutlinePanel;
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
    void openFolder();
    void saveFile();
    void toggleFileTree(bool show);
    void saveContextState();
    void restoreContextState(const Context &ctx);
    CodeEditor *currentEditor() const;
    void setSidebarMode(ActivityBar::Mode mode);
    void applyEditorPreferences(CodeEditor *editor);
    void setAppFontFamily(const QString &family);
    void setAppFontSize(int pointSize);
    void setWordWrapEnabled(bool enabled);
    void loadUiPreferences();
    void saveUiPreferences() const;
    void updateEditorChrome(CodeEditor *editor);
    void releaseEditor(CodeEditor *editor);
    void setLspStatus(const QString &text, const QString &color);
    void onFrontendStatusReceived(const QJsonObject &status);
    void onMetricsReceived(const QJsonObject &metrics);
    void ensureLspRuntime(bool preferCached);
    void startLspRuntime(const QString &lspPath,
                         const QString &stdlibPath,
                         const QString &tag);
    void updateSettingsRuntimeInfo();
    void updateLspDiagnostics();
    void clearRuntimeCache();
    void setLspEnabled(bool enabled);
    void applyWorkspaceEdit(const QJsonObject &edit);
    void saveAllForLsp();

    void applyThemeAndLanguage();
    void updateCentralWidgetState();
    void showPreferences();
    void showSettingsPanel();
    void showShortcutsDialog();
    void showLspManagerDialog();
    void showVimHelpDialog();

private:
    void setWorkspaceRoot(const QString &path);
    bool lspEnabled() const;
    void applyTheme();
    void applyTranslations();
    void applyAppearanceChange();

    QTabWidget *m_tabWidget = nullptr;
    QMap<CodeEditor*, SyntaxHighlighter*> m_highlighters;
    LspClient *m_lspClient = nullptr;
    LspCompletionModel *m_completionModel = nullptr;
    LspCompleter *m_completer = nullptr;
    DiagnosticsPanel *m_diagnosticsPanel = nullptr;
    ReferencesPanel *m_referencesPanel = nullptr;
    CompilerPanel *m_compilerPanel = nullptr;
    BreadcrumbsBar *m_breadcrumbs = nullptr;
    FindReplaceBar *m_findReplaceBar = nullptr;
    ActivityBar *m_activityBar = nullptr;
    FileTreePanel *m_fileTree = nullptr;
    QStackedWidget *m_sidePanel = nullptr;
    SearchPanel *m_searchPanel = nullptr;
    GitPanel *m_gitPanel = nullptr;
    SettingsPanel *m_settingsPanel = nullptr;
    ShortcutsDialog *m_shortcutsDialog = nullptr;
    PreferencesDialog *m_preferencesDialog = nullptr;
    VimHelpDialog *m_vimHelpDialog = nullptr;
    LspManagerDialog *m_lspManagerDialog = nullptr;
    ContextManager *m_contextManager = nullptr;
    SnippetManager *m_snippetManager = nullptr;
    ZithToolchainManager *m_zithToolchainManager = nullptr;
    QString m_lastLspError;
    bool m_initialContextSetup;
    bool m_replacingWorkspaceRoot = false;
    QSplitter *m_splitter;
    int m_fileTreeWidth = 280;
    bool m_fileTreeAnimating = false;
    QString m_appFontFamily = "System Default";
    int m_appFontSize = 13;
    bool m_wordWrapEnabled = false;
    QLabel *m_lspLabel = nullptr;
    QLabel *m_vimLabel = nullptr;
    QLabel *m_contextLabel = nullptr;
    QLabel *m_errorLabel = nullptr;
    QLabel *m_posLabel = nullptr;
    QLabel *m_indentLabel = nullptr;
    QLabel *m_encodingLabel = nullptr;
    QLabel *m_langLabel = nullptr;
    QString m_runtimeStatusText;
    QString m_runtimeTag;
    QString m_activeLspPath;
    QString m_activeStdlibPath;
    QString m_activeWorkspaceRoot;
    QString m_appliedTheme;
    QString m_appliedLocale;
    QList<qint64> m_lspRestartTimes;

    WelcomeWidget *m_welcomeWidget = nullptr;
    OutlinePanel *m_outlinePanel = nullptr;
    QStackedWidget *m_centralStackedWidget = nullptr;

    QMenu *m_fileMenu = nullptr;
    QMenu *m_toolsMenu = nullptr;
    QMenu *m_viewMenu = nullptr;
    QMenu *m_helpMenu = nullptr;

    QAction *m_newAct = nullptr;
    QAction *m_newWinAct = nullptr;
    QAction *m_newProjAct = nullptr;
    QAction *m_openAct = nullptr;
    QAction *m_saveAct = nullptr;
    QAction *m_exitAct = nullptr;
    QAction *m_buildAct = nullptr;
    QAction *m_checkAct = nullptr;
    QAction *m_compileAct = nullptr;
    QAction *m_runAct = nullptr;
    QAction *m_restartLspAct = nullptr;
    QAction *m_gettingStartedAct = nullptr;
    QAction *m_preferencesAct = nullptr;
    
    QAction *m_shortcutsAct = nullptr;
    QAction *m_lspManagerAct = nullptr;
    QAction *m_vimHelpAct = nullptr;
};

#endif
