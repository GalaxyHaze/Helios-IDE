#include "MainWindow.h"
#include "../panels/OutlinePanel.h"
#include "../panels/WelcomeWidget.h"
#include "../panels/LspManagerDialog.h"
#include "ThemeManager.h"
#include "ApplicationThemeController.h"
#include "AppearanceController.h"
#include "ClangdExecutableResolver.h"
#include "TomlSettingsStore.h"
#include "TranslationManager.h"
#include "FileIcons.h"

#include "../editor/Code.h"
#include "../editor/LspClient.h"
#include "../editor/LspCompletionModel.h"
#include "../panels/CompilerPanel.h"
#include "../panels/DiagnosticsPanel.h"
#include "../panels/FileTreePanel.h"
#include "../panels/GitPanel.h"
#include "../panels/ReferencesPanel.h"
#include "../panels/SearchPanel.h"
#include "../panels/SettingsPanel.h"
#include "../panels/BottomPanel.h"
#include "../widgets/ActivityBar.h"
#include "../widgets/BreadcrumbsBar.h"
#include "../widgets/FindReplaceBar.h"
#include "ContextManager.h"
#include "ContextNavigationController.h"
#include "ContextWorkspaceController.h"
#include "WorkspaceRootController.h"
#include "WorkspaceRootInteractionController.h"
#include "LanguageServiceFeedbackController.h"
#include "LanguageServiceWorkspaceController.h"
#include "EditorInteractionController.h"
#include "EditorTabCloseController.h"
#include "EditorLspActionController.h"
#include "EditorSessionController.h"
#include "EditorFileController.h"
#include "LspEditorLifecycleController.h"
#include "LspEditorResultRouter.h"
#include "LspCompletionRouter.h"
#include "LspReferencesRouter.h"
#include "LspCodeActionRouter.h"
#include "LspRuntimePresentationController.h"
#include "LspRuntimeController.h"
#include "LspRuntimeEventController.h"
#include "LspClientEventSource.h"
#include "LspEventSource.h"
#include "LspLogPresenter.h"
#include "LocationNavigator.h"
#include "EditorChromeController.h"
#include "EditorWorkspacePresentationController.h"
#include "StatusBarController.h"
#include "ShellCommandSurface.h"
#include "ShellTranslationController.h"
#include "ShellDialogController.h"
#include "SidebarController.h"
#include "SnippetManager.h"
#include "WorkspaceCommandController.h"
#include "WorkspaceCommandAvailabilityController.h"
#include "WorkspacePanelPresentationController.h"
#include "WorkspaceReplaceController.h"
#include "WorkspaceNavigationController.h"
#include "WindowLayoutController.h"
#include "WorkspaceEditApplier.h"
#include "ZithRuntimeLifecycleCoordinator.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QFontInfo>
#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>

#ifdef HELIOS_THEME_TIMING
#include <QElapsedTimer>
#endif

namespace {
QJsonValue configurationValue(const QJsonObject &root,
                              const QString &section)
{
  if (section.isEmpty())
    return root;

  QJsonValue value = root;
  for (const QString &part : section.split(QLatin1Char('.'),
                                           Qt::SkipEmptyParts)) {
    if (!value.isObject())
      return QJsonValue::Null;
    const QJsonObject object = value.toObject();
    if (!object.contains(part))
      return QJsonValue::Null;
    value = object.value(part);
  }
  return value;
}
} // namespace

