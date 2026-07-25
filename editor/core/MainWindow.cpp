#include "MainWindow.h"
#include "../panels/OutlinePanel.h"
#include "../panels/WelcomeWidget.h"
#include "../panels/ShortcutsDialog.h"
#include "../panels/LspManagerDialog.h"
#include "ThemeManager.h"
#include "AppearanceController.h"
#include "TomlSettingsStore.h"
#include "TranslationManager.h"

#include "../editor/Code.h"
#include "../editor/LspClient.h"
#include "../editor/LspCompletionModel.h"
#include "../editor/Syntax.h"
#include "../panels/CompilerPanel.h"
#include "../panels/DiagnosticsPanel.h"
#include "../panels/FileTreePanel.h"
#include "../panels/GitPanel.h"
#include "../panels/ReferencesPanel.h"
#include "../panels/SearchPanel.h"
#include "../panels/SettingsPanel.h"
#include "../panels/PreferencesDialog.h"
#include "../panels/ShortcutsDialog.h"
#include "../panels/LspManagerDialog.h"
#include "../panels/VimHelpDialog.h"
#include "../widgets/BreadcrumbsBar.h"
#include "../widgets/FindReplaceBar.h"
#include "ContextManager.h"
#include "SnippetManager.h"
#include "ZithToolchainManager.h"

#include <QApplication>
#include <QCheckBox>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QJsonDocument>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>
#include <QScrollBar>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <algorithm>

#ifdef HELIOS_THEME_TIMING
#include <QElapsedTimer>
#endif

namespace {
void setStyleSheetIfChanged(QWidget *widget, const QString &styleSheet) {
  if (widget->styleSheet() != styleSheet)
    widget->setStyleSheet(styleSheet);
}

int offsetForPosition(const QString &text, const LspPosition &position,
                      bool *valid) {
  if (position.line < 0 || position.character < 0) {
    *valid = false;
    return 0;
  }
  int line = 0;
  int offset = 0;
  while (line < position.line && offset < text.size()) {
    const int newline = text.indexOf(QLatin1Char('\n'), offset);
    if (newline < 0) {
      *valid = false;
      return 0;
    }
    offset = newline + 1;
    ++line;
  }
  if (line != position.line) {
    *valid = false;
    return 0;
  }
  const int end = text.indexOf(QLatin1Char('\n'), offset);
  const int lineEnd = end < 0 ? text.size() : end;
  if (offset + position.character > lineEnd) {
    *valid = false;
    return 0;
  }
  return offset + position.character;
}

LspRange rangeFromJson(const QJsonObject &range) {
  const QJsonObject start = range.value("start").toObject();
  const QJsonObject end = range.value("end").toObject();
  return {{start.value("line").toInt(), start.value("character").toInt()},
          {end.value("line").toInt(), end.value("character").toInt()}};
}
} // namespace

class GettingStartedDialog : public QDialog {
public:
  explicit GettingStartedDialog(QWidget *parent = nullptr) : QDialog(parent) {
    setWindowTitle("Getting Started with Helios");
    resize(500, 400);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    auto *title = new QLabel("Welcome to Helios!", this);
    title->setStyleSheet("font-size: 20px; font-weight: bold;");
    layout->addWidget(title);

    auto *intro = new QLabel("Helios is a high-performance C++/Zith "
                             "development editor inspired by JetBrains CLion.",
                             this);
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto *shortcutsGroup = new QGroupBox("Core Shortcuts", this);
    auto *sgLayout = new QGridLayout(shortcutsGroup);
    sgLayout->setSpacing(10);

    int row = 0;
    auto addShortcut = [&](const QString &desc, const QString &keys) {
      auto *descLbl = new QLabel(desc, this);
      auto *keyLbl = new QLabel(keys, this);
      keyLbl->setStyleSheet("font-weight: bold; color: #7287fd;");
      sgLayout->addWidget(descLbl, row, 0);
      sgLayout->addWidget(keyLbl, row, 1);
      row++;
    };

    addShortcut("Open Folder", "Ctrl+Alt+O");
    addShortcut("New File", "Ctrl+N");
    addShortcut("Toggle Project Explorer", "Ctrl+Shift+E");
    addShortcut("Search Workspace", "Ctrl+Shift+F");
    addShortcut("Build Project / Compile", "Ctrl+B");
    addShortcut("Go to Definition", "Ctrl+Click or F12");

    layout->addWidget(shortcutsGroup);

    auto *tipsGroup = new QGroupBox("Quick Tips", this);
    auto *tgLayout = new QVBoxLayout(tipsGroup);
    tgLayout->setSpacing(5);
    tgLayout->addWidget(
        new QLabel("• Double-click files in the Explorer to open them.", this));
    tgLayout->addWidget(new QLabel(
        "• " + TranslationManager::instance().translate("welcome.tip_theme"),
        this));
    tgLayout->addWidget(new QLabel("• Right-click in the editor or file "
                                   "explorer to access rich context actions.",
                                   this));
    layout->addWidget(tipsGroup);

    auto *bottomLayout = new QHBoxLayout();
    auto *dontShowAgain = new QCheckBox("Do not show this on startup", this);
    dontShowAgain->setChecked(
        TomlSettingsStore::instance().onboardingDismissed());
    bottomLayout->addWidget(dontShowAgain);

    bottomLayout->addStretch();
    auto *closeBtn = new QPushButton("Close", this);
    closeBtn->setStyleSheet(
        "background: #7287fd; color: white; padding: 6px 15px; border: none; "
        "border-radius: 4px; font-weight: bold;");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    bottomLayout->addWidget(closeBtn);

    layout->addLayout(bottomLayout);

    auto &tm = ThemeManager::instance();
    setStyleSheet(
        QString("QDialog { background: %1; color: %2; }"
                "QGroupBox { font-weight: bold; border: 1px solid %3; "
                "border-radius: 6px; margin-top: 10px; padding-top: 15px; "
                "color: %2; }"
                "QGroupBox::title { subcontrol-origin: margin; left: 10px; "
                "padding: 0 3px; color: %2; }"
                "QLabel { color: %2; }"
                "QCheckBox { color: %2; }")
            .arg(tm.palette().color(QPalette::Window).name(),
                 tm.palette().color(QPalette::WindowText).name(),
                 tm.customColor("sidebarBorder", QColor("#363a4f")).name()));

    connect(dontShowAgain, &QCheckBox::checkStateChanged, this, [](int state) {
      TomlSettingsStore::instance().setOnboardingDismissed(state ==
                                                           Qt::Checked);
    });
  }
};

static QString baseStyle() {
  auto &tm = ThemeManager::instance();
  QPalette pal = tm.palette();
  QString bg = pal.color(QPalette::Window).name();
  QString text = pal.color(QPalette::WindowText).name();
  QString base = pal.color(QPalette::Base).name();
  QString altBase = pal.color(QPalette::AlternateBase).name();
  QString highlight = pal.color(QPalette::Highlight).name();
  QString border = tm.customColor("sidebarBorder", QColor("#363a4f")).name();
  QString hover = tm.customColor("treeHover", QColor("#363a4f")).name();

  return QString(
             "QMainWindow, QWidget { background: %1; color: %2; }"
             "QMenuBar { background: %1; color: %2; border-bottom: 1px solid "
             "%3; padding: 2px; }"
             "QMenuBar::item:selected { background: %4; }"
             "QMenuBar::item:pressed { background: %5; }"
             "QMenu { background: %1; color: %2; border: 1px solid %3; }"
             "QMenu::item:selected { background: %4; }"
             "QMenu::separator { height: 1px; background: %3; margin: 4px 8px; "
             "}"
             "QStatusBar { background: %1; color: %2; border-top: 1px solid "
             "%3; font-size: 11px; }"
             "QStatusBar::item { border: none; }"
             "QDockWidget { background: %1; color: %2; }"
             "QDockWidget::title { background: %6; color: %2; padding: 4px; }")
      .arg(bg, text, border, hover, highlight, altBase);
}

