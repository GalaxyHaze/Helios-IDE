#include "MainWindow.h"
#include "../panels/OutlinePanel.h"
#include "../panels/WelcomeWidget.h"
#include "../panels/ShortcutsDialog.h"
#include "../panels/LspManagerDialog.h"
#include "ThemeManager.h"
#include "AppearanceController.h"
#include "TomlSettingsStore.h"
#include "TranslationManager.h"
#include "FileIcons.h"

#include "../editor/Code.h"
#include "../editor/LspClient.h"
#include "../editor/LspCompletionModel.h"
#include "../editor/Syntax.h"
#include "../editor/CHighlighter.h"
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
#include "../panels/BottomPanel.h"
#include "../panels/VimHelpDialog.h"
#include "../widgets/BreadcrumbsBar.h"
#include "../widgets/FindReplaceBar.h"
#include "ContextManager.h"
#include "SnippetManager.h"
#include "ZithToolchainManager.h"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
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
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcessEnvironment>
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
      keyLbl->setStyleSheet(
          QString("font-weight: bold; color: %1;")
              .arg(ThemeManager::instance()
                       .semanticColor(ThemeManager::SemanticRole::Accent)
                       .name()));
      sgLayout->addWidget(descLbl, row, 0);
      sgLayout->addWidget(keyLbl, row, 1);
      row++;
    };

    addShortcut("Open Folder", "Ctrl+Alt+O");
    addShortcut("New File", "Ctrl+N");
    addShortcut("Toggle Project Explorer", "Ctrl+Shift+E");
    addShortcut("Search Workspace", "Ctrl+Shift+F");
    addShortcut("Build Project", "Ctrl+B");
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
        QString("background: %1; color: %2; padding: 6px 15px; border: none; "
                "border-radius: 4px; font-weight: bold;")
            .arg(ThemeManager::instance()
                     .semanticColor(ThemeManager::SemanticRole::Accent)
                     .name(),
                 ThemeManager::instance()
                     .semanticColor(ThemeManager::SemanticRole::OnAccent)
                     .name()));
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
            .arg(tm.semanticColor(ThemeManager::SemanticRole::Canvas).name(),
                 tm.semanticColor(ThemeManager::SemanticRole::Text).name(),
                 tm.semanticColor(ThemeManager::SemanticRole::Border).name()));

    connect(dontShowAgain, &QCheckBox::checkStateChanged, this, [](int state) {
      TomlSettingsStore::instance().setOnboardingDismissed(state ==
                                                           Qt::Checked);
    });
  }
};