QIcon createHeliosIcon() {
  return QIcon(QStringLiteral(":/icons/helios-icon.svg"));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_lspSettingsPersistence(TomlSettingsStore::instance()),
      m_windowLayoutPersistence(TomlSettingsStore::instance()) {
  setWindowTitle("Helios");
  setWindowIcon(createHeliosIcon());
  setMinimumHeight(600);
  resize(1100, 750);

  AppearanceController::instance().apply();

  // Initial load of external settings/themes/locales
  auto &settings = TomlSettingsStore::instance();

  m_tabWidget = new QTabWidget;
  m_tabWidget->setTabsClosable(true);
  m_tabWidget->setMovable(true);
  m_tabWidget->setDocumentMode(true);
  m_editorLspActions =
      new EditorLspActionController(m_tabWidget, this);
  m_lspEditorLifecycle =
      new LspEditorLifecycleController(m_tabWidget, &m_lspDocuments, this);

  m_breadcrumbs = new BreadcrumbsBar;
  m_findReplaceBar = new FindReplaceBar;
  m_findReplaceBar->hide();

  QWidget *editorPanel = new QWidget;
  QVBoxLayout *editorLayout = new QVBoxLayout(editorPanel);
  editorLayout->setContentsMargins(0, 0, 0, 0);
  editorLayout->setSpacing(0);
  editorLayout->addWidget(m_tabWidget, 1);
  editorLayout->addWidget(m_breadcrumbs);
  editorLayout->addWidget(m_findReplaceBar);

  m_centralStackedWidget = new QStackedWidget(this);
  m_welcomeWidget = new WelcomeWidget(this);
  m_centralStackedWidget->addWidget(m_welcomeWidget);
  m_centralStackedWidget->addWidget(editorPanel);

  m_fileTree = new FileTreePanel;
  m_searchPanel = new SearchPanel(
      []() {
        const auto &settings = TomlSettingsStore::instance();
        return WorkspaceSearch::ScanPolicy{
            settings.searchTextExtensions(),
            settings.searchExcludedDirs()};
      });
  m_gitPanel = new GitPanel;
  m_settingsPanel = new SettingsPanel;
  auto *lspManagerDialog = new LspManagerDialog(this);
  m_shellDialogs = new ShellDialogController(
      this, lspManagerDialog,
      [this]() {
        if (m_lspPresentation)
          m_lspPresentation->refreshClangd();
      },
      this);
  m_lspLogPresenter = new LspLogPresenter(
      m_settingsPanel, m_shellDialogs->lspManagerDialog(),
      [this]() { return lspEnabled(); }, this);

  m_sidePanel = new QStackedWidget;
  m_sidePanel->addWidget(m_fileTree);
  m_sidePanel->addWidget(m_searchPanel);
  m_sidePanel->addWidget(m_gitPanel);
  m_sidePanel->addWidget(m_settingsPanel);
  m_sidePanel->setMinimumWidth(180);
  m_sidePanel->setMaximumWidth(520);

  m_outlinePanel = new OutlinePanel(this);
  m_bottomPanel = new BottomPanel(this);
  m_diagnosticsPanel = m_bottomPanel->diagnostics();
  m_referencesPanel = m_bottomPanel->references();
  m_compilerPanel = m_bottomPanel->compiler();

  m_splitter = new QSplitter(Qt::Horizontal);
  m_splitter->addWidget(m_sidePanel);
  m_splitter->addWidget(m_centralStackedWidget);
  m_splitter->addWidget(m_outlinePanel);
  m_splitter->setStretchFactor(0, 0);
  m_splitter->setStretchFactor(1, 1);
  m_splitter->setStretchFactor(2, 0);
  m_splitter->setChildrenCollapsible(false);

  setCentralWidget(m_splitter);
  WindowLayoutController::Dependencies layoutDependencies;
  layoutDependencies.window = this;
  layoutDependencies.splitter = m_splitter;
  layoutDependencies.sidebar = m_sidePanel;
  layoutDependencies.outline = m_outlinePanel;
  m_windowLayout = new WindowLayoutController(
      std::move(layoutDependencies), m_windowLayoutPersistence, this);
  EditorWorkspacePresentationController::Dependencies workspaceDependencies;
  workspaceDependencies.tabs = m_tabWidget;
  workspaceDependencies.centralStack = m_centralStackedWidget;
  workspaceDependencies.welcomeWidget = m_welcomeWidget;
  workspaceDependencies.editorPanel = m_tabWidget->parentWidget();
  workspaceDependencies.breadcrumbs = m_breadcrumbs;
  workspaceDependencies.findReplaceBar = m_findReplaceBar;
  m_workspacePresentation = new EditorWorkspacePresentationController(
      std::move(workspaceDependencies), this);

  m_snippetManager = new SnippetManager(this);
  m_snippetManager->loadFromJson(":/snippets/zith-snippets.json");

  m_completionModel = new LspCompletionModel(this);
  m_completer = new LspCompleter(m_completionModel, this);

  m_zithRuntime = new ZithRuntimeLifecycleCoordinator(this);
  m_zithLspClient = m_zithRuntime->client();
  m_clangdClient = new LspClient(this);
  m_zithLspEvents = new LspClientEventSource(m_zithLspClient, this);
  m_clangdEvents = new LspClientEventSource(m_clangdClient, this);
  m_lspDocuments.setClient(EditorLanguage::Zith, m_zithLspClient);
  m_lspDocuments.setClient(EditorLanguage::CFamily, m_clangdClient);
  m_clangdLifecycle.setClient(m_clangdClient);
  m_lspEditorResults =
      new LspEditorResultRouter(m_tabWidget, m_outlinePanel, this);
  m_lspEditorResults->attach(m_zithLspClient);
  m_lspEditorResults->attach(m_clangdClient);
  LspCompletionRouter::Dependencies completionDependencies;
  completionDependencies.tabWidget = m_tabWidget;
  completionDependencies.snippetManager = m_snippetManager;
  completionDependencies.completer = m_completer;
  completionDependencies.completionModel = m_completionModel;
  m_lspCompletionRouter = new LspCompletionRouter(
      std::move(completionDependencies), [this]() { return lspEnabled(); },
      this);
  m_lspCompletionRouter->attach(m_zithLspClient);
  m_lspCompletionRouter->attach(m_clangdClient);
  WorkspaceEditApplier::Callbacks workspaceEditCallbacks;
  workspaceEditCallbacks.renameEditor =
      [this](CodeEditor *editor, const QString &path) {
        return m_editorSession && m_editorSession->assignPath(editor, path);
      };
  workspaceEditCallbacks.closeEditor = [this](CodeEditor *editor) {
    if (!m_editorSession)
      return false;
    m_editorSession->releaseEditor(editor);
    return true;
  };
  m_workspaceEditApplier =
      new WorkspaceEditApplier(m_tabWidget,
                               std::move(workspaceEditCallbacks), this);
  configureLspServerRequests(m_zithLspClient);
  configureLspServerRequests(m_clangdClient);
  m_zithRuntime->setPreferOnline(
      m_lspSettingsPersistence.useOnlineZithLsp());

  m_statusBarController =
      new StatusBarController(statusBar(), settings.vimMotionsEnabled(), this);
  m_themeController = new ApplicationThemeController(
      this, m_tabWidget, m_splitter, m_breadcrumbs, menuBar(),
      m_statusBarController, this);
  applyAppearanceChange();
  LspRuntimePresentationController::Dependencies presentationDependencies;
  presentationDependencies.settingsPanel = m_settingsPanel;
  presentationDependencies.lspManagerDialog =
      m_shellDialogs->lspManagerDialog();
  presentationDependencies.logPresenter = m_lspLogPresenter;
  presentationDependencies.statusBar = m_statusBarController;
  presentationDependencies.zithRuntime = m_zithRuntime;
  presentationDependencies.zithLspClient = m_zithLspClient;
  presentationDependencies.clangdLifecycle = &m_clangdLifecycle;
  m_lspPresentation = new LspRuntimePresentationController(
      std::move(presentationDependencies),
      [this]() { return lspEnabled(); },
      [this]() { return cLspEnabled(); },
      [this]() { return resolvedCLspPath(); }, this);
  LspRuntimeController::Callbacks runtimeCallbacks;
  runtimeCallbacks.reconcileClangd =
      [this]() { updateClangdLifecycle(); };
  runtimeCallbacks.refreshActions = [this]() { updateRunActionsEnabled(); };
  runtimeCallbacks.setRestartActionEnabled = [this](bool enabled) {
    if (m_commandSurface)
      m_commandSurface->setRestartLspEnabled(enabled);
  };
  runtimeCallbacks.confirmCacheClear = [this]() {
    return QMessageBox::question(
               this, "Clear Zith runtime cache?",
               "This removes the cached zith-lsp binary and stdlib. Helios "
               "will fetch them again on the next refresh.",
               QMessageBox::Yes | QMessageBox::Cancel,
               QMessageBox::Cancel) == QMessageBox::Yes;
  };
  runtimeCallbacks.showStatus =
      [this](const QString &message, int timeout) {
        if (m_statusBarController)
          m_statusBarController->showMessage(message, timeout);
      };
  LspRuntimeController::Dependencies runtimeDependencies;
  runtimeDependencies.settingsPanel = m_settingsPanel;
  runtimeDependencies.lspManagerDialog = m_shellDialogs->lspManagerDialog();
  runtimeDependencies.zithRuntime = m_zithRuntime;
  runtimeDependencies.clangdClient = m_clangdClient;
  runtimeDependencies.presentation = m_lspPresentation;
  runtimeDependencies.completionModel = m_completionModel;
  runtimeDependencies.diagnosticsPanel = m_diagnosticsPanel;
  runtimeDependencies.outlinePanel = m_outlinePanel;
  runtimeDependencies.settingsPersistence = &m_lspSettingsPersistence;
  m_lspRuntimeController = new LspRuntimeController(
      std::move(runtimeDependencies), std::move(runtimeCallbacks), this);
  connect(m_lspRuntimeController,
          &LspRuntimeController::configurationChanged, this,
          &MainWindow::notifyLspConfigurationChanged);
  m_editorChrome = new EditorChromeController(
      m_breadcrumbs, m_findReplaceBar, m_outlinePanel,
      [this](const QString &path) {
        switch (LanguageIdentity::forPath(path)) {
        case EditorLanguage::Zith:
          return QStringLiteral("Zith");
        case EditorLanguage::CFamily:
          return QStringLiteral("C/C++");
        default:
          return path.isEmpty()
                     ? QStringLiteral("Plain Text")
                     : QFileInfo(path).suffix();
        }
      },
      [this]() { return currentEditor(); },
      [this](const QString &uri, int version) {
        if (auto *editor = currentEditor()) {
          if (auto *client = lspClientForPath(editor->filePath());
              client && client->isReady() &&
              client->supports(LspClient::Capability::DocumentSymbol)) {
            client->requestDocumentSymbols(uri, version);
          }
        }
      },
      [this](int line, int column) {
        if (m_statusBarController)
          m_statusBarController->setEditorPosition(line, column);
      },
      [this](const QString &language) {
        if (m_statusBarController)
          m_statusBarController->setLanguage(language);
      },
      [this](const QString &title) { setWindowTitle(title); }, this);

  m_contextManager = new ContextManager(this);

  LanguageServiceWorkspaceController::Callbacks languageCallbacks;
  languageCallbacks.configuration = [this]() {
    LanguageServiceWorkspaceController::Configuration configuration;
    configuration.lspEnabled = lspEnabled();
    configuration.cFamilyEnabled = cLspEnabled();
    configuration.clangdPath = resolvedCLspPath();
    configuration.workspaceRoot =
        m_contextManager ? m_contextManager->currentRoot() : QString();
    return configuration;
  };
  languageCallbacks.presentationChanged =
      [this]() { m_lspPresentation->refreshClangd(); };
  m_languageServices = new LanguageServiceWorkspaceController(
      m_tabWidget, &m_lspDocuments, &m_clangdLifecycle,
      std::move(languageCallbacks), this);

  WorkspaceReplaceController::Callbacks replaceCallbacks;
  replaceCallbacks.findOpenEditor = [this](const QString &path) {
    for (int i = 0; i < m_tabWidget->count(); ++i) {
      auto *editor = qobject_cast<CodeEditor *>(m_tabWidget->widget(i));
      if (editor && editor->filePath() == path)
        return editor;
    }
    return static_cast<CodeEditor *>(nullptr);
  };
  replaceCallbacks.confirmReplacement =
      [this](const WorkspaceReplaceController::ReplacementConfirmation
                 &confirmation) {
        const auto answer = QMessageBox::question(
            this, "Replace in workspace",
            QString("Replace %1 matches in %2 files?\n"
                    "\"%3\" -> \"%4\"")
                .arg(confirmation.totalMatches)
                .arg(confirmation.fileCount)
                .arg(confirmation.needle, confirmation.replacement),
            QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
        return answer == QMessageBox::Yes;
      };
  replaceCallbacks.showStatus =
      [this](const QString &message, int timeout) {
        if (m_statusBarController)
          m_statusBarController->showMessage(message, timeout);
      };
  replaceCallbacks.refreshSearch = [this]() {
    if (m_searchPanel)
      m_searchPanel->setRootPath(m_searchPanel->rootPath());
  };
  m_workspaceReplace =
      new WorkspaceReplaceController(std::move(replaceCallbacks), this);
  connect(m_searchPanel, &SearchPanel::replaceAllPreviewReady,
          m_workspaceReplace, &WorkspaceReplaceController::replaceAll);

  WorkspaceRootController::Callbacks rootCallbacks;
  rootCallbacks.saveCurrentContext = [this]() { saveContextState(); };
  rootCallbacks.persistRecentProject = [](const QString &path) {
    TomlSettingsStore::instance().addRecentProject(path);
  };
  rootCallbacks.refreshRecentProjects = [this]() {
    if (m_welcomeWidget)
      m_welcomeWidget->refreshRecentProjects();
  };
  m_workspaceRoot =
      new WorkspaceRootController(m_contextManager, std::move(rootCallbacks),
                                  this);
  m_workspaceRootInteraction = new WorkspaceRootInteractionController(
      this, m_workspaceRoot, {},
      [this]() {
        return m_contextManager ? m_contextManager->currentRoot() : QString();
      },
      this);

  WorkspaceNavigationController::Callbacks navigationCallbacks;
  navigationCallbacks.openFile =
      [this](const QString &path) { openFilePath(path); };
  navigationCallbacks.openFileInNewTab = [this](const QString &path) {
    createTab();
    openFilePath(path);
  };
  navigationCallbacks.selectWorkspaceRoot =
      [this](const QString &path) {
        if (m_workspaceRoot)
          m_workspaceRoot->replaceCurrentRoot(path);
      };
  navigationCallbacks.openFolder = [this]() {
    if (m_workspaceRootInteraction)
      m_workspaceRootInteraction->openFolder();
  };
  navigationCallbacks.newProject = [this]() {
    if (m_workspaceRootInteraction)
      m_workspaceRootInteraction->newProject();
  };
  m_workspaceNavigation = new WorkspaceNavigationController(
      m_fileTree, m_welcomeWidget, m_gitPanel,
      std::move(navigationCallbacks), this);

  // The side settings panel contains runtime/LSP status and shortcut help.
  connect(m_settingsPanel, &SettingsPanel::openPreferencesRequested,
          m_shellDialogs, &ShellDialogController::togglePreferences);
  connect(m_settingsPanel, &SettingsPanel::openShortcutsRequested,
          m_shellDialogs, &ShellDialogController::toggleShortcuts);
  connect(m_settingsPanel, &SettingsPanel::openLspManagerRequested,
          m_shellDialogs, &ShellDialogController::toggleLspManager);

  m_lspPresentation->refreshRuntime(
      m_lspRuntimeController->lastError());
  m_lspPresentation->refreshClangd();
  m_shellDialogs->lspManagerDialog()->setCLspEnabled(cLspEnabled());
  m_shellDialogs->lspManagerDialog()->setUseOnlineZithLsp(
      m_lspSettingsPersistence.useOnlineZithLsp());
  m_shellDialogs->lspManagerDialog()->setCLspPath(
      m_lspSettingsPersistence.cLspPath());
  m_lspPresentation->refreshClangd();

  m_contextNavigation = new ContextNavigationController(
      m_contextManager, this, [this]() { saveContextState(); },
      m_workspaceRootInteraction,
      this);

  ContextWorkspaceController::Callbacks contextCallbacks;
  contextCallbacks.applyWorkspaceRoot = [this](const QString &root) {
    m_zithRuntime->setWorkspaceRoot(root);
    m_fileTree->setRootPath(root);
    m_searchPanel->setRootPath(root);
    m_gitPanel->setRootPath(root);
  };
  contextCallbacks.updateContextIndicator =
      [this](int index, int count) {
        if (m_statusBarController)
          m_statusBarController->setContext(index, count);
      };
  contextCallbacks.restoreSession =
      [this](const Context &context) {
        if (m_editorSession)
          m_editorSession->restoreState(context.session);
      };
  contextCallbacks.lspEnabled = [this]() { return lspEnabled(); };
  contextCallbacks.zithLspRunning = [this]() {
    return m_zithLspClient && m_zithLspClient->isRunning();
  };
  contextCallbacks.ensureLspRuntime =
      [this](bool preferCached) {
        m_lspRuntimeController->ensureRuntime(preferCached);
      };
  m_contextWorkspace = new ContextWorkspaceController(
      m_contextManager, std::move(contextCallbacks), this);
  connect(m_contextWorkspace,
          &ContextWorkspaceController::workspaceContextApplied,
          this, &MainWindow::updateClangdLifecycle);

  saveContextState();

  connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int idx) {
    auto *ed = qobject_cast<CodeEditor *>(m_tabWidget->widget(idx));
    if (m_editorChrome)
      m_editorChrome->update(ed);
  });

  m_locationNavigator = new LocationNavigator(
      [this](const QString &path) { openFilePath(path); },
      [this]() { return currentEditor(); }, this);
  m_locationNavigator->attach(m_diagnosticsPanel);
  m_locationNavigator->attach(m_outlinePanel);
  m_locationNavigator->attach(m_referencesPanel);
  m_locationNavigator->attach(m_searchPanel);
  LspReferencesRouter::Dependencies referencesDependencies;
  referencesDependencies.tabWidget = m_tabWidget;
  referencesDependencies.referencesPanel = m_referencesPanel;
  m_lspReferencesRouter = new LspReferencesRouter(
      std::move(referencesDependencies),
      [this]() { m_bottomPanel->showReferences(); }, this);
  m_lspReferencesRouter->attach(m_zithLspClient);
  m_lspReferencesRouter->attach(m_clangdClient);
  LspCodeActionRouter::Callbacks codeActionCallbacks;
  codeActionCallbacks.applyWorkspaceEdit =
      [this](const QJsonObject &edit) { applyWorkspaceEdit(edit); };
  codeActionCallbacks.executeCommand =
      [](LspClient *client, const QJsonObject &command) {
        if (!client)
          return;
        client->executeWorkspaceCommand(
            command.value("command").toString(), command.value("arguments"),
            {});
      };
  m_lspCodeActions =
      new LspCodeActionRouter(this, m_tabWidget, std::move(codeActionCallbacks),
                              this);
  m_lspCodeActions->attach(m_zithLspClient);
  m_lspCodeActions->attach(m_clangdClient);

  EditorInteractionController::Callbacks interactionCallbacks;
  interactionCallbacks.currentEditor = [this]() { return currentEditor(); };
  interactionCallbacks.saveCurrent = [this]() {
    if (m_editorFiles)
      return m_editorFiles->saveEditor(currentEditor());
    return false;
  };
  interactionCallbacks.closeEditor =
      [this](CodeEditor *editor) { releaseEditor(editor); };
  interactionCallbacks.updateCentralWidgetState =
      [this]() {
        if (m_workspacePresentation)
          m_workspacePresentation->synchronize();
      };
  interactionCallbacks.showStatus =
      [this](const QString &message, int timeout) {
        if (m_statusBarController)
          m_statusBarController->showMessage(message, timeout);
      };
  interactionCallbacks.setVimModeLabel = [this](const QString &text) {
    if (m_statusBarController)
      m_statusBarController->setVimMode(text);
  };
  EditorInteractionController::Dependencies interactionDependencies;
  interactionDependencies.lspActions = m_editorLspActions;
  interactionDependencies.editorChrome = m_editorChrome;
  interactionDependencies.locationNavigator = m_locationNavigator;
  m_editorInteraction = new EditorInteractionController(
      std::move(interactionDependencies), std::move(interactionCallbacks),
      this);

  EditorSessionController::Callbacks sessionCallbacks;
  sessionCallbacks.connectEditorSignals =
      [this](CodeEditor *editor) { m_editorInteraction->attach(editor); };
  sessionCallbacks.updateCentralWidgetState =
      [this]() {
        if (m_workspacePresentation)
          m_workspacePresentation->synchronize();
      };
  sessionCallbacks.refreshLspRouting = [this]() {
    if (m_languageServices)
      m_languageServices->refreshRouting();
  };
  EditorSessionController::Dependencies sessionDependencies;
  sessionDependencies.tabWidget = m_tabWidget;
  sessionDependencies.snippetManager = m_snippetManager;
  sessionDependencies.completer = m_completer;
  sessionDependencies.completionModel = m_completionModel;
  sessionDependencies.diagnosticsPanel = m_diagnosticsPanel;
  sessionDependencies.syntaxController = &m_editorSyntax;
  sessionDependencies.documentCoordinator = &m_lspDocuments;
  sessionDependencies.logPresenter = m_lspLogPresenter;
  sessionDependencies.editorChrome = m_editorChrome;
  m_editorSession = new EditorSessionController(
      std::move(sessionDependencies), std::move(sessionCallbacks), this);
  applyEditorPreferences();
  connect(m_editorSession, &EditorSessionController::documentSetChanged,
          this, &MainWindow::updateClangdLifecycle);
  EditorFileController::Callbacks fileCallbacks;
  fileCallbacks.workspaceRoot = [this]() {
    return m_contextManager ? m_contextManager->currentRoot() : QString();
  };
  fileCallbacks.showStatus =
      [this](const QString &message, int timeout) {
        if (m_statusBarController)
          m_statusBarController->showMessage(message, timeout);
      };
  fileCallbacks.refreshCommandAvailability =
      [this]() { updateRunActionsEnabled(); };
  m_editorFiles = new EditorFileController(
      this, m_tabWidget, m_editorSession, std::move(fileCallbacks), this);
  EditorTabCloseController::Callbacks tabCloseCallbacks;
  tabCloseCallbacks.requestSaveDecision = [this](CodeEditor *editor) {
    const auto result = QMessageBox::question(
        this, "Save?",
        QString("Save changes to '%1'?")
            .arg(editor->filePath().isEmpty() ? "Untitled" : editor->filePath()),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (result) {
    case QMessageBox::Save:
      return EditorTabCloseController::SaveDecision::Save;
    case QMessageBox::Discard:
      return EditorTabCloseController::SaveDecision::Discard;
    default:
      return EditorTabCloseController::SaveDecision::Cancel;
    }
  };
  tabCloseCallbacks.saveEditor = [this](CodeEditor *editor) {
    if (m_editorFiles)
      return m_editorFiles->saveEditor(editor);
    return false;
  };
  tabCloseCallbacks.releaseEditor =
      [this](CodeEditor *editor) { releaseEditor(editor); };
  m_editorTabClose = new EditorTabCloseController(
      m_tabWidget, std::move(tabCloseCallbacks), this);
  WorkspaceCommandController::Callbacks commandCallbacks;
  commandCallbacks.lspEnabled = [this]() { return lspEnabled(); };
  commandCallbacks.workspaceRoot = [this]() {
    return m_contextManager ? m_contextManager->currentRoot() : QString();
  };
  commandCallbacks.currentEditor = [this]() { return currentEditor(); };
  commandCallbacks.isZithEditor =
      [this](CodeEditor *editor) { return isZithEditor(editor); };
  commandCallbacks.saveAll = [this]() { saveAllForLsp(); };
  commandCallbacks.showCompiler = [this]() { m_bottomPanel->showCompiler(); };
  commandCallbacks.appendPublishedDiagnostics =
      [this]() { appendPublishedDiagnostics(); };
  commandCallbacks.showStatus = [this](const QString &message, int timeout) {
    if (m_statusBarController)
      m_statusBarController->showMessage(message, timeout);
  };
  WorkspaceCommandController::Dependencies commandDependencies;
  commandDependencies.client = m_zithLspClient;
  commandDependencies.events = m_zithLspEvents;
  commandDependencies.compilerPanel = m_compilerPanel;
  m_workspaceCommands = new WorkspaceCommandController(
      std::move(commandDependencies), std::move(commandCallbacks), this);
  LspRuntimeEventController::Callbacks runtimeEventCallbacks;
  runtimeEventCallbacks.lspEnabled = [this]() { return lspEnabled(); };
  runtimeEventCallbacks.refreshActions =
      [this]() { updateRunActionsEnabled(); };
  LspRuntimeEventController::Dependencies eventDependencies;
  eventDependencies.zithRuntime = m_zithRuntime;
  eventDependencies.zithClient = m_zithLspClient;
  eventDependencies.zithEvents = m_zithLspEvents;
  eventDependencies.clangdClient = m_clangdClient;
  eventDependencies.clangdEvents = m_clangdEvents;
  eventDependencies.clangdLifecycle = &m_clangdLifecycle;
  eventDependencies.runtimeController = m_lspRuntimeController;
  eventDependencies.presentation = m_lspPresentation;
  eventDependencies.editorLifecycle = m_lspEditorLifecycle;
  eventDependencies.logPresenter = m_lspLogPresenter;
  eventDependencies.statusBar = m_statusBarController;
  m_lspRuntimeEvents = new LspRuntimeEventController(
      std::move(eventDependencies), std::move(runtimeEventCallbacks), this);
  m_lspRuntimeEvents->attach();
  addDockWidget(Qt::BottomDockWidgetArea, m_bottomPanel);
  m_bottomPanel->hide();

  m_activityBar = new ActivityBar(this);
  addDockWidget(Qt::LeftDockWidgetArea, m_activityBar);
  m_sidebarController = new SidebarController(
      m_activityBar, m_sidePanel, m_gitPanel,
      [this](bool visible) {
        if (m_windowLayout)
          m_windowLayout->setSidebarVisible(visible);
      },
      [this]() { m_lspPresentation->refreshClangd(); },
      [this]() {
        if (m_fileTree)
          m_fileTree->setFocus();
      },
      this);
  connect(m_activityBar, &ActivityBar::preferencesRequested, m_shellDialogs,
          &ShellDialogController::togglePreferences);

  LanguageServiceFeedbackController::Callbacks feedbackCallbacks;
  feedbackCallbacks.applyWorkspaceEdit =
      [this](const QJsonObject &edit) { applyWorkspaceEdit(edit); };
  feedbackCallbacks.showStatus = [this](const QString &message, int timeout) {
    if (m_statusBarController)
      m_statusBarController->showMessage(message, timeout);
  };
  feedbackCallbacks.updateDiagnosticCounts =
      [this](int errors, int warnings) {
        if (m_statusBarController)
          m_statusBarController->setDiagnostics(errors, warnings);
      };
  m_languageServiceFeedback = new LanguageServiceFeedbackController(
      m_diagnosticsPanel, std::move(feedbackCallbacks), this);
  m_languageServiceFeedback->attach(m_zithLspEvents);
  m_languageServiceFeedback->attach(m_clangdEvents);

  connect(m_compilerPanel, &CompilerPanel::stopRequested, m_workspaceCommands,
          &WorkspaceCommandController::stopRunningTask);
  connect(m_tabWidget, &QTabWidget::currentChanged, this,
          [this](int) { updateRunActionsEnabled(); });

  m_windowLayout->restore();

  m_commandSurface = new ShellCommandSurface(menuBar(), this);
  connect(m_commandSurface, &ShellCommandSurface::commandRequested, this,
          &MainWindow::handleShellCommand);
  m_commandSurface->installShortcuts(this);
  m_translationController =
      new ShellTranslationController(m_activityBar, m_commandSurface, this);
  WorkspacePanelPresentationController::CurrentEditor currentEditor =
      [this]() { return this->currentEditor(); };
  WorkspacePanelPresentationController::PersistOutlineVisibility
      persistOutlineVisibility = [this](bool visible) {
        if (m_windowLayout)
          m_windowLayout->setOutlineVisible(visible);
      };
  WorkspacePanelPresentationController::Dependencies panelDependencies;
  panelDependencies.outlinePanel = m_outlinePanel;
  panelDependencies.bottomPanel = m_bottomPanel;
  panelDependencies.commandSurface = m_commandSurface;
  panelDependencies.editorChrome = m_editorChrome;
  m_workspacePanelPresentation = new WorkspacePanelPresentationController(
      std::move(panelDependencies), std::move(currentEditor),
      std::move(persistOutlineVisibility), this);
  m_workspaceCommandAvailability =
      new WorkspaceCommandAvailabilityController(
          m_commandSurface, this);
  connect(m_workspaceCommands, &WorkspaceCommandController::stateChanged,
          this, [this](const WorkspaceCommandState &state) {
            if (m_workspaceCommandAvailability)
              m_workspaceCommandAvailability->refresh(state);
          });
  updateRunActionsEnabled();

  // Apply active theme/language and load last recent workspace on startup
  applyThemeAndLanguage();
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
          [this]() { applyTheme(); });
  connect(&AppearanceController::instance(), &AppearanceController::appearanceChanged,
          this, &MainWindow::applyAppearanceChange);
  connect(&TomlSettingsStore::instance(),
          &TomlSettingsStore::editorPreferencesChanged, this,
          &MainWindow::applyEditorPreferences);
  connect(&TranslationManager::instance(), &TranslationManager::localeChanged,
          this, [this]() {
            m_appliedLocale = TomlSettingsStore::instance().locale();
            applyTranslations();
          });

  QStringList recents = settings.recentProjects();
  QString lastProj;
  for (const QString &proj : recents) {
    if (!proj.isEmpty() && QFileInfo::exists(proj)) {
      lastProj = proj;
      break;
    }
  }

  if (!lastProj.isEmpty() && m_workspaceRoot)
    m_workspaceRoot->restoreCurrentRoot(lastProj);

  // Hide/show sidebar from saved state and synchronize its active mode.
  m_sidebarController->setVisible(settings.sidebarVisible());

  if (lspEnabled()) {
    m_lspRuntimeController->initialize(true);
  } else {
    m_lspRuntimeController->initialize(false);
  }

  // Initial central stack update
  m_workspacePresentation->synchronize();

  // Trigger onboarding dialog if not dismissed on startup
  if (!settings.onboardingDismissed()) {
    QTimer::singleShot(500, this, [this]() {
      m_shellDialogs->showGettingStarted();
    });
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  saveContextState();
  if (m_windowLayout)
    m_windowLayout->save();
  QMainWindow::closeEvent(event);
}

void MainWindow::openFilePath(const QString &path) {
  if (m_editorSession)
    m_editorSession->openFilePath(path);
}

CodeEditor *MainWindow::createTab(bool makeCurrent) {
  return m_editorSession ? m_editorSession->createTab(makeCurrent) : nullptr;
}

void MainWindow::saveContextState() {
  if (m_contextManager && m_editorSession)
    m_contextManager->setContextState(m_contextManager->currentIndex(),
                                      m_editorSession->captureState());
}

CodeEditor *MainWindow::currentEditor() const {
  if (m_tabWidget->count() == 0)
    return nullptr;
  return qobject_cast<CodeEditor *>(m_tabWidget->currentWidget());
}

void MainWindow::handleShellCommand(ShellCommand command)
{
  if (m_workspaceCommands &&
      m_workspaceCommands->handleShellCommand(command))
    return;
  if (m_editorLspActions &&
      m_editorLspActions->handleShellCommand(command))
    return;
  if (m_lspRuntimeController &&
      m_lspRuntimeController->handleShellCommand(command))
    return;
  if (m_workspacePanelPresentation &&
      m_workspacePanelPresentation->handleShellCommand(command))
    return;
  if (m_workspacePresentation &&
      m_workspacePresentation->handleShellCommand(command))
    return;
  if (m_sidebarController &&
      m_sidebarController->handleShellCommand(command))
    return;
  if (m_shellDialogs &&
      m_shellDialogs->handleShellCommand(command))
    return;
  if (m_workspaceNavigation &&
      m_workspaceNavigation->handleShellCommand(command))
    return;
  if (m_editorFiles && m_editorFiles->handleShellCommand(command))
    return;

  switch (command) {
  case ShellCommand::NewWindow: {
    auto *window = new MainWindow;
    window->show();
    break;
  }
  case ShellCommand::Exit:
    close();
    break;
  default:
    break;
  }
}

void MainWindow::applyThemeAndLanguage() {
  auto &store = TomlSettingsStore::instance();
  bool themeChanged = (store.theme() != m_appliedTheme);
  bool localeChanged = (store.locale() != m_appliedLocale);

  if (!themeChanged && !localeChanged)
    return;

  if (themeChanged) {
    if (store.theme() == "custom" && !store.customThemePath().isEmpty())
      ThemeManager::instance().loadThemeFile(store.customThemePath(), "Custom Theme");
    else
      ThemeManager::instance().loadTheme(store.theme());
    m_appliedTheme = store.theme();
    applyTheme();
  }

  if (localeChanged) {
    TranslationManager::instance().loadLocale(store.locale());
    m_appliedLocale = store.locale();
    applyTranslations();
  }
}

void MainWindow::applyTheme() {
  if (m_themeController)
    m_themeController->apply();
}

void MainWindow::applyTranslations() {
  if (m_translationController)
    m_translationController->apply();
}

void MainWindow::applyAppearanceChange()
{
  applyEditorPreferences();
  applyTheme();
}

void MainWindow::applyEditorPreferences()
{
  if (!m_editorSession)
    return;

  const auto &settings = TomlSettingsStore::instance();
  m_editorSession->setEditorPreferences(
      {AppearanceController::instance().editorFont(), settings.wordWrap(),
       settings.vimMotionsEnabled()});
}

bool MainWindow::lspEnabled() const {
  return m_lspSettingsPersistence.lspEnabled();
}

bool MainWindow::cLspEnabled() const {
  return m_lspSettingsPersistence.cLspEnabled();
}

QString MainWindow::resolvedCLspPath() const {
  return ClangdExecutableResolver::resolve(
      m_lspSettingsPersistence.cLspPath(),
      QProcessEnvironment::systemEnvironment().value("PATH"));
}

LspClient *MainWindow::lspClientForPath(const QString &path) const {
  return m_languageServices ? m_languageServices->clientForPath(path)
                            : nullptr;
}

bool MainWindow::isZithEditor(CodeEditor *editor) const {
  return m_languageServices &&
         m_languageServices->isEditorLanguage(
             editor, EditorLanguage::Zith);
}

void MainWindow::updateClangdLifecycle() {
  if (m_languageServices)
    m_languageServices->reconcile();
}

void MainWindow::releaseEditor(CodeEditor *editor) {
  if (m_editorSession)
    m_editorSession->releaseEditor(editor);
}

void MainWindow::saveAllForLsp() {
  if (m_editorSession)
    m_editorSession->saveAllForLsp();
}

void MainWindow::updateRunActionsEnabled() {
  if (!m_workspaceCommandAvailability || !m_workspaceCommands)
    return;
  m_workspaceCommandAvailability->refresh(m_workspaceCommands->state());
}

void MainWindow::appendPublishedDiagnostics() {
  if (!m_compilerPanel || !m_diagnosticsPanel)
    return;
  QList<LspDiagnostic> diagnostics;
  const QList<LspDiagnostic> published = m_diagnosticsPanel->allDiagnostics();
  for (const LspDiagnostic &diagnostic : published) {
    if (diagnostic.severity != 0 || !diagnostic.message.isEmpty())
      diagnostics.append(diagnostic);
  }
  if (diagnostics.isEmpty())
    return;
  if (m_compilerPanel)
    m_compilerPanel->appendDiagnostics(diagnostics);
}

void MainWindow::applyWorkspaceEdit(const QJsonObject &edit) {
  if (!m_workspaceEditApplier)
    return;
  const WorkspaceEditApplier::Result result =
      m_workspaceEditApplier->apply(edit);
  if (!result.applied)
    m_statusBarController->showMessage(result.error, 5000);
}

QJsonObject MainWindow::applyWorkspaceEditRequest(
    const QJsonObject &params) {
  if (!m_workspaceEditApplier)
    return {{"applied", false},
            {"failureReason", QStringLiteral("Workspace edit unavailable.")}};

  const QJsonObject edit = params.value(QStringLiteral("edit")).toObject();
  const WorkspaceEditApplier::Result result =
      m_workspaceEditApplier->apply(edit);
  if (!result.applied && m_statusBarController)
    m_statusBarController->showMessage(result.error, 5000);

  QJsonObject response{{QStringLiteral("applied"), result.applied}};
  if (!result.applied && !result.error.isEmpty())
    response.insert(QStringLiteral("failureReason"), result.error);
  return response;
}

QJsonValue MainWindow::showMessageRequest(const QJsonObject &params) {
  const QString message = params.value(QStringLiteral("message")).toString();
  const QJsonArray actions = params.value(QStringLiteral("actions")).toArray();
  if (actions.isEmpty()) {
    QMessageBox::information(this, QStringLiteral("Language Server"),
                             message);
    return QJsonValue::Null;
  }

  QMessageBox box(this);
  const int type = params.value(QStringLiteral("type")).toInt(3);
  switch (type) {
  case 1:
    box.setIcon(QMessageBox::Critical);
    break;
  case 2:
    box.setIcon(QMessageBox::Warning);
    break;
  case 4:
    box.setIcon(QMessageBox::NoIcon);
    break;
  default:
    box.setIcon(QMessageBox::Information);
    break;
  }
  box.setWindowTitle(QStringLiteral("Language Server"));
  box.setText(message);

  QList<QPair<QPushButton *, QJsonObject>> buttons;
  for (const QJsonValue &value : actions) {
    const QJsonObject action = value.toObject();
    const QString title =
        action.value(QStringLiteral("title")).toString();
    if (title.isEmpty())
      continue;
    auto *button = box.addButton(title, QMessageBox::AcceptRole);
    buttons.append({button, action});
  }

  if (buttons.isEmpty())
    return QJsonValue::Null;
  box.exec();
  for (const auto &button : buttons) {
    if (box.clickedButton() == button.first)
      return button.second;
  }
  return QJsonValue::Null;
}

QJsonArray MainWindow::configurationRequest(const QJsonArray &items) const {
  const QJsonObject configuration = lspConfiguration();
  QJsonArray values;
  for (const QJsonValue &item : items) {
    const QString section =
        item.toObject().value(QStringLiteral("section")).toString();
    values.append(configurationValue(configuration, section));
  }
  return values;
}

QJsonObject MainWindow::lspConfiguration() const
{
  const auto &settings = TomlSettingsStore::instance();
  const QJsonObject editor{
      {QStringLiteral("wordWrap"), settings.wordWrap()},
      {QStringLiteral("vimMotionsEnabled"), settings.vimMotionsEnabled()},
      {QStringLiteral("tabSize"), 4},
      {QStringLiteral("insertSpaces"), true}};
  const QJsonObject lsp{
      {QStringLiteral("enabled"), settings.lspEnabled()},
      {QStringLiteral("useOnlineZithLsp"), settings.useOnlineZithLsp()},
      {QStringLiteral("cFamilyEnabled"), settings.cLspEnabled()}};
  const QJsonObject zith{
      {QStringLiteral("enabled"), settings.lspEnabled()},
      {QStringLiteral("useOnline"), settings.useOnlineZithLsp()}};
  const QJsonObject clangd{
      {QStringLiteral("enabled"), settings.cLspEnabled()},
      {QStringLiteral("path"), settings.cLspPath()},
      {QStringLiteral("resolvedPath"), resolvedCLspPath()}};
  return {
      {QStringLiteral("helios"),
       QJsonObject{{QStringLiteral("locale"), settings.locale()},
                   {QStringLiteral("lsp"), lsp},
                   {QStringLiteral("editor"), editor},
                   {QStringLiteral("zith"), zith},
                   {QStringLiteral("clangd"), clangd}}},
      {QStringLiteral("editor"), editor},
      {QStringLiteral("lsp"), lsp},
      {QStringLiteral("zith"), zith},
      {QStringLiteral("clangd"), clangd}};
}

void MainWindow::notifyLspConfigurationChanged()
{
  const QJsonObject settings = lspConfiguration();
  if (m_zithLspClient)
    m_zithLspClient->notifyConfigurationChanged(settings);
  if (m_clangdClient)
    m_clangdClient->notifyConfigurationChanged(settings);
}

void MainWindow::configureLspServerRequests(LspClient *client) {
  if (!client)
    return;

  connect(client, &LspClient::initialized, this, [this, client]() {
    client->notifyConfigurationChanged(lspConfiguration());
  });

  LspClient::ServerRequestHandlers handlers;
  handlers.applyWorkspaceEdit =
      [this](const QJsonObject &params) {
        return applyWorkspaceEditRequest(params);
      };
  handlers.showMessageRequest =
      [this](const QJsonObject &params) { return showMessageRequest(params); };
  handlers.configuration =
      [this](const QJsonArray &items) { return configurationRequest(items); };
  client->setServerRequestHandlers(std::move(handlers));
}