QIcon createHeliosIcon() {
  return QIcon(QStringLiteral(":/icons/helios-icon.svg"));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_initialContextSetup(true) {
  setWindowTitle("Helios");
  setWindowIcon(createHeliosIcon());
  setMinimumHeight(600);
  resize(1100, 750);

  AppearanceController::instance().apply();

  // Initial load of external settings/themes/locales
  auto &settings = TomlSettingsStore::instance();
  m_appFontFamily = settings.uiFontFamily();
  m_appFontSize = settings.uiFontSize();
  m_wordWrapEnabled = settings.wordWrap();

  m_tabWidget = new QTabWidget;
  m_tabWidget->setTabsClosable(true);
  m_tabWidget->setMovable(true);
  m_tabWidget->setDocumentMode(true);

  m_breadcrumbs = new BreadcrumbsBar;
  m_findReplaceBar = new FindReplaceBar;
  m_findReplaceBar->hide();

  QWidget *editorPanel = new QWidget;
  QVBoxLayout *editorLayout = new QVBoxLayout(editorPanel);
  editorLayout->setContentsMargins(0, 0, 0, 0);
  editorLayout->setSpacing(0);
  editorLayout->addWidget(m_breadcrumbs);
  editorLayout->addWidget(m_findReplaceBar);
  editorLayout->addWidget(m_tabWidget, 1);

  m_centralStackedWidget = new QStackedWidget(this);
  m_welcomeWidget = new WelcomeWidget(this);
  m_centralStackedWidget->addWidget(m_welcomeWidget);
  m_centralStackedWidget->addWidget(editorPanel);

  m_fileTree = new FileTreePanel;
  m_searchPanel = new SearchPanel;
  m_gitPanel = new GitPanel;
  m_settingsPanel = new SettingsPanel;
  m_shortcutsDialog = new ShortcutsDialog(this);
  m_lspManagerDialog = new LspManagerDialog(this);

  m_sidePanel = new QStackedWidget;
  m_sidePanel->addWidget(m_fileTree);
  m_sidePanel->addWidget(m_searchPanel);
  m_sidePanel->addWidget(m_gitPanel);
  m_sidePanel->addWidget(m_settingsPanel);
  m_sidePanel->setMinimumWidth(150);
  m_sidePanel->setMaximumWidth(420);

  m_outlinePanel = new OutlinePanel(this);

  m_splitter = new QSplitter(Qt::Horizontal);
  m_splitter->addWidget(m_sidePanel);
  m_splitter->addWidget(m_centralStackedWidget);
  m_splitter->addWidget(m_outlinePanel);
  m_splitter->setStretchFactor(0, 0);
  m_splitter->setStretchFactor(1, 1);
  m_splitter->setStretchFactor(2, 0);
  m_splitter->setSizes({settings.sidebarWidth(), 700, 220});
  m_splitter->setChildrenCollapsible(false);

  setCentralWidget(m_splitter);

  m_snippetManager = new SnippetManager(this);
  m_snippetManager->loadFromJson(":/snippets/zith-snippets.json");

  m_completionModel = new LspCompletionModel(this);
  m_completer = new LspCompleter(m_completionModel, this);

  m_lspClient = new LspClient(this);
  m_zithToolchainManager = new ZithToolchainManager(this);

  applyAppearanceChange();

  connect(
      m_lspClient, &LspClient::completionResults, this,
      [this](const QString &uri, int, const QList<LspCompletionItem> &items) {
        if (!lspEnabled())
          return;

        CodeEditor *activeEd = currentEditor();
        if (!activeEd || activeEd->fileUri() != uri)
          return;

        auto all = items;
        all.append(m_snippetManager->allSnippets());
        m_completionModel->setItems(all);
        if (!all.isEmpty()) {
          m_completer->setWidget(activeEd);
          m_completer->complete();
        }
      });

  connect(m_lspClient, &LspClient::initialized, this, [this]() {
    if (!lspEnabled()) {
      m_lspClient->stop();
      return;
    }

    setLspStatus("LSP ⬤", "#a6d189");
    m_runtimeStatusText =
        m_runtimeTag.isEmpty()
            ? "LSP connected."
            : QString("Connected to runtime %1.").arg(m_runtimeTag);
    updateSettingsRuntimeInfo();
    statusBar()->showMessage("LSP connected", 3000);
    for (int i = 0; i < m_tabWidget->count(); ++i) {
      auto *ed = qobject_cast<CodeEditor *>(m_tabWidget->widget(i));
      if (!ed || ed->filePath().isEmpty())
        continue;
      m_lspClient->openDocument(ed->fileUri(), "zith", ed->toPlainText(),
                                ed->documentVersion());
    }
    m_lastLspError.clear();
    updateLspDiagnostics();
  });

  connect(m_lspClient, &LspClient::serverStopped, this, [this]() {
    if (!lspEnabled()) {
      m_runtimeStatusText = "Disabled";
      setLspStatus("LSP Disabled", "#6c7086");
      updateSettingsRuntimeInfo();
      updateLspDiagnostics();
      return;
    }

    setLspStatus("LSP ○", "#f9e2af");
    m_runtimeStatusText = "LSP server stopped.";
    updateSettingsRuntimeInfo();
    updateLspDiagnostics();
  });

  connect(m_lspClient, &LspClient::processStopped, this, [this](bool expected) {
    if (expected || !lspEnabled() || m_activeLspPath.isEmpty())
      return;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    while (!m_lspRestartTimes.isEmpty() &&
           m_lspRestartTimes.first() <= now - 60000)
      m_lspRestartTimes.removeFirst();
    if (m_lspRestartTimes.size() >= 3) {
      m_runtimeStatusText =
          "LSP crashed repeatedly; automatic restart stopped.";
      setLspStatus("LSP !", "#e78284");
      updateSettingsRuntimeInfo();
      return;
    }
    m_lspRestartTimes.append(now);
    const int delaySeconds = 1 << (m_lspRestartTimes.size() - 1);
    m_runtimeStatusText =
        QString("LSP stopped; restarting in %1 second(s)...").arg(delaySeconds);
    setLspStatus("LSP ○", "#f9e2af");
    updateSettingsRuntimeInfo();
    QTimer::singleShot(delaySeconds * 1000, this, [this]() {
      if (lspEnabled() && m_lspClient && !m_lspClient->isRunning())
        m_lspClient->start(m_activeLspPath, m_activeStdlibPath,
                           m_activeWorkspaceRoot);
    });
  });

  connect(m_lspClient, &LspClient::logMessage, this,
          [this](const QString &msg) {
            if (lspEnabled() && m_settingsPanel)
              m_settingsPanel->appendLspLog(msg);
            if (lspEnabled() && m_lspManagerDialog)
              m_lspManagerDialog->appendLspLog(msg);
          });

  connect(m_lspClient, &LspClient::frontendStatusReceived, this, &MainWindow::onFrontendStatusReceived);
  connect(m_lspClient, &LspClient::metricsReceived, this, &MainWindow::onMetricsReceived);

  connect(m_lspClient, &LspClient::serverError, this,
          [this](const QString &msg) {
            if (!lspEnabled())
              return;

            setLspStatus("LSP !", "#e78284");
            m_runtimeStatusText = "LSP error: " + msg;
            m_lastLspError = msg;
            updateSettingsRuntimeInfo();
            updateLspDiagnostics();
            if (m_settingsPanel)
              m_settingsPanel->appendLspLog("[error] " + msg);
            if (m_lspManagerDialog)
              m_lspManagerDialog->appendLspLog("[error] " + msg);
            statusBar()->showMessage("LSP: " + msg, 5000);
          });

  // Formatting integrations
  connect(m_lspClient, &LspClient::formattingResult, this,
          [this](const QString &uri, int version,
                 const QList<QPair<LspRange, QString>> &edits) {
            if (auto *ed = currentEditor(); ed && ed->fileUri() == uri &&
                                            ed->documentVersion() == version) {
              ed->applyEdits(edits);
            }
          });

  // Outline / Symbols integrations
  connect(m_lspClient, &LspClient::documentSymbolsResult, this,
          [this](const QString &uri, int version, const QJsonArray &symbols) {
            if (auto *ed = currentEditor();
                ed && ed->fileUri() == uri && ed->documentVersion() == version)
              m_outlinePanel->setSymbols(symbols);
          });
  connect(m_outlinePanel, &OutlinePanel::symbolSelected, this,
          [this](int line, int col) {
            if (auto *ed = currentEditor()) {
              ed->goToLine(line, col);
            }
          });

  m_contextLabel = new QLabel("◀ 1/1 ▶");
  m_contextLabel->setStyleSheet("color: #7287fd; padding: 0 4px;");

  m_lspLabel = new QLabel("LSP ⬤");
  m_lspLabel->setStyleSheet("color: #a6d189; padding: 0 4px;");

  m_errorLabel = new QLabel("✕0  ⚠0");
  m_errorLabel->setStyleSheet("color: #a5adce; padding: 0 4px;");

  m_posLabel = new QLabel("Ln 1, Col 1");
  m_posLabel->setStyleSheet("color: #a5adce; padding: 0 4px;");

  m_indentLabel = new QLabel("Spaces: 4");
  m_indentLabel->setStyleSheet("color: #6c7086; padding: 0 4px;");

  m_encodingLabel = new QLabel("UTF-8");
  m_encodingLabel->setStyleSheet("color: #6c7086; padding: 0 4px;");

  m_langLabel = new QLabel("Zith");
  m_langLabel->setStyleSheet(
      "color: #8839ef; padding: 0 4px; font-weight: bold;");

  statusBar()->addWidget(m_contextLabel);
  statusBar()->addWidget(m_lspLabel);
  m_vimLabel = new QLabel(settings.vimMotionsEnabled() ? "VIM: NORMAL" : "VIM: OFF");
  m_vimLabel->setStyleSheet("padding: 0 4px;");
  statusBar()->addPermanentWidget(m_vimLabel);

  statusBar()->addPermanentWidget(m_errorLabel);
  statusBar()->addPermanentWidget(m_posLabel);
  statusBar()->addPermanentWidget(m_indentLabel);
  statusBar()->addPermanentWidget(m_encodingLabel);
  statusBar()->addPermanentWidget(m_langLabel);

  connect(m_zithToolchainManager, &ZithToolchainManager::statusChanged, this,
          [this](const QString &message) {
            if (!lspEnabled())
              return;

            m_runtimeStatusText = message;
            updateSettingsRuntimeInfo();
            statusBar()->showMessage(message, 5000);
          });
  connect(m_zithToolchainManager, &ZithToolchainManager::failed, this,
          [this](const QString &message) {
            if (!lspEnabled())
              return;

            setLspStatus("LSP !", "#e78284");
            m_runtimeStatusText = message;
            updateSettingsRuntimeInfo();
            statusBar()->showMessage(message, 8000);
          });
  connect(m_zithToolchainManager, &ZithToolchainManager::ready, this,
          &MainWindow::startLspRuntime);

  m_contextManager = new ContextManager(this);

  // Tree signal integrations
  connect(m_fileTree, &FileTreePanel::fileActivated, this,
          [this](const QString &path) { openFilePath(path); });
  connect(m_fileTree, &FileTreePanel::fileActivatedInNewTab, this,
          [this](const QString &path) {
            createTab();
            openFilePath(path);
          });
  connect(m_fileTree, &FileTreePanel::projectRootChanged, this,
          [this](const QString &path) { setWorkspaceRoot(path); });

  // Welcome screen connections
  connect(m_welcomeWidget, &WelcomeWidget::openFolderRequested, this,
          &MainWindow::openFolder);
  connect(m_welcomeWidget, &WelcomeWidget::newProjectRequested, this,
          &MainWindow::newProject);
  connect(m_welcomeWidget, &WelcomeWidget::projectSelected, this,
          [this](const QString &path) { setWorkspaceRoot(path); });

  connect(m_searchPanel, &SearchPanel::fileActivated, this,
          [this](const QString &path, int line, int column) {
            openFilePath(path);
            if (auto *ed = currentEditor())
              ed->goToLine(line, column);
          });

  connect(m_gitPanel, &GitPanel::fileActivated, this,
          [this](const QString &path) { openFilePath(path); });

  // The side settings panel contains runtime/LSP status and shortcut help.
  connect(m_settingsPanel, &SettingsPanel::openPreferencesRequested, this,
          &MainWindow::showPreferences);
  connect(m_settingsPanel, &SettingsPanel::openShortcutsRequested, this,
          &MainWindow::showShortcutsDialog);
  connect(m_settingsPanel, &SettingsPanel::openLspManagerRequested, this,
          &MainWindow::showLspManagerDialog);
  connect(m_settingsPanel, &SettingsPanel::lspEnabledChanged, this,
          &MainWindow::setLspEnabled);
  connect(m_settingsPanel, &SettingsPanel::refreshRuntimeRequested, this,
          [this]() {
            if (lspEnabled())
              ensureLspRuntime(false);
          });
  connect(m_settingsPanel, &SettingsPanel::clearRuntimeCacheRequested, this,
          &MainWindow::clearRuntimeCache);
  connect(m_lspManagerDialog, &LspManagerDialog::lspEnabledChanged, this,
          &MainWindow::setLspEnabled);
  connect(m_lspManagerDialog, &LspManagerDialog::refreshRuntimeRequested, this,
          [this]() {
            if (lspEnabled())
              ensureLspRuntime(false);
          });
  connect(m_lspManagerDialog, &LspManagerDialog::clearRuntimeCacheRequested,
          this, &MainWindow::clearRuntimeCache);

  updateSettingsRuntimeInfo();

  // Context Navigation
  auto *ctxLeft = new QShortcut(QKeySequence("Alt+Left"), this);
  connect(ctxLeft, &QShortcut::activated, this, [this]() {
    saveContextState();
    m_contextManager->navigateLeft();
  });

  auto *ctxRight = new QShortcut(QKeySequence("Alt+Right"), this);
  connect(ctxRight, &QShortcut::activated, this, [this]() {
    int last = m_contextManager->count() - 1;
    int cur = m_contextManager->currentIndex();
    if (cur == last && !m_contextManager->currentRoot().isEmpty()) {
      QString dir =
          QFileDialog::getExistingDirectory(this, "Open project folder");
      if (!dir.isEmpty()) {
        saveContextState();
        m_contextManager->appendNew(dir);
      }
    } else if (cur < last) {
      saveContextState();
      m_contextManager->navigateRight();
    }
  });

  connect(m_contextManager, &ContextManager::contextChanged, this,
          [this](int index, const Context &ctx) {
            m_fileTree->setRootPath(ctx.rootPath);
            m_searchPanel->setRootPath(ctx.rootPath);
            m_gitPanel->setRootPath(ctx.rootPath);
            if (m_contextLabel)
              m_contextLabel->setText(QString("%1/%2").arg(index + 1).arg(
                  m_contextManager->count()));
            if (!m_initialContextSetup && !m_replacingWorkspaceRoot) {
              restoreContextState(ctx);
              if (lspEnabled() && m_lspClient && m_lspClient->isRunning()) {
                ensureLspRuntime(true);
              }
            }
          });

  // Shortcuts and Actions
  auto *openFolderShortcut = new QShortcut(QKeySequence("Ctrl+Alt+O"), this);
  connect(openFolderShortcut, &QShortcut::activated, this,
          [this]() { openFolder(); });

  auto *findShortcut = new QShortcut(QKeySequence::Find, this);
  connect(findShortcut, &QShortcut::activated, this, [this]() {
    m_findReplaceBar->setEditor(currentEditor());
    m_findReplaceBar->showFind();
  });

  auto *replaceShortcut = new QShortcut(QKeySequence("Ctrl+H"), this);
  connect(replaceShortcut, &QShortcut::activated, this, [this]() {
    m_findReplaceBar->setEditor(currentEditor());
    m_findReplaceBar->showReplace();
  });

  auto *findNextShortcut = new QShortcut(QKeySequence::FindNext, this);
  connect(findNextShortcut, &QShortcut::activated, this, [this]() {
    if (m_findReplaceBar->isVisible()) {
      m_findReplaceBar->findNext();
    } else {
      m_findReplaceBar->setEditor(currentEditor());
      m_findReplaceBar->showFind();
    }
  });

  auto *findPrevShortcut = new QShortcut(QKeySequence::FindPrevious, this);
  connect(findPrevShortcut, &QShortcut::activated, this, [this]() {
    if (m_findReplaceBar->isVisible()) {
      m_findReplaceBar->findPrevious();
    } else {
      m_findReplaceBar->setEditor(currentEditor());
      m_findReplaceBar->showFind();
    }
  });

  auto *explorerShortcut = new QShortcut(QKeySequence("Ctrl+Shift+E"), this);
  connect(explorerShortcut, &QShortcut::activated, this, [this]() {
    setSidebarMode(ActivityBar::Explorer);
    if (m_sidePanel->isVisible()) m_fileTree->setFocus();
  });

  auto *workspaceSearchShortcut =
      new QShortcut(QKeySequence("Ctrl+Shift+F"), this);
  connect(workspaceSearchShortcut, &QShortcut::activated, this, [this]() {
    setSidebarMode(ActivityBar::Search);
  });

  auto *gitShortcut = new QShortcut(QKeySequence("Ctrl+Shift+G"), this);
  connect(gitShortcut, &QShortcut::activated, this, [this]() {
    setSidebarMode(ActivityBar::Git);
  });

  auto *settingsShortcut = new QShortcut(QKeySequence("Ctrl+,"), this);
  connect(settingsShortcut, &QShortcut::activated, this,
          &MainWindow::showPreferences);

  auto *hideSidebarShortcut = new QShortcut(QKeySequence("Ctrl+Shift+X"), this);
  connect(hideSidebarShortcut, &QShortcut::activated, this, [this]() {
    toggleFileTree(false);
  });

  m_initialContextSetup = false;
  saveContextState();

  connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int idx) {
    auto *ed = qobject_cast<CodeEditor *>(m_tabWidget->widget(idx));
    updateEditorChrome(ed);
    updateCentralWidgetState();
  });

  connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, [this](int idx) {
    auto *ed = qobject_cast<CodeEditor *>(m_tabWidget->widget(idx));
    if (!ed)
      return;
    if (ed->document()->isModified()) {
      auto result = QMessageBox::question(
          this, "Save?",
          QString("Save changes to '%1'?")
              .arg(ed->filePath().isEmpty() ? "Untitled" : ed->filePath()),
          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
      if (result == QMessageBox::Save) {
        m_tabWidget->setCurrentIndex(idx);
        saveFile();
        if (!ed->document()->isModified())
          releaseEditor(ed);
        return;
      } else if (result == QMessageBox::Cancel) {
        return;
      }
    }
    releaseEditor(ed);
    updateCentralWidgetState();
  });

  m_diagnosticsPanel = new DiagnosticsPanel(this);
  addDockWidget(Qt::BottomDockWidgetArea, m_diagnosticsPanel);

  m_referencesPanel = new ReferencesPanel(this);
  addDockWidget(Qt::BottomDockWidgetArea, m_referencesPanel);
  m_referencesPanel->hide();

  connect(m_lspClient, &LspClient::diagnosticsReceived, m_diagnosticsPanel,
          &DiagnosticsPanel::setDiagnostics);
  connect(m_diagnosticsPanel, &DiagnosticsPanel::navigateToLocation, this,
          [this](const QString &uri, int line, int col) {
            openFilePath(QUrl(uri).toLocalFile());
            if (auto *ed = currentEditor())
              ed->goToLine(line, col);
          });
  connect(m_diagnosticsPanel, &DiagnosticsPanel::countsChanged, this,
          [this](int errs, int warns) {
            if (m_errorLabel)
              m_errorLabel->setText(QString("✕%1  ⚠%2").arg(errs).arg(warns));
          });
  connect(m_referencesPanel, &ReferencesPanel::navigateToLocation, this,
          [this](const QString &uri, int line, int character) {
            openFilePath(QUrl(uri).toLocalFile());
            if (auto *editor = currentEditor())
              editor->goToLine(line, character);
          });
  connect(m_lspClient, &LspClient::referencesResult, this,
          [this](const QString &, int, const QList<LspLocation> &locations) {
            if (!m_referencesPanel)
              return;
            m_referencesPanel->setReferences(locations);
            if (!locations.isEmpty()) {
              m_referencesPanel->show();
              m_referencesPanel->raise();
            }
          });
  connect(m_lspClient, &LspClient::renameResult, this,
          [this](const QString &, int, const QJsonObject &edit) {
            applyWorkspaceEdit(edit);
          });
  connect(m_lspClient, &LspClient::codeActionsResult, this,
          [this](const QString &, int, const QJsonArray &actions) {
            if (actions.isEmpty())
              return;
            QMenu menu(this);
            for (const QJsonValue &value : actions) {
              const QJsonObject action = value.toObject();
              QAction *entry =
                  menu.addAction(action.value("title").toString("Code action"));
              if (action.contains("edit"))
                connect(entry, &QAction::triggered, this, [this, action] {
                  applyWorkspaceEdit(action.value("edit").toObject());
                });
              else {
                entry->setEnabled(false);
                entry->setToolTip(
                    "This action requires workspace/executeCommand, which "
                    "Helios does not invoke.");
              }
            }
            menu.exec(QCursor::pos());
          });
  connect(m_lspClient, &LspClient::saveAllRequested, this,
          &MainWindow::saveAllForLsp);
  connect(m_lspClient, &LspClient::showMessage, this,
          [this](const QString &message) {
            statusBar()->showMessage(message, 5000);
          });

  m_compilerPanel = new CompilerPanel(this);
  addDockWidget(Qt::BottomDockWidgetArea, m_compilerPanel);
  m_compilerPanel->hide();

  auto runZith = [this](const QString &cmd, bool singleFile) {
    auto *ed = currentEditor();
    if (singleFile && (!ed || ed->filePath().isEmpty())) {
      statusBar()->showMessage("No open file to compile/check", 4000);
      return;
    }
    QString dir =
        m_contextManager ? m_contextManager->currentRoot() : QString();
    if (dir.isEmpty() && ed) {
      dir = QFileInfo(ed->filePath()).absolutePath();
    }
    if (dir.isEmpty()) {
      statusBar()->showMessage("No active directory for compilation", 4000);
      return;
    }

    QString compiler = m_activeLspPath;
    if (compiler.isEmpty() || !compiler.contains("zith")) {
      compiler = "zith";
    }

    QStringList args = {cmd};
    if (singleFile && ed) {
      args.append(ed->filePath());
    }

    m_compilerPanel->show();
    m_compilerPanel->raise();
    m_compilerPanel->runCommand(dir, compiler, args);
  };

  m_activityBar = new ActivityBar(this);
  addDockWidget(Qt::LeftDockWidgetArea, m_activityBar);

  connect(m_activityBar, &ActivityBar::modeChanged, this,
          [this](ActivityBar::Mode mode) { setSidebarMode(mode); });
  connect(m_activityBar, &ActivityBar::preferencesRequested, this,
          &MainWindow::showPreferences);

  // Menus
  m_fileMenu = menuBar()->addMenu("&File");
  m_newAct = m_fileMenu->addAction("&New File", QKeySequence::New);
  connect(m_newAct, &QAction::triggered, this, [this]() { newFile(); });

  m_newWinAct =
      m_fileMenu->addAction("New &Window", QKeySequence("Ctrl+Shift+N"));
  connect(m_newWinAct, &QAction::triggered, this, [this]() {
    auto *win = new MainWindow;
    win->show();
  });

  m_newProjAct =
      m_fileMenu->addAction("New &Project...", QKeySequence("Ctrl+Alt+N"));
  connect(m_newProjAct, &QAction::triggered, this, [this]() { newProject(); });

  m_openAct = m_fileMenu->addAction("&Open...", QKeySequence::Open);
  connect(m_openAct, &QAction::triggered, this, [this]() { openFile(); });

  m_saveAct = m_fileMenu->addAction("&Save", QKeySequence::Save);
  connect(m_saveAct, &QAction::triggered, this, [this]() { saveFile(); });

  m_exitAct = m_fileMenu->addAction("E&xit", QKeySequence::Quit);
  connect(m_exitAct, &QAction::triggered, this, [this]() { close(); });

  m_toolsMenu = menuBar()->addMenu("&Tools");
  m_buildAct = m_toolsMenu->addAction("&Build", QKeySequence("Ctrl+B"));
  connect(m_buildAct, &QAction::triggered, this,
          [runZith]() { runZith("build", false); });

  m_checkAct =
      m_toolsMenu->addAction("&Check File", QKeySequence("Ctrl+Shift+C"));
  connect(m_checkAct, &QAction::triggered, this,
          [runZith]() { runZith("check", true); });

  m_compileAct =
      m_toolsMenu->addAction("&Compile File", QKeySequence("Ctrl+Shift+B"));
  connect(m_compileAct, &QAction::triggered, this,
          [runZith]() { runZith("compile", true); });

  m_runAct = m_toolsMenu->addAction("&Run", QKeySequence("Ctrl+Shift+R"));
  connect(m_runAct, &QAction::triggered, this,
          [runZith]() { runZith("run", true); });

  m_restartLspAct =
      m_toolsMenu->addAction("Restart &LSP", QKeySequence("Ctrl+Shift+L"));
  connect(m_restartLspAct, &QAction::triggered, this, [this]() {
    if (lspEnabled())
      ensureLspRuntime(false);
    else {
      m_runtimeStatusText = "Disabled";
      updateSettingsRuntimeInfo();
    }
  });

  m_viewMenu = menuBar()->addMenu("&View");
  m_preferencesAct = m_viewMenu->addAction("&Preferences...", QKeySequence("Ctrl+,"));
  connect(m_preferencesAct, &QAction::triggered, this, &MainWindow::showPreferences);
  m_vimHelpAct = m_viewMenu->addAction("Vim Motions...");
  connect(m_vimHelpAct, &QAction::triggered, this,
          &MainWindow::showVimHelpDialog);
  m_shortcutsAct = m_viewMenu->addAction("Shortcuts...");
  connect(m_shortcutsAct, &QAction::triggered, this,
          &MainWindow::showShortcutsDialog);
  m_lspManagerAct = m_viewMenu->addAction("LSP Manager...");
  connect(m_lspManagerAct, &QAction::triggered, this,
          &MainWindow::showLspManagerDialog);
  m_viewMenu->addAction(m_diagnosticsPanel->toggleViewAction());
  m_viewMenu->addAction(m_referencesPanel->toggleViewAction());
  m_viewMenu->addAction(m_compilerPanel->toggleViewAction());

  m_helpMenu = menuBar()->addMenu("&Help");
  m_gettingStartedAct =
      m_helpMenu->addAction("&Getting Started", this, [this]() {
        GettingStartedDialog dlg(this);
        dlg.exec();
      });
  m_gettingStartedAct->setShortcut(QKeySequence("F1"));

  // Apply active theme/language and load last recent workspace on startup
  applyThemeAndLanguage();
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
          [this]() { applyTheme(); });
  connect(&AppearanceController::instance(), &AppearanceController::appearanceChanged,
          this, &MainWindow::applyAppearanceChange);
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

  if (!lastProj.isEmpty()) {
    m_contextManager->setCurrentRoot(lastProj);
    m_fileTree->setRootPath(lastProj);
    m_welcomeWidget->refreshRecentProjects();
  } else {
    m_contextManager->setCurrentRoot("");
    m_fileTree->setRootPath("");
  }

  // Hide/show sidebar from saved state
  m_sidePanel->setVisible(settings.sidebarVisible());

  if (lspEnabled()) {
    m_runtimeStatusText = "Resolving latest Zith runtime...";
    setLspStatus("LSP ○", "#f9e2af");
    updateSettingsRuntimeInfo();
    ensureLspRuntime(true);
  } else {
    m_runtimeStatusText = "Disabled";
    setLspStatus("LSP Disabled", "#6c7086");
    updateSettingsRuntimeInfo();
    updateLspDiagnostics();
  }

  // Initial central stack update
  updateCentralWidgetState();

  // Trigger onboarding dialog if not dismissed on startup
  if (!settings.onboardingDismissed()) {
    QTimer::singleShot(500, this, [this]() {
      GettingStartedDialog dlg(this);
      dlg.exec();
    });
  }
}