static QString baseStyle() {
  auto &tm = ThemeManager::instance();
  QString bg = tm.semanticColor(ThemeManager::SemanticRole::Canvas).name();
  QString text = tm.semanticColor(ThemeManager::SemanticRole::Text).name();
  QString base = tm.semanticColor(ThemeManager::SemanticRole::SurfaceAlt).name();
  QString altBase = tm.palette().color(QPalette::AlternateBase).name();
  QString highlight = tm.semanticColor(ThemeManager::SemanticRole::Selected).name();
  QString border = tm.semanticColor(ThemeManager::SemanticRole::Border).name();
  QString hover = tm.semanticColor(ThemeManager::SemanticRole::Hover).name();
  QString faint =
      tm.semanticColor(ThemeManager::SemanticRole::TextFaint).name();
  const QString inputBg =
      tm.semanticColor(ThemeManager::SemanticRole::InputBg).name();
  const QString buttonBg =
      tm.semanticColor(ThemeManager::SemanticRole::ButtonBg).name();
  const QString accent =
      tm.semanticColor(ThemeManager::SemanticRole::Accent).name();
  const QColor shadowBase =
      tm.isDark() ? QColor(24, 18, 52) : QColor(70, 58, 106);
  const int shadowAlpha = tm.isDark() ? 190 : 125;
  const QString shadow = QStringLiteral("rgba(%1, %2, %3, %4)")
                             .arg(shadowBase.red())
                             .arg(shadowBase.green())
                             .arg(shadowBase.blue())
                             .arg(shadowAlpha);

  return QString(
             "QMainWindow, QWidget { background: %1; color: %2; }"
             "QMenuBar { background: %1; color: %2; border-bottom: 1px solid "
             "%3; padding: 2px 4px; }"
             "QMenuBar::item:selected { background: %4; }"
             "QMenuBar::item:pressed { background: %5; }"
             "QMenu { background: %1; color: %2; border: 1px solid %3; "
             "border-left: 2px solid %11; border-bottom: 2px solid %11; "
             "border-radius: 6px; padding: 4px; }"
             "QMenu::item:selected { background: %4; }"
             "QMenu::item:disabled { color: %7; }"
             "QMenu::separator { height: 1px; background: %3; margin: 4px 8px; "
             "}"
             "QStatusBar { background: %1; color: %2; border-top: 1px solid "
             "%3; font-size: 11px; }"
             "QStatusBar::item { border: none; }"
             "QDockWidget { background: %1; color: %2; }"
             "QDockWidget::title { background: %6; color: %2; padding: 4px; }"
             "QScrollBar:vertical { background: transparent; width: 7px; "
             "margin: 2px 3px; }"
             "QScrollBar:vertical:hover { background: transparent; width: 9px; }"
             "QScrollBar::handle:vertical { background: %4; border-radius: 999px; "
             "min-height: 22px; margin: 0 1px; }"
             "QScrollBar::handle:vertical:hover { background: %10; }"
             "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical, "
             "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { "
             "width: 0px; height: 0px; background: transparent; }"
             "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical, "
             "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { "
             "background: transparent; }"
             "QScrollBar:horizontal { background: transparent; height: 7px; "
             "margin: 3px 2px; }"
             "QScrollBar:horizontal:hover { background: transparent; height: 9px; }"
             "QScrollBar::handle:horizontal { background: %4; border-radius: 999px; "
             "min-width: 22px; margin: 1px 0; }"
             "QScrollBar::handle:horizontal:hover { background: %10; }"
             "QPushButton, QToolButton { background: %9; color: %2; border: 1px "
             "solid %3; border-left: 2px solid %11; border-bottom: 2px solid %11; "
             "border-radius: 6px; padding: 6px 12px; }"
             "QPushButton:hover, QToolButton:hover { background: %4; border-left: "
             "3px solid %11; border-bottom: 3px solid %11; }"
             "QPushButton:pressed, QToolButton:pressed { background: %5; "
             "border-left: 1px solid %11; border-bottom: 1px solid %11; }"
             "QLineEdit, QComboBox, QPlainTextEdit, QTextEdit { background: %8; "
             "color: %2; border: 1px solid %3; border-radius: 6px; "
             "padding: 5px 8px; selection-background-color: %5; }"
             "QLineEdit:focus, QComboBox:focus, QPlainTextEdit:focus, "
             "QTextEdit:focus { border: 1px solid %10; }")
      .arg(bg, text, border, hover, highlight, altBase, faint,
           inputBg, buttonBg, accent, shadow);
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
  editorLayout->addWidget(m_tabWidget, 1);
  editorLayout->addWidget(m_breadcrumbs);
  editorLayout->addWidget(m_findReplaceBar);

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
  m_sidePanel->setMinimumWidth(180);
  m_sidePanel->setMaximumWidth(520);

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
  m_outlinePanel->setVisible(settings.outlineVisible());
  connect(m_splitter, &QSplitter::splitterMoved, this, [](int pos, int) {
      TomlSettingsStore::instance().setSidebarWidth(pos);
  });

  m_outlineSymbolsTimer = new QTimer(this);
  m_outlineSymbolsTimer->setSingleShot(true);
  m_outlineSymbolsTimer->setInterval(150);
  connect(m_outlineSymbolsTimer, &QTimer::timeout, this, [this]() {
    if (!m_outlinePanel->isVisible() || m_outlineSymbolsUri.isEmpty())
      return;
    CodeEditor *ed = currentEditor();
    if (!ed || ed->fileUri() != m_outlineSymbolsUri)
      return;
    if (auto *client = lspClientForPath(ed->filePath());
        client && client->isReady()) {
      client->requestDocumentSymbols(m_outlineSymbolsUri,
                                     m_outlineSymbolsVersion);
    }
  });

  setCentralWidget(m_splitter);

  m_snippetManager = new SnippetManager(this);
  m_snippetManager->loadFromJson(":/snippets/zith-snippets.json");

  m_completionModel = new LspCompletionModel(this);
  m_completer = new LspCompleter(m_completionModel, this);

  m_zithLspClient = new LspClient(this);
  m_clangdClient = new LspClient(this);
  m_zithToolchainManager = new ZithToolchainManager(this);
  m_zithToolchainManager->setPreferOnline(
      TomlSettingsStore::instance().useOnlineZithLsp());

  applyAppearanceChange();

  const auto connectCompletion = [this](LspClient *client) {
    connect(client, &LspClient::completionResults, this,
            [this](const QString &uri, int,
                   const QList<LspCompletionItem> &items) {
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
  };
  connectCompletion(m_zithLspClient);
  connectCompletion(m_clangdClient);

  connect(m_zithLspClient, &LspClient::initialized, this, [this]() {
  if (!lspEnabled()) {
      m_zithLspClient->stop();
      return;
    }
    updateRunActionsEnabled();

    setLspStatus("LSP ⬤",
                 ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Success).name());
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
      if (isZithEditor(ed))
        m_zithLspClient->openDocument(
            ed->fileUri(), lspLanguageId(EditorLanguage::Zith),
            ed->toPlainText(), ed->documentVersion());
    }
    m_lastLspError.clear();
    updateLspDiagnostics();
  });

  connect(m_zithLspClient, &LspClient::serverStopped, this, [this]() {
    for (int i = 0; i < m_tabWidget->count(); ++i) {
      auto *ed = qobject_cast<CodeEditor *>(m_tabWidget->widget(i));
      if (ed && ed->lspClient() == m_zithLspClient)
        ed->detachLspClient();
    }
    updateRunActionsEnabled();
    if (!lspEnabled()) {
      m_runtimeStatusText = "Disabled";
      setLspStatus("LSP Disabled",
                   ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::TextFaint).name());
      updateSettingsRuntimeInfo();
      updateLspDiagnostics();
      return;
    }

    setLspStatus("LSP ○",
                 ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Warning).name());
    m_runtimeStatusText = "LSP server stopped.";
    updateSettingsRuntimeInfo();
    updateLspDiagnostics();
  });

  connect(m_zithLspClient, &LspClient::processStopped, this,
          [this](bool expected) {
    m_runOutput.clear();
    m_activeProgressToken.clear();
    if (m_compilerPanel) {
      m_compilerPanel->clearActiveProgress();
      m_compilerPanel->appendOutput(
          "LSP stopped; any active process is no longer being tracked.");
      m_compilerPanel->setRunningTask({});
    }
    updateRunActionsEnabled();
    if (expected || !lspEnabled() || m_activeLspPath.isEmpty())
      return;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    while (!m_lspRestartTimes.isEmpty() &&
           m_lspRestartTimes.first() <= now - 60000)
      m_lspRestartTimes.removeFirst();
    if (m_lspRestartTimes.size() >= 3) {
      m_runtimeStatusText =
          "LSP crashed repeatedly; automatic restart stopped.";
      setLspStatus("LSP !",
                   ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Error).name());
      updateSettingsRuntimeInfo();
      return;
    }
    m_lspRestartTimes.append(now);
    const int delaySeconds = 1 << (m_lspRestartTimes.size() - 1);
    m_runtimeStatusText =
        QString("LSP stopped; restarting in %1 second(s)...").arg(delaySeconds);
    setLspStatus("LSP ○",
                 ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Warning).name());
    updateSettingsRuntimeInfo();
    QTimer::singleShot(delaySeconds * 1000, this, [this]() {
      if (lspEnabled() && m_zithLspClient &&
          !m_zithLspClient->isRunning())
        m_zithLspClient->start(m_activeLspPath, m_activeStdlibPath,
                               m_activeWorkspaceRoot);
    });
  });

  const auto appendLspLog = [this](const QString &msg) {
    if (lspEnabled() && m_settingsPanel)
      m_settingsPanel->appendLspLog(msg);
    if (m_lspManagerDialog)
      m_lspManagerDialog->appendLspLog(msg);
  };
  const auto connectServerError = [this, appendLspLog](LspClient *client) {
    connect(client, &LspClient::serverError, this,
            [this, appendLspLog, client](const QString &msg) {
              if (!lspEnabled())
                return;
              if (client == m_zithLspClient) {
                setLspStatus("LSP !",
                             ThemeManager::instance()
                                 .semanticColor(
                                     ThemeManager::SemanticRole::Error)
                                 .name());
        m_runtimeStatusText = "LSP error: " + msg;
                m_lastLspError = msg;
                updateSettingsRuntimeInfo();
                updateLspDiagnostics();
              } else {
                m_activeCLspPath.clear();
                m_clangdError = msg;
                updateClangdRuntimeInfo();
              }
              appendLspLog("[error] " + msg);
              statusBar()->showMessage("LSP: " + msg, 5000);
            });
  };
  connectServerError(m_zithLspClient);
  connectServerError(m_clangdClient);

  connect(m_zithLspClient, &LspClient::logMessage, this, appendLspLog);
  connect(m_zithLspClient, &LspClient::frontendStatusReceived, this,
          &MainWindow::onFrontendStatusReceived);
  connect(m_zithLspClient, &LspClient::metricsReceived, this,
          &MainWindow::onMetricsReceived);
  connect(m_clangdClient, &LspClient::logMessage, this, appendLspLog);

  const auto connectFormatting = [this](LspClient *client) {
    connect(client, &LspClient::formattingResult, this,
            [this](const QString &uri, int version,
                   const QList<QPair<LspRange, QString>> &edits) {
              if (auto *ed = currentEditor();
                  ed && ed->fileUri() == uri &&
                  ed->documentVersion() == version) {
                ed->applyEdits(edits);
              }
            });
  };
  connectFormatting(m_zithLspClient);
  connectFormatting(m_clangdClient);

  const auto connectSymbols = [this](LspClient *client) {
    connect(client, &LspClient::documentSymbolsResult, this,
            [this](const QString &uri, int version,
                   const QJsonArray &symbols) {
              if (auto *ed = currentEditor();
                  ed && ed->fileUri() == uri &&
                  ed->documentVersion() == version)
                m_outlinePanel->setSymbols(symbols);
            });
  };
  connectSymbols(m_zithLspClient);
  connectSymbols(m_clangdClient);
  connect(m_outlinePanel, &OutlinePanel::symbolSelected, this,
          [this](int line, int col) {
            if (auto *ed = currentEditor()) {
              ed->goToLine(line, col);
            }
          });

  m_contextLabel = new QLabel("◀ 1/1 ▶");
  m_contextLabel->setStyleSheet("");

  m_lspLabel = new QLabel("LSP ⬤");
  m_lspLabel->setStyleSheet("");

  m_errorLabel = new QLabel("✕0  ⚠0");
  m_errorLabel->setStyleSheet("");

  m_posLabel = new QLabel("Ln 1, Col 1");
  m_posLabel->setStyleSheet("");

  m_indentLabel = new QLabel("Spaces: 4");
  m_indentLabel->setStyleSheet("");

  m_encodingLabel = new QLabel("UTF-8");
  m_encodingLabel->setStyleSheet("");

  m_langLabel = new QLabel("Zith");
  m_langLabel->setStyleSheet("");

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

            setLspStatus("LSP !",
                         ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Error).name());
            m_runtimeStatusText = message;
            updateSettingsRuntimeInfo();
            statusBar()->showMessage(message, 8000);
          });
  connect(m_zithToolchainManager, &ZithToolchainManager::ready, this,
          &MainWindow::startLspRuntime);

  connect(m_clangdClient, &LspClient::initialized, this, [this]() {
    for (int i = 0; i < m_tabWidget->count(); ++i) {
      auto *ed = qobject_cast<CodeEditor *>(m_tabWidget->widget(i));
      if (isCFamilyEditor(ed) && ed->lspClient() == m_clangdClient)
        m_clangdClient->openDocument(
            ed->fileUri(), lspLanguageId(EditorLanguage::CFamily),
            ed->toPlainText(), ed->documentVersion());
    }
    m_clangdError.clear();
    updateClangdRuntimeInfo();
  });
  connect(m_clangdClient, &LspClient::serverStopped, this, [this]() {
    for (int i = 0; i < m_tabWidget->count(); ++i) {
      auto *ed = qobject_cast<CodeEditor *>(m_tabWidget->widget(i));
      if (ed && ed->lspClient() == m_clangdClient)
        ed->detachLspClient();
    }
    updateClangdRuntimeInfo();
  });

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

  connect(m_searchPanel, &SearchPanel::replaceAllPreviewReady, this,
          &MainWindow::onReplaceAllPreviewReady);

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
  connect(m_lspManagerDialog, &LspManagerDialog::useOnlineZithLspChanged, this,
          [this](bool enabled) {
            TomlSettingsStore::instance().setUseOnlineZithLsp(enabled);
            m_zithToolchainManager->setPreferOnline(enabled);
            if (lspEnabled())
              ensureLspRuntime(false);
          });
  connect(m_lspManagerDialog, &LspManagerDialog::cLspEnabledChanged, this,
          [this](bool enabled) {
            TomlSettingsStore::instance().setCLspEnabled(enabled);
            updateClangdLifecycle();
          });
  connect(m_lspManagerDialog, &LspManagerDialog::cLspPathChanged, this,
          [this](const QString &path) {
            TomlSettingsStore::instance().setCLspPath(path);
            m_activeCLspPath.clear();
            updateClangdLifecycle();
          });
  connect(m_lspManagerDialog, &LspManagerDialog::refreshRuntimeRequested, this,
          [this]() {
            if (lspEnabled())
              ensureLspRuntime(false);
          });
  connect(m_lspManagerDialog, &LspManagerDialog::clearRuntimeCacheRequested,
          this, &MainWindow::clearRuntimeCache);

  updateSettingsRuntimeInfo();
  updateClangdRuntimeInfo();
  m_lspManagerDialog->setCLspEnabled(cLspEnabled());
  m_lspManagerDialog->setUseOnlineZithLsp(
      TomlSettingsStore::instance().useOnlineZithLsp());
  m_lspManagerDialog->setCLspPath(TomlSettingsStore::instance().cLspPath());
  updateClangdRuntimeInfo();

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
              if (lspEnabled() && m_zithLspClient &&
                  m_zithLspClient->isRunning()) {
                ensureLspRuntime(true);
              }
              updateClangdLifecycle();
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

  m_bottomPanel = new BottomPanel(this);
  m_diagnosticsPanel = m_bottomPanel->diagnostics();
  m_referencesPanel = m_bottomPanel->references();
  m_compilerPanel = m_bottomPanel->compiler();
  addDockWidget(Qt::BottomDockWidgetArea, m_bottomPanel);
  m_bottomPanel->hide();
  connect(m_bottomPanel, &BottomPanel::closeRequested, this, [this]() { setBottomPanelVisible(false); });

  m_activityBar = new ActivityBar(this);
  addDockWidget(Qt::LeftDockWidgetArea, m_activityBar);
  connect(m_activityBar, &ActivityBar::modeChanged, this,
          [this](ActivityBar::Mode mode) { setSidebarMode(mode); });
  connect(m_activityBar, &ActivityBar::preferencesRequested, this,
          &MainWindow::showPreferences);

  connect(m_zithLspClient, &LspClient::diagnosticsReceived,
          m_diagnosticsPanel, &DiagnosticsPanel::setDiagnostics);
  connect(m_clangdClient, &LspClient::diagnosticsReceived, m_diagnosticsPanel,
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
  const auto connectReferences = [this](LspClient *client) {
    connect(client, &LspClient::referencesResult, this,
            [this](const QString &, int, const QList<LspLocation> &locations) {
              if (!m_referencesPanel)
                return;
              m_referencesPanel->setReferences(locations);
              if (!locations.isEmpty())
                m_bottomPanel->showReferences();
            });
  };
  connectReferences(m_zithLspClient);
  connectReferences(m_clangdClient);

  const auto connectRename = [this](LspClient *client) {
    connect(client, &LspClient::renameResult, this,
            [this](const QString &, int, const QJsonObject &edit) {
              applyWorkspaceEdit(edit);
            });
  };
  connectRename(m_zithLspClient);
  connectRename(m_clangdClient);

  const auto connectCodeActions = [this](LspClient *client) {
    connect(client, &LspClient::codeActionsResult, this,
            [this, client](const QString &, int, const QJsonArray &actions) {
              if (actions.isEmpty())
                return;
              QMenu menu(this);
              for (const QJsonValue &value : actions) {
                const QJsonObject action = value.toObject();
                QAction *entry =
                    menu.addAction(action.value("title").toString("Code action"));
                const QJsonObject command = action.value("command").toObject();
                if (action.contains("edit"))
                  connect(entry, &QAction::triggered, this, [this, action] {
                    applyWorkspaceEdit(action.value("edit").toObject());
                  });
                else if (!command.isEmpty() && client &&
                         client->hasExecuteCommandProvider())
                  connect(entry, &QAction::triggered, this,
                          [this, client, command] {
                            client->executeWorkspaceCommand(
                                command.value("command").toString(),
                                command.value("arguments"), {});
                          });
                else
                  entry->setEnabled(false);
              }
              menu.exec(QCursor::pos());
            });
  };
  connectCodeActions(m_zithLspClient);
  connectCodeActions(m_clangdClient);

  const auto connectShowMessage = [this](LspClient *client) {
    connect(client, &LspClient::showMessage, this,
            [this](const QString &message) {
              statusBar()->showMessage(message, 5000);
            });
  };
  connectShowMessage(m_zithLspClient);
  connectShowMessage(m_clangdClient);

  // Process/runtime events belong to the Zith backend only. clangd never
  // serves compiler/run tasks, so it must not clear or write to this panel.
  connect(m_zithLspClient, &LspClient::processOutputReceived, this,
          [this](const QString &taskId, const QString &chunk) {
            if (!m_compilerPanel)
              return;
            const QString activeTask = m_compilerPanel->runningTaskId();
            if (activeTask.isEmpty()) {
              m_runOutput.buffer(taskId, chunk);
            } else if (activeTask == taskId) {
              m_compilerPanel->appendRawOutput(chunk.toUtf8());
            }
          });
  connect(m_zithLspClient, &LspClient::processExitReceived, this,
          [this](const QString &taskId, int exitCode) {
            if (!m_compilerPanel)
              return;
            if (m_compilerPanel->runningTaskId() != taskId) {
              m_runOutput.bufferExit(taskId, exitCode);
              return;
            }
            m_runOutput.discardFor(taskId);
            m_compilerPanel->appendRawOutput(
                (exitCode == 0 ? "Process exited with code 0"
                               : QString("Process exited with code %1")
                                     .arg(exitCode)).toUtf8());
            m_compilerPanel->setRunningTask({});
            updateRunActionsEnabled();
          });
  connect(m_zithLspClient, &LspClient::workDoneProgressReceived, this,
          &MainWindow::onWorkDoneProgressReceived);
  connect(m_zithLspClient, &LspClient::saveAllRequested, this,
          &MainWindow::saveAllForLsp);
  connect(m_zithLspClient, &LspClient::commandResult, this,
          &MainWindow::onLspCommandResult);

  connect(m_compilerPanel, &CompilerPanel::stopRequested, this,
          &MainWindow::stopRunningTask);
  connect(m_tabWidget, &QTabWidget::currentChanged, this,
          [this](int) { updateRunActionsEnabled(); });

  const QByteArray savedGeometry =
      TomlSettingsStore::instance().mainWindowGeometry();
  if (!savedGeometry.isEmpty())
    restoreGeometry(savedGeometry);

  const QByteArray savedState =
      TomlSettingsStore::instance().mainWindowState();
  if (!savedState.isEmpty())
    restoreState(savedState);

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
  connect(m_buildAct, &QAction::triggered, this, &MainWindow::runBuild);

  m_checkAct =
      m_toolsMenu->addAction("&Check File", QKeySequence("Ctrl+Shift+C"));
  connect(m_checkAct, &QAction::triggered, this, &MainWindow::runCheckFile);

  m_runAct = m_toolsMenu->addAction("&Run", QKeySequence("Ctrl+Shift+R"));
  connect(m_runAct, &QAction::triggered, this, &MainWindow::runProject);

  m_stopAct = m_toolsMenu->addAction("&Stop", QKeySequence("Ctrl+Shift+Q"));
  connect(m_stopAct, &QAction::triggered, this, &MainWindow::stopRunningTask);
  m_stopAct->setEnabled(false);
  updateRunActionsEnabled();

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
  m_outlineToggleAct =
      m_viewMenu->addAction("Structure", this, [this]() {
        const bool visible = !m_outlinePanel->isVisible();
        m_outlinePanel->setVisible(visible);
        TomlSettingsStore::instance().setOutlineVisible(visible);
        if (!visible && m_outlinePanel)
          m_outlinePanel->clear();
        else if (visible && currentEditor())
          updateEditorChrome(currentEditor());
      });
  m_outlineToggleAct->setCheckable(true);
  m_outlineToggleAct->setChecked(m_outlinePanel->isVisible());
  m_bottomToggleAct = m_viewMenu->addAction("Toggle Bottom Panel", this,
      [this]() { setBottomPanelVisible(!m_bottomPanel->isVisible()); });
  m_bottomToggleAct->setCheckable(true);
  m_bottomToggleAct->setChecked(m_bottomPanel->isVisible());

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

  // The sidebar visibility is kept in settings, while the selected mode and
  // active icon are kept in sync after restoreState may have changed either.
  const int sidebarIndex = qBound(0, m_sidePanel->currentIndex(),
                                  int(ActivityBar::Settings));
  m_activityBar->setActiveMode(ActivityBar::Mode(sidebarIndex),
                               m_sidePanel->isVisible());
  toggleFileTree(TomlSettingsStore::instance().sidebarVisible());

  if (lspEnabled()) {
    m_runtimeStatusText = "Resolving latest Zith runtime...";
    setLspStatus("LSP ○",
                 ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Warning).name());
    updateSettingsRuntimeInfo();
    ensureLspRuntime(true);
  } else {
    m_runtimeStatusText = "Disabled";
    setLspStatus("LSP Disabled",
                 ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::TextFaint).name());
    updateSettingsRuntimeInfo();
    updateLspDiagnostics();
  }
  updateClangdLifecycle();

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

