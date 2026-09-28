#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QIcon>
#include <QJsonArray>
#include <QJsonObject>
#include "ClangdLifecycleCoordinator.h"
#include "EditorSyntaxController.h"
#include "LanguageIdentity.h"
#include "LspDocumentCoordinator.h"
#include "LspSettingsPersistence.h"
#include "ShellCommand.h"
#include "WindowLayoutPersistence.h"

class QCloseEvent;
class QTabWidget;
class QLabel;
class QSplitter;
class QStackedWidget;
class QTimer;
class CodeEditor;
class LspClient;
class LspClientEventSource;
class LspCompletionModel;
class LspCompleter;
class ActivityBar;
class ApplicationThemeController;
class EditorLspActionController;
class EditorInteractionController;
class EditorTabCloseController;
class EditorSessionController;
class EditorFileController;
class LspEditorLifecycleController;
class LspEditorResultRouter;
class LspCompletionRouter;
class LspReferencesRouter;
class LspCodeActionRouter;
class LocationNavigator;
class EditorChromeController;
class EditorWorkspacePresentationController;
class WorkspaceCommandAvailabilityController;
class WorkspacePanelPresentationController;
class WorkspaceReplaceController;
class WorkspaceNavigationController;
class DiagnosticsPanel;
class ReferencesPanel;
class CompilerPanel;
class BreadcrumbsBar;
class FindReplaceBar;
class FileTreePanel;
class SearchPanel;
class ContextManager;
class ContextNavigationController;
class ContextWorkspaceController;
class WorkspaceRootController;
class WorkspaceRootInteractionController;
class LanguageServiceFeedbackController;
class LanguageServiceWorkspaceController;
class SidebarController;
class LspRuntimePresentationController;
class LspRuntimeController;
class LspRuntimeEventController;
class LspLogPresenter;
class SnippetManager;
class GitPanel;
class SettingsPanel;
class WorkspaceCommandController;
class WorkspaceEditApplier;
class WindowLayoutController;
class ZithRuntimeLifecycleCoordinator;
class StatusBarController;
class ShellCommandSurface;
class ShellTranslationController;
class ShellDialogController;
class BottomPanel;
class LspManagerDialog;
class WelcomeWidget;
class OutlinePanel;
struct Context;

QIcon createHeliosIcon();

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    using EditorLanguage = LanguageIdentity::Language;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void openFilePath(const QString &path);

private slots:
    CodeEditor *createTab(bool makeCurrent = true);
    void saveContextState();
    CodeEditor *currentEditor() const;
    void releaseEditor(CodeEditor *editor);
    void applyWorkspaceEdit(const QJsonObject &edit);
    void saveAllForLsp();
    void updateRunActionsEnabled();
    void appendPublishedDiagnostics();

    void applyThemeAndLanguage();
    void handleShellCommand(ShellCommand command);
    void applyEditorPreferences();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    bool lspEnabled() const;
    bool cLspEnabled() const;
    QString resolvedCLspPath() const;
    LspClient *lspClientForPath(const QString &path) const;
    bool isZithEditor(CodeEditor *editor) const;
    void updateClangdLifecycle();
    void applyTheme();
    void applyTranslations();
    void applyAppearanceChange();
    QJsonObject applyWorkspaceEditRequest(const QJsonObject &params);
    QJsonValue showMessageRequest(const QJsonObject &params);
    QJsonArray configurationRequest(const QJsonArray &items) const;
    QJsonObject lspConfiguration() const;
    void notifyLspConfigurationChanged();
    void configureLspServerRequests(LspClient *client);

    QTabWidget *m_tabWidget = nullptr;
    LspClient *m_zithLspClient = nullptr;
    LspClient *m_clangdClient = nullptr;
    LspClientEventSource *m_zithLspEvents = nullptr;
    LspClientEventSource *m_clangdEvents = nullptr;
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
    ShellDialogController *m_shellDialogs = nullptr;
    ContextManager *m_contextManager = nullptr;
    ContextNavigationController *m_contextNavigation = nullptr;
    ContextWorkspaceController *m_contextWorkspace = nullptr;
    WorkspaceRootController *m_workspaceRoot = nullptr;
    WorkspaceRootInteractionController *m_workspaceRootInteraction =
        nullptr;
    LanguageServiceFeedbackController *m_languageServiceFeedback = nullptr;
    LanguageServiceWorkspaceController *m_languageServices = nullptr;
    SidebarController *m_sidebarController = nullptr;
    LspRuntimePresentationController *m_lspPresentation = nullptr;
    LspLogPresenter *m_lspLogPresenter = nullptr;
    LspRuntimeController *m_lspRuntimeController = nullptr;
    LspRuntimeEventController *m_lspRuntimeEvents = nullptr;
    SnippetManager *m_snippetManager = nullptr;
    ZithRuntimeLifecycleCoordinator *m_zithRuntime = nullptr;
    LspDocumentCoordinator m_lspDocuments;
    ClangdLifecycleCoordinator m_clangdLifecycle;
    EditorSyntaxController m_editorSyntax;
    StatusBarController *m_statusBarController = nullptr;
    TomlLspSettingsPersistence m_lspSettingsPersistence;
    TomlWindowLayoutPersistence m_windowLayoutPersistence;
    WindowLayoutController *m_windowLayout = nullptr;
    ApplicationThemeController *m_themeController = nullptr;
    EditorInteractionController *m_editorInteraction = nullptr;
    EditorTabCloseController *m_editorTabClose = nullptr;
    EditorSessionController *m_editorSession = nullptr;
    EditorFileController *m_editorFiles = nullptr;
    EditorLspActionController *m_editorLspActions = nullptr;
    LspEditorLifecycleController *m_lspEditorLifecycle = nullptr;
    LspEditorResultRouter *m_lspEditorResults = nullptr;
    LspCompletionRouter *m_lspCompletionRouter = nullptr;
    LspReferencesRouter *m_lspReferencesRouter = nullptr;
    LspCodeActionRouter *m_lspCodeActions = nullptr;
    LocationNavigator *m_locationNavigator = nullptr;
    EditorChromeController *m_editorChrome = nullptr;
    EditorWorkspacePresentationController *m_workspacePresentation = nullptr;
    QSplitter *m_splitter;
    QString m_appliedTheme;
    QString m_appliedLocale;
    WorkspaceCommandController *m_workspaceCommands = nullptr;
    WorkspaceCommandAvailabilityController *m_workspaceCommandAvailability =
        nullptr;
    WorkspacePanelPresentationController *m_workspacePanelPresentation =
        nullptr;
    WorkspaceReplaceController *m_workspaceReplace = nullptr;
    WorkspaceNavigationController *m_workspaceNavigation = nullptr;
    WorkspaceEditApplier *m_workspaceEditApplier = nullptr;

    WelcomeWidget *m_welcomeWidget = nullptr;
    OutlinePanel *m_outlinePanel = nullptr;
    QStackedWidget *m_centralStackedWidget = nullptr;

    ShellCommandSurface *m_commandSurface = nullptr;
    ShellTranslationController *m_translationController = nullptr;
    BottomPanel *m_bottomPanel = nullptr;
};

#endif