void MainWindow::updateCentralWidgetState() {
  if (m_tabWidget->count() == 0) {
    m_centralStackedWidget->setCurrentWidget(m_welcomeWidget);
    m_breadcrumbs->hide();
    m_findReplaceBar->hide();
  } else {
    m_centralStackedWidget->setCurrentWidget(m_tabWidget->parentWidget());
    m_breadcrumbs->show();
  }
}

void MainWindow::openFilePath(const QString &path) {
  if (path.isEmpty())
    return;

  for (int i = 0; i < m_tabWidget->count(); ++i) {
    auto *ed = qobject_cast<CodeEditor *>(m_tabWidget->widget(i));
    if (ed && ed->filePath() == path) {
      m_tabWidget->setCurrentIndex(i);
      updateEditorChrome(ed);
      updateCentralWidgetState();
      return;
    }
  }

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  QString content = QString::fromUtf8(file.readAll());
  file.close();

  auto *editor = createTab(false);
  editor->setFilePath(path);
  editor->setInitialDocumentText(content);
  editor->document()->setModified(false);
  editor->clearDiagnostics();

  const int idx = m_tabWidget->indexOf(editor);
  m_tabWidget->setTabText(idx, QFileInfo(path).fileName());
  m_tabWidget->setTabToolTip(idx, path);

  if (m_diagnosticsPanel)
    m_diagnosticsPanel->clearDiagnostics(editor->fileUri());
  if (m_completionModel)
    m_completionModel->setItems({});

  m_tabWidget->setCurrentIndex(idx);
  updateEditorChrome(editor);
  updateCentralWidgetState();

  QTimer::singleShot(0, editor, [this, editor, content]() {
    if (lspEnabled() && m_lspClient && m_lspClient->isReady())
      m_lspClient->openDocument(editor->fileUri(), "zith", content,
                                editor->documentVersion());
  });
}