void MainWindow::closeEvent(QCloseEvent *event) {
  TomlSettingsStore::instance().setMainWindowGeometry(saveGeometry());
  TomlSettingsStore::instance().setMainWindowState(saveState());
  QMainWindow::closeEvent(event);
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
  applyLanguageForEditor(editor, path);
  editor->clearDiagnostics();

  const int idx = m_tabWidget->indexOf(editor);
  m_tabWidget->setTabText(idx, QFileInfo(path).fileName());
  m_tabWidget->setTabToolTip(idx, path);
  m_tabWidget->setTabIcon(idx, fileIconForPath(path));

  if (m_diagnosticsPanel)
    m_diagnosticsPanel->clearDiagnostics(editor->fileUri());
  if (m_completionModel)
    m_completionModel->setItems({});

  m_tabWidget->setCurrentIndex(idx);
  updateEditorChrome(editor);
  updateCentralWidgetState();
  updateClangdLifecycle();

  QTimer::singleShot(0, this, [this, editor]() {
    syncEditorWithLsp(editor, true);
  });
}

CodeEditor *MainWindow::createTab(bool makeCurrent) {
  auto *editor = new CodeEditor;
  editor->setFont(AppearanceController::instance().editorFont());

  auto *hl = new SyntaxHighlighter(editor->document());
  m_highlighters.insert(editor, hl);

  editor->setSnippetManager(m_snippetManager);
  editor->setCompleter(m_completer);
  editor->setLspClient(nullptr);
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
  m_tabWidget->setTabIcon(idx, fileIconForSuffix({}));

  updateCentralWidgetState();
  return editor;
}