CodeEditor *MainWindow::createTab(bool makeCurrent) {
  auto *editor = new CodeEditor;
  editor->setFont(AppearanceController::instance().editorFont());

  auto *hl = new SyntaxHighlighter(editor->document());
  m_highlighters.insert(editor, hl);

  editor->setSnippetManager(m_snippetManager);
  editor->setCompleter(m_completer);
  editor->setLspClient(m_lspClient);
  applyEditorPreferences(editor);

  connectEditorSignals(editor);

  int idx = -1;
  if (makeCurrent) {
    idx = m_tabWidget->addTab(editor, "Untitled");
    m_tabWidget->setCurrentIndex(idx);
  } else {
    QSignalBlocker blocker(m_tabWidget);
    idx = m_tabWidget->addTab(editor, "Untitled");
  }
  m_tabWidget->setTabToolTip(idx, {});

  updateCentralWidgetState();
  return editor;
}

void MainWindow::connectEditorSignals(CodeEditor *editor) {
  connect(editor, &CodeEditor::zoomChanged, this,
          [this](double scale) { setAppFontSize(qRound(12 * scale)); });

  connect(editor, &CodeEditor::cursorPositionChanged, this, [this]() {
    if (auto *ed = currentEditor())
      updateEditorChrome(ed);
  });
  connect(editor, &CodeEditor::vimModeChanged, this, [this, editor](const QString &mode) {
    if (editor == currentEditor() && m_vimLabel)
      m_vimLabel->setText("VIM: " + mode);
  });

  connect(editor, &CodeEditor::navigateToLocation, this,
          [this](const QString &uri, int line, int col) {
            openFilePath(QUrl(uri).toLocalFile());
            if (auto *ed = currentEditor())
              ed->goToLine(line, col);
          });
  connect(editor, &CodeEditor::renameRequested, this,
          [this](const QString &uri, int version, const LspPosition &position,
                 const QString &name) {
            for (int i = 0; i < m_tabWidget->count(); ++i)
              if (auto *open =
                      qobject_cast<CodeEditor *>(m_tabWidget->widget(i)))
                open->flushPendingLspChanges();
            if (m_lspClient && m_lspClient->isReady())
              m_lspClient->requestRename(uri, version, position, name);
          });
  connect(
      editor, &CodeEditor::codeActionsRequested, this,
      [this, editor](const QString &uri, int version, const LspRange &range) {
        if (m_lspClient && m_lspClient->isReady())
          m_lspClient->requestCodeActions(uri, version, range,
                                          editor->diagnostics());
      });
}

void MainWindow::newFile() { createTab(true); }

void MainWindow::newProject() {
  QString dir = QFileDialog::getExistingDirectory(this, "New project folder");
  if (dir.isEmpty())
    return;

  saveContextState();
  m_contextManager->appendNew(dir);
  TomlSettingsStore::instance().addRecentProject(dir);
  m_welcomeWidget->refreshRecentProjects();
}

void MainWindow::openFolder() {
  const QString initialPath =
      m_contextManager && !m_contextManager->currentRoot().isEmpty()
          ? m_contextManager->currentRoot()
          : QDir::homePath();

  const QString path = QFileDialog::getExistingDirectory(
      this, "Open Folder", initialPath, QFileDialog::ShowDirsOnly);
  if (!path.isEmpty())
    setWorkspaceRoot(path);
}

void MainWindow::setWorkspaceRoot(const QString &path) {
  QFileInfo fi(path);
  if (!fi.exists() || !fi.isDir())
    return;
  QString normalized = fi.absoluteFilePath();

  saveContextState();

  m_replacingWorkspaceRoot = true;
  m_contextManager->setCurrentRoot(normalized);
  m_replacingWorkspaceRoot = false;

  m_fileTree->setRootPath(normalized);
  TomlSettingsStore::instance().addRecentProject(normalized);
  m_welcomeWidget->refreshRecentProjects();
}

void MainWindow::openFile() {
  QString path = QFileDialog::getOpenFileName(
      this, "Open file",
      m_contextManager ? m_contextManager->currentRoot() : QString(),
      "Zith Files (*.zith);;All Files (*)");
  if (!path.isEmpty())
    openFilePath(path);
}