void MainWindow::applyLanguageForEditor(CodeEditor *editor, const QString &path) {
  if (!editor)
    return;

  auto *current = m_highlighters.value(editor, nullptr);

  if (languageForPath(path) == EditorLanguage::CFamily) {
    if (qobject_cast<CHighlighter *>(current))
      return;
    if (current) {
      current->setDocument(nullptr);
      delete current;
    }
    m_highlighters.insert(editor, new CHighlighter(editor->document()));
    return;
  }

  if (qobject_cast<SyntaxHighlighter *>(current) &&
      languageForPath(path) != EditorLanguage::PlainText)
    return;
  if (current) {
    current->setDocument(nullptr);
    delete current;
  }
  m_highlighters.insert(
      editor, new SyntaxHighlighter(editor->document()));
}

void MainWindow::connectEditorSignals(CodeEditor *editor) {
  connect(editor, &CodeEditor::zoomChanged, this,
          [](double scale) {
            AppearanceController::instance().setEditorFontSize(
                qRound(12 * scale));
          });

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
          [this, editor](const QString &uri, int version,
                         const LspPosition &position, const QString &name) {
            for (int i = 0; i < m_tabWidget->count(); ++i)
              if (auto *open =
                      qobject_cast<CodeEditor *>(m_tabWidget->widget(i)))
                open->flushPendingLspChanges();
            if (auto *client = lspClientForEditor(editor);
                client && client->isReady())
              client->requestRename(uri, version, position, name);
          });
  connect(
      editor, &CodeEditor::codeActionsRequested, this,
      [this, editor](const QString &uri, int version, const LspRange &range) {
        if (auto *client = lspClientForEditor(editor);
            client && client->isReady())
          client->requestCodeActions(uri, version, range,
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

  const bool wasUntitled = ed->filePath().isEmpty();
  if (ed->filePath().isEmpty()) {
    QString path = QFileDialog::getSaveFileName(
        this, "Save File As",
        m_contextManager ? m_contextManager->currentRoot() : QString(),
        "Zith Files (*.zith);;All Files (*)");
    if (path.isEmpty())
      return;
    LspClient *previousClient = ed->lspClient();
    const QString previousUri = ed->fileUri();
    if (previousClient && previousClient->isReady() && !previousUri.isEmpty())
      previousClient->closeDocument(previousUri);
    if (m_diagnosticsPanel)
      m_diagnosticsPanel->clearDiagnostics(previousUri);
    if (m_completionModel)
      m_completionModel->setItems({});
    ed->setFilePath(path);
    ed->setLspClient(nullptr);
    applyLanguageForEditor(ed, path);
    const int idx = m_tabWidget->currentIndex();
    m_tabWidget->setTabText(idx, QFileInfo(path).fileName());
    m_tabWidget->setTabToolTip(idx, path);
    m_tabWidget->setTabIcon(idx, fileIconForPath(path));
    updateEditorChrome(ed);
  }

  QFile file(ed->filePath());
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);
    out << ed->toPlainText();
    file.close();
    ed->document()->setModified(false);
    statusBar()->showMessage("File saved", 2000);
    syncEditorWithLsp(ed, wasUntitled);
    updateRunActionsEnabled();
    if (wasUntitled)
      updateClangdLifecycle();
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
      closeEditorWithLsp(editor);
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
  updateClangdRuntimeInfo();
  m_sidePanel->setCurrentWidget(m_settingsPanel);
  if (!m_sidePanel->isVisible())
    toggleFileTree(true);
  TomlSettingsStore::instance().setSidebarVisible(true);
  m_activityBar->setActiveMode(ActivityBar::Settings, true);
}

void MainWindow::setBottomPanelVisible(bool visible) {
  if (!m_bottomPanel)
    return;
  m_bottomPanel->setVisible(visible);
  if (m_bottomToggleAct)
    m_bottomToggleAct->setChecked(visible);
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
  updateClangdRuntimeInfo();
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
  if (m_langLabel) {
    switch (languageForPath(editor->filePath())) {
    case EditorLanguage::Zith:
      m_langLabel->setText("Zith");
      break;
    case EditorLanguage::CFamily:
      m_langLabel->setText("C/C++");
      break;
    default:
      m_langLabel->setText(editor->filePath().isEmpty()
                               ? "Plain Text"
                               : QFileInfo(editor->filePath()).suffix());
      break;
    }
  }
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

  m_outlineSymbolsUri = editor->fileUri();
  m_outlineSymbolsVersion = editor->documentVersion();
  if (!m_outlinePanel->isVisible())
    return;
  if (m_outlineRequestedUri.isEmpty() ||
      m_outlineRequestedUri != m_outlineSymbolsUri) {
    m_outlinePanel->clear();
    m_outlineRequestedUri = m_outlineSymbolsUri;
    m_outlineRequestedVersion = m_outlineSymbolsVersion;
    m_outlineSymbolsTimer->start();
  } else if (m_outlineRequestedVersion != m_outlineSymbolsVersion) {
    m_outlineRequestedVersion = m_outlineSymbolsVersion;
    m_outlineSymbolsTimer->start();
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

  const QString text = tm.semanticColor(ThemeManager::SemanticRole::Text).name();
  const QString muted = tm.semanticColor(ThemeManager::SemanticRole::TextMuted).name();
  const QString faint = tm.semanticColor(ThemeManager::SemanticRole::TextFaint).name();
  const QString accent = tm.semanticColor(ThemeManager::SemanticRole::Accent).name();
  const QString success = tm.semanticColor(ThemeManager::SemanticRole::Success).name();

  if (m_contextLabel) {
    m_contextLabel->setStyleSheet(
        QString("color: %1; padding: 0 4px;").arg(accent));
  }
  if (m_errorLabel) {
    m_errorLabel->setStyleSheet(
        QString("color: %1; padding: 0 4px;").arg(muted));
  }
  if (m_posLabel) {
    m_posLabel->setStyleSheet(
        QString("color: %1; padding: 0 4px;").arg(muted));
  }
  if (m_indentLabel) {
    m_indentLabel->setStyleSheet(
        QString("color: %1; padding: 0 4px;").arg(faint));
  }
  if (m_encodingLabel) {
    m_encodingLabel->setStyleSheet(
        QString("color: %1; padding: 0 4px;").arg(faint));
  }
  if (m_langLabel) {
    m_langLabel->setStyleSheet(
        QString("color: %1; padding: 0 4px; font-weight: bold;").arg(accent));
  }
  if (m_vimLabel)
    m_vimLabel->setStyleSheet(QString("color: %1; padding: 0 4px;").arg(text));

  if (m_lspLabel) {
    const QString lspText = m_lspLabel->text();
    if (lspText.contains("!"))
      setLspStatus(lspText,
                   tm.semanticColor(ThemeManager::SemanticRole::Error).name());
    else if (lspText.contains("○") || lspText.contains("◐"))
      setLspStatus(lspText,
                   tm.semanticColor(ThemeManager::SemanticRole::Warning).name());
    else if (lspText.contains("Disabled"))
      setLspStatus(lspText,
                   tm.semanticColor(ThemeManager::SemanticRole::TextFaint).name());
    else
      setLspStatus(lspText,
                   tm.semanticColor(ThemeManager::SemanticRole::Success).name());
  }

  setStyleSheetIfChanged(
      m_tabWidget,
      QString("QTabWidget::pane { border: none; background: %1; }"
              "QTabBar { background: %1; padding-top: 4px; }"
              "QTabBar::tab { background: %1; color: %2; padding: 6px 14px;"
              "  border-right: 1px solid %3; border-radius: 6px 6px 0 0;"
              "  font-size: %7; margin-left: 2px; }"
              "QTabBar::tab:selected { color: %4; background: %5;"
              "  border-top: 2px solid %8; }"
              "QTabBar::tab:hover:!selected { background: %6; }")
          .arg(tm.semanticColor(ThemeManager::SemanticRole::SurfaceMuted).name(),
               tm.semanticColor(ThemeManager::SemanticRole::TextMuted).name(),
               tm.semanticColor(ThemeManager::SemanticRole::BorderStrong).name(),
               tm.semanticColor(ThemeManager::SemanticRole::SelectedText).name(),
               tm.semanticColor(ThemeManager::SemanticRole::SurfaceAlt).name(),
               tm.semanticColor(ThemeManager::SemanticRole::Hover).name(),
               QString::number(qMax(appearance.uiFont().pointSize() - 2, appearance.minFontSize())),
               tm.semanticColor(ThemeManager::SemanticRole::Accent).name()));

  setStyleSheetIfChanged(
      m_splitter,
      QString("QSplitter::handle { background: transparent; width: 3px; "
              "margin: 0 0 0 -1px; }"
              "QSplitter::handle:hover { background: %1; }")
          .arg(tm.semanticColor(ThemeManager::SemanticRole::AccentHover).name()));

  const QString breadcrumbBg =
      tm.semanticColor(ThemeManager::SemanticRole::SurfaceAlt).name();
  const QString breadcrumbFg =
      tm.semanticColor(ThemeManager::SemanticRole::TextMuted).name();
  setStyleSheetIfChanged(
      m_breadcrumbs,
      QString("BreadcrumbsBar { background: %1; color: %2;"
              " border-radius: 6px 6px 0 0; padding: 2px 0; }")
          .arg(breadcrumbBg, breadcrumbFg));

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
  m_runAct->setText(tr.translate("menu.run"));
  m_stopAct->setText(tr.translate("menu.stop"));
  m_restartLspAct->setText(tr.translate("menu.restart_lsp"));

  m_viewMenu->setTitle(tr.translate("menu.view"));
  m_preferencesAct->setText(tr.translate("menu.preferences"));
  m_vimHelpAct->setText(tr.translate("menu.vim_motions"));
  m_shortcutsAct->setText(tr.translate("menu.shortcuts"));
  m_lspManagerAct->setText(tr.translate("menu.lsp_manager"));
  m_outlineToggleAct->setText(tr.translate("menu.structure"));
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

bool MainWindow::cLspEnabled() const {
  return TomlSettingsStore::instance().cLspEnabled();
}

QString MainWindow::resolvedCLspPath() const {
  const QString configured = TomlSettingsStore::instance().cLspPath().trimmed();
  if (!configured.isEmpty())
    return configured;

  const QString path =
      QProcessEnvironment::systemEnvironment().value("PATH");
  const QStringList dirs = path.split(QLatin1Char(':'), Qt::SkipEmptyParts);
  for (const QString &dir : dirs) {
    const QFileInfo candidate(QDir(dir), QStringLiteral("clangd"));
    if (candidate.isFile() && candidate.isExecutable())
      return candidate.absoluteFilePath();
  }
  return QString();
}

MainWindow::EditorLanguage
MainWindow::languageForPath(const QString &path) const {
  const QString suffix = QFileInfo(path).suffix().toLower();
  if (suffix == QLatin1String("zith"))
    return EditorLanguage::Zith;
  static const char *cFamilySuffixes[] = {
      "c",  "h",  "cc", "cpp", "cxx", "hh", "hpp", "hxx"};
  for (const char *candidate : cFamilySuffixes)
    if (suffix == QLatin1String(candidate))
      return EditorLanguage::CFamily;
  return EditorLanguage::PlainText;
}

QString MainWindow::lspLanguageId(MainWindow::EditorLanguage language) const {
  switch (language) {
  case EditorLanguage::Zith:
    return QStringLiteral("zith");
  case EditorLanguage::CFamily:
    return QStringLiteral("cpp");
  default:
    return QString();
  }
}

LspClient *MainWindow::lspClientForLanguage(
    MainWindow::EditorLanguage language) const {
  switch (language) {
  case EditorLanguage::Zith:
    return m_zithLspClient;
  case EditorLanguage::CFamily:
    return m_clangdClient;
  default:
    return nullptr;
  }
}

LspClient *MainWindow::lspClientForPath(const QString &path) const {
  return lspClientForLanguage(languageForPath(path));
}

bool MainWindow::shouldUseLspForPath(const QString &path) const {
  const EditorLanguage language = languageForPath(path);
  if (language == EditorLanguage::Zith)
    return lspEnabled();
  if (language == EditorLanguage::CFamily)
    return lspEnabled() && cLspEnabled() && !resolvedCLspPath().isEmpty();
  return false;
}

bool MainWindow::isZithEditor(CodeEditor *editor) const {
  return editor && languageForPath(editor->filePath()) == EditorLanguage::Zith;
}

bool MainWindow::isCFamilyEditor(CodeEditor *editor) const {
  return editor &&
         languageForPath(editor->filePath()) == EditorLanguage::CFamily;
}

LspClient *MainWindow::lspClientForEditor(CodeEditor *editor) const {
  return editor ? editor->lspClient() : nullptr;
}

void MainWindow::syncEditorWithLsp(CodeEditor *editor, bool openDocument) {
  if (!editor || editor->filePath().isEmpty())
    return;

  const EditorLanguage language = languageForPath(editor->filePath());
  LspClient *target = lspClientForLanguage(language);
  LspClient *current = editor->lspClient();

  if (current && current != target) {
    if (current->isReady() && !editor->fileUri().isEmpty())
      current->closeDocument(editor->fileUri());
  }
  if (target != current) {
    editor->setLspClient(target);
  }

  if (!shouldUseLspForPath(editor->filePath()) || !target ||
      !target->isReady())
    return;

  if (openDocument) {
    target->openDocument(editor->fileUri(),
                         lspLanguageId(language).toUtf8().constData(),
                         editor->toPlainText(), editor->documentVersion());
  } else {
    editor->flushPendingLspChanges();
    target->saveDocument(editor->fileUri());
  }
}

void MainWindow::closeEditorWithLsp(CodeEditor *editor) {
  if (!editor || editor->fileUri().isEmpty())
    return;

  LspClient *current = editor->lspClient();
  if (current && current->isReady())
    current->closeDocument(editor->fileUri());
  editor->detachLspClient();
}

void MainWindow::updateClangdLifecycle() {
  bool hasCFamily = false;
  for (int i = 0; i < m_tabWidget->count(); ++i) {
    auto *editor = qobject_cast<CodeEditor *>(m_tabWidget->widget(i));
    if (isCFamilyEditor(editor))
      hasCFamily = true;
  }

  const QString path = resolvedCLspPath();
  if (!lspEnabled() || !hasCFamily || !cLspEnabled()) {
    if (m_clangdClient && m_clangdClient->isRunning())
      m_clangdClient->stop();
    if (m_clangdClient)
      m_activeCLspPath.clear();
    m_clangdError.clear();
    updateClangdRuntimeInfo();
    return;
  }

  if (path.isEmpty()) {
    if (m_clangdClient && m_clangdClient->isRunning())
      m_clangdClient->stop();
    m_activeCLspPath.clear();
    m_clangdError = tr("clangd was not found in PATH.");
    updateClangdRuntimeInfo();
    return;
  }

  updateClangdRuntimeInfo();
  if (m_clangdClient->isRunning() && m_activeCLspPath == path)
    return;
  if (m_clangdClient->isRunning())
    m_clangdClient->stop();
  m_clangdError.clear();
  const QString workspace =
      m_contextManager ? m_contextManager->currentRoot() : QString();
  if (m_clangdClient->start(path, QString(), workspace,
                            QStringLiteral("clangd")))
    m_activeCLspPath = path;
  else
    m_activeCLspPath.clear();
  updateClangdRuntimeInfo();
}

void MainWindow::updateClangdRuntimeInfo() {
  if (!m_settingsPanel)
    return;
  const QString path = resolvedCLspPath();
  QPair<QString, QString> info;
  if (!lspEnabled() || !cLspEnabled())
    info = {tr("Disabled"), QString()};
  else if (m_clangdClient && m_clangdClient->isReady())
    info = {tr("Connected"),
            m_activeCLspPath.isEmpty()
                ? QString()
                : QString("clangd is active. Results improve with compile_commands.json.")};
  else if (path.isEmpty())
    info = {
        tr("Not found"),
        tr("clangd was not found in PATH. Install clangd or set its path in the LSP Manager.")};
  else if (m_clangdClient && m_clangdClient->isRunning())
    info = {
        tr("Starting"),
        tr("clangd is starting; it may need compile_commands.json for full precision.")};
  else
    info = {m_clangdError.isEmpty() ? tr("Stopped") : tr("Error"),
            m_clangdError.isEmpty()
                ? tr("clangd is configured but not running with an open C-family file.")
                : m_clangdError};
  m_settingsPanel->setCLspInfo(info.first, path, info.second);
  if (m_lspManagerDialog)
    m_lspManagerDialog->setCLspInfo(info.first, path, info.second);
}

void MainWindow::setLspEnabled(bool enabled) {
  TomlSettingsStore::instance().setLspEnabled(enabled);
  if (m_settingsPanel)
    m_settingsPanel->setLspEnabled(enabled);
  if (m_lspManagerDialog)
    m_lspManagerDialog->setLspEnabled(enabled);

  if (!enabled) {
    if (m_zithToolchainManager)
      m_zithToolchainManager->cancel();
    if (m_zithLspClient)
      m_zithLspClient->stop();
    if (m_clangdClient)
      m_clangdClient->stop();
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
    setLspStatus("LSP Disabled",
                 ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::TextFaint).name());
    updateSettingsRuntimeInfo();
    updateLspDiagnostics();
    updateClangdRuntimeInfo();
  } else {
    m_lastLspError.clear();
    ensureLspRuntime(true);
    updateClangdLifecycle();
  }

  if (m_restartLspAct)
    m_restartLspAct->setEnabled(enabled);
  updateRunActionsEnabled();
}

void MainWindow::ensureLspRuntime(bool preferCached) {
  if (!m_zithToolchainManager || !lspEnabled())
    return;

  m_runtimeStatusText = preferCached ? "Resolving latest Zith runtime..."
                                     : "Refreshing Zith runtime...";
  setLspStatus("LSP ○",
               ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Warning).name());
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
  if (m_zithLspClient->isRunning() && m_activeLspPath == lspPath &&
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
  if (m_zithLspClient->isRunning())
    m_zithLspClient->stop();

  m_completionModel->setItems({});
  m_runtimeTag = tag;
  m_activeLspPath = lspPath;
  m_activeStdlibPath = stdlibPath;
  m_activeWorkspaceRoot = currentWorkspace;
  m_runtimeStatusText = QString("Starting runtime %1...").arg(tag);

  setLspStatus("LSP ○",
               ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Warning).name());
  updateSettingsRuntimeInfo();
  if (m_zithLspClient->start(lspPath, stdlibPath, currentWorkspace)) {
    statusBar()->showMessage(QString("Starting Zith runtime %1...").arg(tag),
                             3000);
  } else {
    setLspStatus("LSP !",
                 ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Error).name());
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
  if (m_zithLspClient) {
    if (m_zithLspClient->isReady())
      connection = "Connected";
    else if (m_zithLspClient->isRunning())
      connection = "Starting (waiting for initialize)";
    else
      connection = "Stopped";
  }

  QString syncMode = "Unknown";
  if (m_zithLspClient && m_zithLspClient->isReady()) {
    switch (m_zithLspClient->documentSyncKind()) {
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
  if (m_zithLspClient->isRunning())
    m_zithLspClient->stop();

  QString errorMessage;
  if (!m_zithToolchainManager->clearCachedRuntime(&errorMessage)) {
    m_runtimeStatusText = errorMessage;
    setLspStatus("LSP !",
                 ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Error).name());
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
  setLspStatus("LSP ○",
               ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Warning).name());
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

  closeEditorWithLsp(editor);
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
  updateClangdLifecycle();
}

void MainWindow::setLspStatus(const QString &text, const QString &color) {
  if (!m_lspLabel)
    return;

  m_lspLabel->setText(text);
  const QString resolved =
      color.isEmpty() ? m_lspLabelColor
                      : color;
  if (!color.isEmpty())
    m_lspLabelColor = color;
  const QString finalColor =
      resolved.isEmpty() ? ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Success).name()
                         : resolved;
  m_lspLabel->setStyleSheet(
      QString("color: %1; padding: 0 4px;").arg(finalColor));
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
    if (auto *client = lspClientForPath(editor->filePath());
        client && client->isReady())
      client->saveDocument(editor->fileUri());
  }
}