void MainWindow::saveFile() {
  auto *ed = currentEditor();
  if (!ed)
    return;

  if (ed->filePath().isEmpty()) {
    QString path = QFileDialog::getSaveFileName(
        this, "Save File As",
        m_contextManager ? m_contextManager->currentRoot() : QString(),
        "Zith Files (*.zith);;All Files (*)");
    if (path.isEmpty())
      return;
    ed->setFilePath(path);
    const int idx = m_tabWidget->currentIndex();
    m_tabWidget->setTabText(idx, QFileInfo(path).fileName());
    m_tabWidget->setTabToolTip(idx, path);
  }

  QFile file(ed->filePath());
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);
    out << ed->toPlainText();
    file.close();
    ed->document()->setModified(false);
    statusBar()->showMessage("File saved", 2000);
    if (lspEnabled() && m_lspClient && m_lspClient->isReady()) {
      m_lspClient->saveDocument(ed->fileUri());
    }
  }
}

void MainWindow::toggleFileTree(bool show) {
  m_sidePanel->setVisible(show);
  TomlSettingsStore::instance().setSidebarVisible(show);
  m_activityBar->setActiveMode(ActivityBar::Mode(m_sidePanel->currentIndex()), show);
}

void MainWindow::saveContextState() {
  int idx = m_contextManager->currentIndex();
  QStringList files;
  for (int i = 0; i < m_tabWidget->count(); ++i) {
    auto *ed = qobject_cast<CodeEditor *>(m_tabWidget->widget(i));
    if (ed && !ed->filePath().isEmpty())
      files << ed->filePath();
  }
  m_contextManager->setContextState(idx, files, m_tabWidget->currentIndex());
}

void MainWindow::restoreContextState(const Context &ctx) {
  while (m_tabWidget->count() > 0) {
    auto *editor = qobject_cast<CodeEditor *>(m_tabWidget->widget(0));
    if (editor) {
      if (lspEnabled() && m_lspClient && m_lspClient->isReady() &&
          !editor->fileUri().isEmpty())
        m_lspClient->closeDocument(editor->fileUri());
      releaseEditor(editor);
    }
  }

  for (const QString &file : ctx.openFiles) {
    openFilePath(file);
  }

  if (ctx.currentTab >= 0 && ctx.currentTab < m_tabWidget->count()) {
    m_tabWidget->setCurrentIndex(ctx.currentTab);
  }

  updateCentralWidgetState();
}

CodeEditor *MainWindow::currentEditor() const {
  if (m_tabWidget->count() == 0)
    return nullptr;
  return qobject_cast<CodeEditor *>(m_tabWidget->currentWidget());
}

void MainWindow::setSidebarMode(ActivityBar::Mode mode) {
  bool visible = m_sidePanel->isVisible();

  if (visible &&
      m_sidePanel->currentIndex() == int(mode)) {
    toggleFileTree(false);
    return;
  }

  m_sidePanel->setCurrentIndex(int(mode));

  if (mode == ActivityBar::Git)
    m_gitPanel->refreshStatus();

  toggleFileTree(true);
}

void MainWindow::showSettingsPanel()
{
  m_sidePanel->setCurrentWidget(m_settingsPanel);
  if (!m_sidePanel->isVisible())
    toggleFileTree(true);
  TomlSettingsStore::instance().setSidebarVisible(true);
}

void MainWindow::showShortcutsDialog()
{
  if (!m_shortcutsDialog) return;
  if (m_shortcutsDialog->isVisible()) {
      m_shortcutsDialog->close();
      return;
  }
  m_shortcutsDialog->show();
  m_shortcutsDialog->raise();
  m_shortcutsDialog->activateWindow();
}

void MainWindow::showLspManagerDialog()
{
  if (!m_lspManagerDialog) return;
  if (m_lspManagerDialog->isVisible()) {
      m_lspManagerDialog->close();
      return;
  }
  m_lspManagerDialog->show();
  m_lspManagerDialog->raise();
  m_lspManagerDialog->activateWindow();
}

void MainWindow::applyEditorPreferences(CodeEditor *editor) {
  if (!editor)
    return;

  editor->setFont(AppearanceController::instance().editorFont());
  editor->setLineWrapMode(m_wordWrapEnabled ? QPlainTextEdit::WidgetWidth
                                            : QPlainTextEdit::NoWrap);
  editor->update();
  editor->setVimMotionsEnabled(TomlSettingsStore::instance().vimMotionsEnabled());
}

void MainWindow::setAppFontFamily(const QString &family) {
  m_appFontFamily = family;

  AppearanceController::instance().setUiFontFamily(family);

  for (int i = 0; i < m_tabWidget->count(); ++i) {
    if (auto *ed = qobject_cast<CodeEditor *>(m_tabWidget->widget(i))) {
      applyEditorPreferences(ed);
    }
  }
  saveUiPreferences();
}

void MainWindow::setAppFontSize(int pointSize) {
  m_appFontSize = pointSize;

  AppearanceController::instance().setUiFontSize(pointSize);

  for (int i = 0; i < m_tabWidget->count(); ++i) {
    if (auto *ed = qobject_cast<CodeEditor *>(m_tabWidget->widget(i))) {
      applyEditorPreferences(ed);
    }
  }
  saveUiPreferences();
}

void MainWindow::setWordWrapEnabled(bool enabled) {
  m_wordWrapEnabled = enabled;
  for (int i = 0; i < m_tabWidget->count(); ++i)
    applyEditorPreferences(qobject_cast<CodeEditor *>(m_tabWidget->widget(i)));
  saveUiPreferences();
}

void MainWindow::loadUiPreferences() {
  auto &settings = TomlSettingsStore::instance();
  m_appFontFamily = settings.fontFamily();
  m_appFontSize = settings.fontSize();
  m_wordWrapEnabled = settings.wordWrap();
}

void MainWindow::saveUiPreferences() const {
  auto &settings = TomlSettingsStore::instance();
  settings.setFontFamily(m_appFontFamily);
  settings.setFontSize(m_appFontSize);
  settings.setWordWrap(m_wordWrapEnabled);
}

void MainWindow::updateEditorChrome(CodeEditor *editor) {
  if (!editor) {
    m_findReplaceBar->setEditor(nullptr);
    m_breadcrumbs->clear();
    m_posLabel->setText("Ln 1, Col 1");
    m_outlinePanel->clear();
    setWindowTitle("Helios");
    return;
  }

  m_findReplaceBar->setEditor(editor);
  if (editor->filePath().isEmpty())
    m_breadcrumbs->clear();
  else
    m_breadcrumbs->setPath(editor->filePath());

  QTextCursor cursor = editor->textCursor();
  m_posLabel->setText(QString("Ln %1, Col %2")
                          .arg(cursor.blockNumber() + 1)
                          .arg(cursor.columnNumber() + 1));

  if (editor->filePath().isEmpty()) {
    setWindowTitle("Helios");
  } else {
    setWindowTitle(
        QString("%1 — Helios").arg(QFileInfo(editor->filePath()).fileName()));
  }

  // Request symbols for outline view
  if (lspEnabled() && m_lspClient && m_lspClient->isReady()) {
    m_lspClient->requestDocumentSymbols(editor->fileUri(),
                                        editor->documentVersion());
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
    m_settingsPanel->setTheme(m_appliedTheme);
  }

  if (localeChanged) {
    TranslationManager::instance().loadLocale(store.locale());
    m_appliedLocale = store.locale();
    applyTranslations();
    m_settingsPanel->setLocale(m_appliedLocale);
  }
}

void MainWindow::applyTheme() {
#ifdef HELIOS_THEME_TIMING
  QElapsedTimer timer;
  timer.start();
#endif

  auto &tm = ThemeManager::instance();
  auto &appearance = AppearanceController::instance();
  QPalette pal = tm.palette();
  QApplication::setPalette(pal);
  setStyleSheetIfChanged(this, baseStyle());

  setStyleSheetIfChanged(
      m_tabWidget,
      QString("QTabWidget::pane { border: none; background: %1; }"
              "QTabBar { background: %1; }"
              "QTabBar::tab { background: %1; color: %2; padding: 4px 14px;"
              "  border-right: 1px solid %3; font-size: %7; }"
              "QTabBar::tab:selected { color: %4; background: %5; }"
              "QTabBar::tab:hover:!selected { background: %6; }")
          .arg(tm.customColor("tabBarBg", QColor("#11111b")).name(),
               tm.customColor("tabFg", QColor("#9ca0b0")).name(),
               tm.customColor("tabBorder", QColor("#1e1e2e")).name(),
               tm.customColor("tabSelectedFg", QColor("#c6d0f5")).name(),
               tm.customColor("editorBg", QColor("#1e1e2e")).name(),
               tm.customColor("tabHoverBg", QColor("#181825")).name(),
               QString::number(qMax(appearance.uiFont().pointSize() - 2, appearance.minFontSize()))));

  setStyleSheetIfChanged(
      m_splitter,
      QString("QSplitter::handle { background: %1; }")
          .arg(tm.customColor("sidebarBorder", QColor("#1e1e2e")).name()));

#ifdef HELIOS_THEME_TIMING
  qDebug() << "Theme application completed in" << timer.elapsed() << "ms";
#endif
}

void MainWindow::showPreferences()
{
  if (!m_preferencesDialog) {
      m_preferencesDialog = new PreferencesDialog(this);
  }
  if (m_preferencesDialog->isVisible()) {
      m_preferencesDialog->close();
      return;
  }
  m_preferencesDialog->show();
  m_preferencesDialog->raise();
  m_preferencesDialog->activateWindow();
}


void MainWindow::showVimHelpDialog()
{
  if (!m_vimHelpDialog) {
      m_vimHelpDialog = new VimHelpDialog(this);
  }
  if (m_vimHelpDialog->isVisible()) {
      m_vimHelpDialog->close();
      return;
  }
  m_vimHelpDialog->show();
  m_vimHelpDialog->raise();
  m_vimHelpDialog->activateWindow();
}

void MainWindow::applyTranslations() {
  auto &tr = TranslationManager::instance();

  m_fileMenu->setTitle(tr.translate("menu.file"));
  m_newAct->setText(tr.translate("menu.new_file"));
  m_newProjAct->setText(tr.translate("menu.new_project"));
  m_openAct->setText(tr.translate("menu.open_file"));
  m_saveAct->setText(tr.translate("menu.save"));
  m_exitAct->setText(tr.translate("menu.exit"));

  m_toolsMenu->setTitle(tr.translate("menu.tools"));
  m_buildAct->setText(tr.translate("menu.build"));
  m_checkAct->setText(tr.translate("menu.check"));
  m_compileAct->setText(tr.translate("menu.compile"));
  m_runAct->setText(tr.translate("menu.run"));
  m_restartLspAct->setText(tr.translate("menu.restart_lsp"));

  m_viewMenu->setTitle(tr.translate("menu.view"));
  m_preferencesAct->setText(tr.translate("menu.preferences"));
  m_vimHelpAct->setText(tr.translate("menu.vim_motions"));
  m_shortcutsAct->setText(tr.translate("menu.shortcuts"));
  m_lspManagerAct->setText(tr.translate("menu.lsp_manager"));
  m_helpMenu->setTitle(tr.translate("menu.help"));
  m_gettingStartedAct->setText(tr.translate("welcome.getting_started"));

  m_activityBar->setButtonToolTip(
      ActivityBar::Explorer,
      tr.translate("shortcut.explorer") + " (Ctrl+Shift+E)");
  m_activityBar->setButtonToolTip(
      ActivityBar::Search,
      tr.translate("shortcut.global_search") + " (Ctrl+Shift+F)");
  m_activityBar->setButtonToolTip(
      ActivityBar::Git,
      tr.translate("shortcut.git_panel") + " (Ctrl+Shift+G)");
  m_activityBar->setButtonToolTip(
      ActivityBar::Settings,
      tr.translate("shortcut.settings") + " (Ctrl+,)");
}

void MainWindow::applyAppearanceChange()
{
  for (int i = 0; i < m_tabWidget->count(); ++i)
    applyEditorPreferences(qobject_cast<CodeEditor *>(m_tabWidget->widget(i)));
  menuBar()->setFont(AppearanceController::instance().uiFont());
  applyTheme();
}

bool MainWindow::lspEnabled() const {
  return TomlSettingsStore::instance().lspEnabled();
}

void MainWindow::setLspEnabled(bool enabled) {
  TomlSettingsStore::instance().setLspEnabled(enabled);
  if (m_settingsPanel)
    m_settingsPanel->setLspEnabled(enabled);
  if (m_lspManagerDialog)
    m_lspManagerDialog->setLspEnabled(enabled);

  if (m_restartLspAct)
    m_restartLspAct->setEnabled(enabled);

  if (!enabled) {
    if (m_zithToolchainManager)
      m_zithToolchainManager->cancel();
    if (m_lspClient)
      m_lspClient->stop();
    if (m_completionModel)
      m_completionModel->setItems({});
    if (m_diagnosticsPanel)
      m_diagnosticsPanel->clear();
    if (m_outlinePanel)
      m_outlinePanel->clear();

    m_runtimeTag.clear();
    m_activeLspPath.clear();
    m_activeStdlibPath.clear();

    m_runtimeStatusText = "Disabled";
    setLspStatus("LSP Disabled", "#6c7086");
    updateSettingsRuntimeInfo();
    updateLspDiagnostics();
  } else {
    m_lastLspError.clear();
    ensureLspRuntime(true);
  }
}

void MainWindow::ensureLspRuntime(bool preferCached) {
  if (!m_zithToolchainManager || !lspEnabled())
    return;

  m_runtimeStatusText = preferCached ? "Resolving latest Zith runtime..."
                                     : "Refreshing Zith runtime...";
  setLspStatus("LSP ○", "#f9e2af");
  updateSettingsRuntimeInfo();
  m_zithToolchainManager->ensureLatest(preferCached);
}

void MainWindow::startLspRuntime(const QString &lspPath,
                                 const QString &stdlibPath,
                                 const QString &tag) {
  if (!lspEnabled())
    return;
  QString currentWorkspace =
      m_contextManager ? m_contextManager->currentRoot() : QString();
  if (m_lspClient->isRunning() && m_activeLspPath == lspPath &&
      m_activeStdlibPath == stdlibPath &&
      m_activeWorkspaceRoot == currentWorkspace) {
    m_runtimeTag = tag;
    m_runtimeStatusText = QString("Runtime %1 already active.").arg(tag);
    updateSettingsRuntimeInfo();
    statusBar()->showMessage(
        QString("Zith runtime %1 is already active.").arg(tag), 3000);
    return;
  }

  m_zithToolchainManager->cancel();
  if (m_lspClient->isRunning())
    m_lspClient->stop();

  m_completionModel->setItems({});
  m_runtimeTag = tag;
  m_activeLspPath = lspPath;
  m_activeStdlibPath = stdlibPath;
  m_activeWorkspaceRoot = currentWorkspace;
  m_runtimeStatusText = QString("Starting runtime %1...").arg(tag);

  setLspStatus("LSP ○", "#f9e2af");
  updateSettingsRuntimeInfo();
  if (m_lspClient->start(lspPath, stdlibPath, currentWorkspace)) {
    statusBar()->showMessage(QString("Starting Zith runtime %1...").arg(tag),
                             3000);
  } else {
    setLspStatus("LSP !", "#e78284");
    m_runtimeStatusText = "Failed to start the resolved Zith runtime.";
    updateSettingsRuntimeInfo();
    statusBar()->showMessage("Failed to start LSP server", 5000);
  }
}

void MainWindow::updateSettingsRuntimeInfo() {
  if (!m_zithToolchainManager)
    return;

  if (m_settingsPanel) {
    m_settingsPanel->setRuntimeInfo(
        m_runtimeStatusText, m_runtimeTag, m_activeLspPath, m_activeStdlibPath,
        m_zithToolchainManager->runtimeCacheRootPath());
  }
  if (m_lspManagerDialog) {
    m_lspManagerDialog->setRuntimeInfo(
        m_runtimeStatusText, m_runtimeTag, m_activeLspPath, m_activeStdlibPath,
        m_zithToolchainManager->runtimeCacheRootPath());
  }

  updateLspDiagnostics();
}

void MainWindow::updateLspDiagnostics() {
  if (!m_settingsPanel && !m_lspManagerDialog)
    return;

  QString connection = "Not started";
  if (m_lspClient) {
    if (m_lspClient->isReady())
      connection = "Connected";
    else if (m_lspClient->isRunning())
      connection = "Starting (waiting for initialize)";
    else
      connection = "Stopped";
  }

  QString syncMode = "Unknown";
  if (m_lspClient && m_lspClient->isReady()) {
    switch (m_lspClient->documentSyncKind()) {
    case 2:
      syncMode = "Incremental";
      break;
    case 1:
      syncMode = "Full";
      break;
    case 0:
      syncMode = "None";
      break;
    default:
      syncMode = "Unknown";
      break;
    }
  }

  if (m_settingsPanel)
    m_settingsPanel->setLspDiagnostics(connection, syncMode, m_lastLspError);
  if (m_lspManagerDialog)
    m_lspManagerDialog->setLspDiagnostics(connection, syncMode, m_lastLspError);
}