bool MainWindow::canExecuteLspWorkspaceCommand() const {
  return lspEnabled() && m_zithLspClient && m_zithLspClient->isReady() &&
         m_zithLspClient->hasExecuteCommandProvider();
}

void MainWindow::updateRunActionsEnabled() {
  if (!m_buildAct || !m_checkAct || !m_runAct || !m_stopAct)
    return;
  const bool canExecute = canExecuteLspWorkspaceCommand();
  const bool activeZith =
      isZithEditor(currentEditor());
  m_buildAct->setEnabled(canExecute && activeZith);
  m_checkAct->setEnabled(canExecute && activeZith && currentEditor() &&
                         !currentEditor()->filePath().isEmpty());
  m_runAct->setEnabled(canExecute && activeZith &&
                       !m_compilerPanel->isRunning());
  m_stopAct->setEnabled(canExecute && activeZith &&
                        m_compilerPanel->isRunning());
  // State-specific tooltips
  auto stateTooltip = [&](const QString &base) -> QString {
    if (!canExecute)
      return QStringLiteral("Requires a zith-lsp server with workspace/executeCommand support");
    if (!activeZith)
      return QStringLiteral("Not a Zith file");
    return base;
  };

  m_buildAct->setToolTip(stateTooltip("Build project (Ctrl+B)"));
  m_checkAct->setToolTip(
      canExecute && activeZith && currentEditor() && !currentEditor()->filePath().isEmpty()
          ? QStringLiteral("Check current file (Ctrl+Shift+C)")
          : stateTooltip("Check current file (Ctrl+Shift+C)"));
  m_runAct->setToolTip(
      canExecute && activeZith && m_compilerPanel->isRunning()
          ? QStringLiteral("A task is already running")
          : stateTooltip("Run project (Ctrl+Shift+R)"));
  m_stopAct->setToolTip(stateTooltip("Stop running task (Ctrl+Shift+Q)"));

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

void MainWindow::onWorkDoneProgressReceived(const QString &token,
                                            const QString &kind,
                                            const QString &message) {
  if (!m_compilerPanel)
    return;
  if (!m_activeProgressToken.isEmpty() && token != m_activeProgressToken)
    return;
  if (kind == QLatin1String("begin")) {
    if (m_activeProgressToken.isEmpty()) {
      m_activeProgressToken = token;
      m_compilerPanel->setActiveProgressToken(token);
      m_compilerPanel->appendOutput(QStringLiteral("Compiling..."));
    }
  } else if (kind == QLatin1String("report")) {
    if (!m_activeProgressToken.isEmpty() &&
        (message.isEmpty() || message != QLatin1String("Compiling")))
      m_compilerPanel->appendWorkDoneProgress(token, kind, message);
  } else if (kind == QLatin1String("end")) {
    if (!m_activeProgressToken.isEmpty() && token == m_activeProgressToken) {
      m_compilerPanel->appendWorkDoneProgress(token, kind,
                                               message.isEmpty()
                                                   ? QStringLiteral("Finished")
                                                   : message);
      m_activeProgressToken.clear();
      m_compilerPanel->clearActiveProgress();
    }
  }
}

void MainWindow::onLspCommandResult(const QString &command, bool success,
                                    const QJsonValue &result) {
  if (!m_compilerPanel)
    return;

  const QJsonObject resultObject = result.toObject();
  const bool serverReportedSuccess =
      resultObject.value(QStringLiteral("success")).toBool(true);
  const bool effectiveSuccess = success && serverReportedSuccess;
  const QString programUri =
      resultObject.value(QStringLiteral("programUri")).toString();
  const QString serverMessage =
      resultObject.value(QStringLiteral("message")).toString();
  const bool hasCodegenAvailable =
      resultObject.contains(QStringLiteral("codegenAvailable"));
  const bool codegenAvailable =
      resultObject.value(QStringLiteral("codegenAvailable")).toBool(false);
  const QString failurePrefix =
      command == QLatin1String("zith.build")
          ? QStringLiteral("Build failed: ")
          : command == QLatin1String("zith.check")
                ? QStringLiteral("Check failed: ")
                : command == QLatin1String("zith.run")
                      ? QStringLiteral("Run failed: ")
                      : QString();

  const auto appendServerFailure = [&]() {
    if (result.isString()) {
      m_compilerPanel->appendOutput(failurePrefix + result.toString());
      return;
    }
    if (resultObject.isEmpty())
      return;
    m_compilerPanel->appendOutput(QStringLiteral("Server reported success=false"));
    if (!programUri.isEmpty())
      m_compilerPanel->appendOutput(
          QStringLiteral("Program URI: %1").arg(programUri));
    if (hasCodegenAvailable)
      m_compilerPanel->appendOutput(
          QStringLiteral("codegenAvailable: %1")
              .arg(codegenAvailable ? QStringLiteral("true")
                                    : QStringLiteral("false")));
    if (!serverMessage.isEmpty())
      m_compilerPanel->appendOutput(serverMessage);
  };

  if (command == QLatin1String("zith.build")) {
    m_compilerPanel->showBuildResult(effectiveSuccess, programUri);
    if (!effectiveSuccess) {
      appendServerFailure();
      appendPublishedDiagnostics();
    }
    updateRunActionsEnabled();
  } else if (command == QLatin1String("zith.check")) {
    m_compilerPanel->showBuildResult(effectiveSuccess);
    if (!effectiveSuccess) {
      appendServerFailure();
      appendPublishedDiagnostics();
    }
    if (auto *ed = currentEditor())
      ed->viewport()->update();
  } else if (command == QLatin1String("zith.run")) {
    if (!effectiveSuccess) {
      m_compilerPanel->showBuildResult(false);
      appendServerFailure();
      m_compilerPanel->setRunningTask({});
      updateRunActionsEnabled();
      return;
    }
    const QString taskId =
        resultObject.value(QStringLiteral("taskId")).toString();
    if (taskId.isEmpty()) {
      m_compilerPanel->appendOutput(
          "Server started run without a taskId; cannot track or stop it.");
      m_compilerPanel->showBuildResult(false);
      m_compilerPanel->setRunningTask({});
      updateRunActionsEnabled();
      return;
    }
    m_compilerPanel->setRunningTask(taskId);
    for (const QString &chunk : m_runOutput.takeFor(taskId))
      m_compilerPanel->appendRawOutput(chunk.toUtf8());
    if (const auto exitCode = m_runOutput.takeExitFor(taskId)) {
      m_compilerPanel->appendRawOutput(
          (*exitCode == 0 ? "Process exited with code 0"
                          : QString("Process exited with code %1")
                                .arg(*exitCode))
              .toUtf8());
      m_compilerPanel->setRunningTask({});
      updateRunActionsEnabled();
      return;
    }
    m_compilerPanel->appendOutput(QString("Run started (task %1)").arg(taskId));
    m_compilerPanel->appendOutput(QString("Program URI: %1")
                                      .arg(result.toObject()
                                               .value("programUri")
                                               .toString()));
    updateRunActionsEnabled();
  }
}

void MainWindow::runBuild() {
  if (!canExecuteLspWorkspaceCommand()) {
    const QString message =
        "zith-lsp must advertise workspace/executeCommand to run Build.";
    if (m_compilerPanel) {
      m_bottomPanel->showCompiler();
      m_compilerPanel->startBuild("Build unavailable");
      m_compilerPanel->appendOutput(message);
    }
    statusBar()->showMessage(message, 6000);
    return;
  }
  const QString root =
      m_contextManager ? m_contextManager->currentRoot() : QString();
  if (root.isEmpty()) {
    statusBar()->showMessage("No active project root for Build", 5000);
    return;
  }
  saveAllForLsp();
  m_bottomPanel->showCompiler();
  m_activeProgressToken.clear();
  m_compilerPanel->startBuild(QString("Build project %1").arg(root));
  m_zithLspClient->executeWorkspaceCommand(
      QStringLiteral("zith.build"),
      QJsonArray{QUrl::fromLocalFile(root).toString()}, {});
}

void MainWindow::runCheckFile() {
  auto *ed = currentEditor();
  if (!ed || ed->filePath().isEmpty()) {
    statusBar()->showMessage("No open file to check", 4000);
    return;
  }
  if (!canExecuteLspWorkspaceCommand()) {
    const QString message =
        "zith-lsp must advertise workspace/executeCommand to run Check File.";
    if (m_compilerPanel) {
      m_bottomPanel->showCompiler();
      m_compilerPanel->startBuild("Check unavailable");
      m_compilerPanel->appendOutput(message);
    }
    statusBar()->showMessage(message, 6000);
    return;
  }
  ed->flushPendingLspChanges();
  m_bottomPanel->showCompiler();
  m_activeProgressToken.clear();
  m_compilerPanel->startBuild(QString("Check %1").arg(ed->filePath()));
  m_zithLspClient->executeWorkspaceCommand(
      QStringLiteral("zith.check"),
      QJsonArray{ed->fileUri()}, {});
}

void MainWindow::runProject() {
  if (!canExecuteLspWorkspaceCommand()) {
    const QString message =
        "zith-lsp must advertise workspace/executeCommand to run the project.";
    m_bottomPanel->showCompiler();
    m_compilerPanel->startBuild("Run unavailable");
    m_compilerPanel->appendOutput(message);
    statusBar()->showMessage(message, 6000);
    return;
  }
  const QString root =
      m_contextManager ? m_contextManager->currentRoot() : QString();
  if (root.isEmpty()) {
    statusBar()->showMessage("No active project root for Run", 5000);
    return;
  }
  saveAllForLsp();
  m_bottomPanel->showCompiler();
  m_activeProgressToken.clear();
  m_compilerPanel->startBuild(QString("Run project %1").arg(root));
  m_zithLspClient->executeWorkspaceCommand(
      QStringLiteral("zith.run"),
      QJsonArray{QUrl::fromLocalFile(root).toString()}, {});
}

void MainWindow::stopRunningTask() {
  const QString taskId =
      m_compilerPanel ? m_compilerPanel->runningTaskId() : QString();
  if (taskId.isEmpty() || !canExecuteLspWorkspaceCommand())
    return;
  m_zithLspClient->executeWorkspaceCommand(
      QStringLiteral("zith.stop"), QJsonArray{taskId}, {});
  m_compilerPanel->appendOutput(QString("Stopping task %1").arg(taskId));
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

void MainWindow::onReplaceAllPreviewReady(
    const QString &needle,
    const QString &replacement,
    const QVector<SearchReplaceTarget> &targets) {
  if (targets.isEmpty()) {
    statusBar()->showMessage(
        QString("No matches for \"%1\".").arg(needle), 5000);
    return;
  }

  int totalMatches = 0;
  for (const SearchReplaceTarget &target : targets)
    totalMatches += target.matches;

  const auto answer = QMessageBox::question(
      this,
      "Replace in workspace",
      QString("Replace %1 matches in %2 files?\n"
              "\"%3\" -> \"%4\"")
          .arg(totalMatches)
          .arg(targets.size())
          .arg(needle, replacement),
      QMessageBox::Yes | QMessageBox::Cancel,
      QMessageBox::Cancel);
  if (answer != QMessageBox::Yes)
    return;

  applyWorkspaceReplace(needle, replacement, targets);
}

void MainWindow::applyWorkspaceReplace(
    const QString &needle,
    const QString &replacement,
    const QVector<SearchReplaceTarget> &targets) {
  int replaced = 0;
  for (const SearchReplaceTarget &target : targets) {
    CodeEditor *openEditor = nullptr;
    for (int i = 0; i < m_tabWidget->count(); ++i) {
      auto *editor = qobject_cast<CodeEditor *>(m_tabWidget->widget(i));
      if (editor && editor->filePath() == target.path) {
        openEditor = editor;
        break;
      }
    }

    if (openEditor) {
      const QList<QPair<LspRange, QString>> edits =
          SearchPanel::replaceEdits(openEditor->toPlainText(), needle, replacement);
      if (edits.isEmpty())
        continue;
      openEditor->applyEdits(edits);
      openEditor->document()->setModified(true);
      openEditor->flushPendingLspChanges();
    } else {
      QFile file(target.path);
      if (!file.open(QIODevice::ReadOnly))
        continue;
      const QString text = QString::fromUtf8(file.readAll());
      file.close();

      const QList<QPair<LspRange, QString>> edits =
          SearchPanel::replaceEdits(text, needle, replacement);
      if (edits.isEmpty())
        continue;

      QSaveFile out(target.path);
      if (!out.open(QIODevice::WriteOnly)) {
        statusBar()->showMessage(
            "Could not write " + target.path, 5000);
        continue;
      }
      out.write(SearchPanel::applyReplaceEdits(text, edits).toUtf8());
      if (!out.commit()) {
        statusBar()->showMessage(
            "Could not save " + target.path, 5000);
        continue;
      }
    }
    replaced += target.matches;
  }

  statusBar()->showMessage(
      QString("Replaced %1 matches in workspace.").arg(replaced), 8000);
  m_searchPanel->setRootPath(m_searchPanel->rootPath());
}

void MainWindow::onFrontendStatusReceived(const QJsonObject &status) {
    QString state = status.value("state").toString();
    QString msg = status.value("message").toString();
    if (state == "warming") {
        m_runtimeStatusText = "Frontend warming up...";
        setLspStatus("LSP ◐",
                     ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Info).name());
    } else if (state == "ready") {
        m_runtimeStatusText = "Frontend ready";
        setLspStatus("LSP ⬤",
                     ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Success).name());
    } else if (state == "error") {
        m_runtimeStatusText = "Frontend error: " + msg;
        setLspStatus("LSP !",
                     ThemeManager::instance().semanticColor(ThemeManager::SemanticRole::Error).name());
    }
    if (m_settingsPanel) m_settingsPanel->appendLspLog("Frontend status: " + state + (msg.isEmpty() ? "" : " - " + msg));
    if (m_lspManagerDialog) m_lspManagerDialog->appendLspLog("Frontend status: " + state + (msg.isEmpty() ? "" : " - " + msg));
}

void MainWindow::onMetricsReceived(const QJsonObject &metrics) {
    if (m_settingsPanel) m_settingsPanel->appendLspLog("Metrics: " + QString::fromUtf8(QJsonDocument(metrics).toJson(QJsonDocument::Compact)));
    if (m_lspManagerDialog) m_lspManagerDialog->appendLspLog("Metrics: " + QString::fromUtf8(QJsonDocument(metrics).toJson(QJsonDocument::Compact)));
}