void MainWindow::clearRuntimeCache() {
  if (!m_zithToolchainManager || !lspEnabled())
    return;

  const QMessageBox::StandardButton result = QMessageBox::question(
      this, "Clear Zith runtime cache?",
      "This removes the cached zith-lsp binary and stdlib. Helios will fetch "
      "them again on the next refresh.",
      QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
  if (result != QMessageBox::Yes)
    return;

  m_zithToolchainManager->cancel();
  if (m_lspClient->isRunning())
    m_lspClient->stop();

  QString errorMessage;
  if (!m_zithToolchainManager->clearCachedRuntime(&errorMessage)) {
    m_runtimeStatusText = errorMessage;
    setLspStatus("LSP !", "#e78284");
    updateSettingsRuntimeInfo();
    statusBar()->showMessage(errorMessage, 8000);
    return;
  }

  m_completionModel->setItems({});
  m_runtimeTag.clear();
  m_activeLspPath.clear();
  m_activeStdlibPath.clear();
  m_runtimeStatusText =
      "Runtime cache cleared. Resolving latest Zith runtime...";
  setLspStatus("LSP ○", "#f9e2af");
  updateSettingsRuntimeInfo();
  statusBar()->showMessage("Zith runtime cache cleared.", 4000);
  if (lspEnabled())
    ensureLspRuntime(false);
  else {
    m_runtimeStatusText = "Disabled";
    updateSettingsRuntimeInfo();
  }
}

void MainWindow::releaseEditor(CodeEditor *editor) {
  if (!editor)
    return;

  if (lspEnabled() && m_lspClient && m_lspClient->isReady() &&
      !editor->fileUri().isEmpty())
    m_lspClient->closeDocument(editor->fileUri());
  if (m_diagnosticsPanel)
    m_diagnosticsPanel->clearDiagnostics(editor->fileUri());

  const int idx = m_tabWidget->indexOf(editor);
  if (idx >= 0)
    m_tabWidget->removeTab(idx);
  if (m_highlighters.contains(editor)) {
    auto *hl = m_highlighters.take(editor);
    if (hl) {
      hl->setDocument(nullptr);
      delete hl;
    }
  }
  editor->deleteLater();
}

void MainWindow::setLspStatus(const QString &text, const QString &color) {
  if (!m_lspLabel)
    return;

  m_lspLabel->setText(text);
  m_lspLabel->setStyleSheet(QString("color: %1; padding: 0 4px;").arg(color));
}

void MainWindow::saveAllForLsp() {
  for (int i = 0; i < m_tabWidget->count(); ++i) {
    auto *editor = qobject_cast<CodeEditor *>(m_tabWidget->widget(i));
    if (!editor)
      continue;
    editor->flushPendingLspChanges();
    if (editor->filePath().isEmpty()) {
      if (editor->document()->isModified() && m_settingsPanel)
        m_settingsPanel->appendLspLog(
            "Cannot save untitled document requested by zith/requestSaveAll.");
      if (editor->document()->isModified() && m_lspManagerDialog)
        m_lspManagerDialog->appendLspLog(
            "Cannot save untitled document requested by zith/requestSaveAll.");
      continue;
    }
    if (!editor->document()->isModified())
      continue;
    QFile file(editor->filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      if (m_settingsPanel)
        m_settingsPanel->appendLspLog("Could not save " + editor->filePath());
      if (m_lspManagerDialog)
        m_lspManagerDialog->appendLspLog("Could not save " + editor->filePath());
      continue;
    }
    QTextStream stream(&file);
    stream << editor->toPlainText();
    file.close();
    editor->document()->setModified(false);
    if (m_lspClient)
      m_lspClient->saveDocument(editor->fileUri());
  }
}

void MainWindow::applyWorkspaceEdit(const QJsonObject &edit) {
  if (edit.contains("documentChanges") || !edit.value("changes").isObject()) {
    statusBar()->showMessage("Unsupported workspace edit from LSP.", 5000);
    return;
  }
  struct Target {
    QString uri;
    QString text;
    QList<QPair<LspRange, QString>> edits;
    CodeEditor *editor = nullptr;
  };
  QList<Target> targets;
  const QJsonObject changes = edit.value("changes").toObject();
  for (auto it = changes.begin(); it != changes.end(); ++it) {
    const QUrl url(it.key());
    if (!url.isLocalFile()) {
      statusBar()->showMessage("Workspace edit contains a non-local URI.",
                               5000);
      return;
    }
    Target target;
    target.uri = it.key();
    for (int i = 0; i < m_tabWidget->count(); ++i) {
      auto *candidate = qobject_cast<CodeEditor *>(m_tabWidget->widget(i));
      if (candidate && candidate->fileUri() == target.uri) {
        target.editor = candidate;
        target.text = candidate->toPlainText();
        break;
      }
    }
    if (!target.editor) {
      QFile file(url.toLocalFile());
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        statusBar()->showMessage("Could not read workspace-edit target.", 5000);
        return;
      }
      target.text = QString::fromUtf8(file.readAll());
    }
    for (const QJsonValue &value : it.value().toArray()) {
      const QJsonObject json = value.toObject();
      target.edits.append({rangeFromJson(json.value("range").toObject()),
                           json.value("newText").toString()});
    }
    targets.append(target);
  }
  for (const Target &target : targets) {
    for (const auto &entry : target.edits) {
      bool valid = true;
      const int start =
          offsetForPosition(target.text, entry.first.start, &valid);
      const int end = offsetForPosition(target.text, entry.first.end, &valid);
      if (!valid || end < start) {
        statusBar()->showMessage(
            "Workspace edit contains an invalid range; no files changed.",
            5000);
        return;
      }
    }
  }
  for (Target &target : targets) {
    std::sort(target.edits.begin(), target.edits.end(),
              [](const auto &a, const auto &b) {
                if (a.first.start.line != b.first.start.line)
                  return a.first.start.line > b.first.start.line;
                return a.first.start.character > b.first.start.character;
              });
    if (target.editor) {
      target.editor->applyEdits(target.edits);
      target.editor->document()->setModified(true);
    } else {
      QString updated = target.text;
      for (const auto &entry : target.edits) {
        bool valid = true;
        const int start = offsetForPosition(updated, entry.first.start, &valid);
        const int end = offsetForPosition(updated, entry.first.end, &valid);
        if (!valid)
          return;
        updated.replace(start, end - start, entry.second);
      }
      QSaveFile file(QUrl(target.uri).toLocalFile());
      if (!file.open(QIODevice::WriteOnly) ||
          file.write(updated.toUtf8()) < 0 || !file.commit()) {
        statusBar()->showMessage("Could not write workspace-edit target.",
                                 5000);
        return;
      }
    }
  }
}

void MainWindow::onFrontendStatusReceived(const QJsonObject &status) {
    QString state = status.value("state").toString();
    QString msg = status.value("message").toString();
    if (state == "warming") {
        m_runtimeStatusText = "Frontend warming up...";
        setLspStatus("LSP ◐", "#89b4fa");
    } else if (state == "ready") {
        m_runtimeStatusText = "Frontend ready";
        setLspStatus("LSP ⬤", "#a6d189");
    } else if (state == "error") {
        m_runtimeStatusText = "Frontend error: " + msg;
        setLspStatus("LSP !", "#e78284");
    }
    if (m_settingsPanel) m_settingsPanel->appendLspLog("Frontend status: " + state + (msg.isEmpty() ? "" : " - " + msg));
    if (m_lspManagerDialog) m_lspManagerDialog->appendLspLog("Frontend status: " + state + (msg.isEmpty() ? "" : " - " + msg));
}

void MainWindow::onMetricsReceived(const QJsonObject &metrics) {
    if (m_settingsPanel) m_settingsPanel->appendLspLog("Metrics: " + QString::fromUtf8(QJsonDocument(metrics).toJson(QJsonDocument::Compact)));
    if (m_lspManagerDialog) m_lspManagerDialog->appendLspLog("Metrics: " + QString::fromUtf8(QJsonDocument(metrics).toJson(QJsonDocument::Compact)));
}
