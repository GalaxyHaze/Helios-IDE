#include <QtTest>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QUrl>
#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QPushButton>
#include <QLabel>
#include <QSyntaxHighlighter>
#include <QModelIndex>
#include <QTreeWidget>
#include <QTabWidget>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QShortcut>
#include <QToolButton>
#include <QSysInfo>

#include "../editor/core/TomlSettingsStore.h"
#include "../editor/core/TomlSettingsCodec.h"
#include "../editor/core/AppearanceController.h"
#include "../editor/core/ZithToolchainManager.h"
#include "../editor/core/ThemeManager.h"
#include "../editor/core/ThemeDefinitionParser.h"
#include "../editor/core/TranslationManager.h"
#include "../editor/core/RunOutputCollector.h"
#include "../editor/core/WorkspaceCommandController.h"
#include "../editor/core/WorkspaceCommandResultDecoder.h"
#include "../editor/core/WorkspaceCommandAvailabilityController.h"
#include "../editor/core/WorkspacePanelPresentationController.h"
#include "../editor/core/WorkspaceEdit.h"
#include "../editor/core/WorkspaceEditApplier.h"
#include "../editor/core/WindowLayoutController.h"
#include "../editor/core/WindowLayoutPersistence.h"
#include "../editor/core/LanguageIdentity.h"
#include "../editor/core/LanguageServiceFeedbackController.h"
#include "../editor/core/LspClientEventSource.h"
#include "../editor/core/LspEventSource.h"
#include "../editor/core/LspSettingsPersistence.h"
#include "../editor/core/ClangdLifecycleCoordinator.h"
#include "../editor/core/ClangdExecutableResolver.h"
#include "../editor/core/ContextManager.h"
#include "../editor/core/ContextNavigationController.h"
#include "../editor/core/ContextWorkspaceController.h"
#include "../editor/core/WorkspaceRootController.h"
#include "../editor/core/WorkspaceRootInteractionController.h"
#include "../editor/core/LanguageServiceWorkspaceController.h"
#include "../editor/core/SidebarController.h"
#include "../editor/core/EditorSyntaxController.h"
#include "../editor/core/EditorInteractionController.h"
#include "../editor/core/EditorTabCloseController.h"
#include "../editor/core/EditorSessionController.h"
#include "../editor/core/EditorFileController.h"
#include "../editor/core/EditorLspActionController.h"
#include "../editor/core/LspDocumentCoordinator.h"
#include "../editor/core/LspEditorLifecycleController.h"
#include "../editor/core/LspEditorResultRouter.h"
#include "../editor/core/LspReferencesRouter.h"
#include "../editor/core/LspCodeActionRouter.h"
#include "../editor/core/LspRuntimePresentationController.h"
#include "../editor/core/LspRuntimePresentationModel.h"
#include "../editor/core/LspLogPresenter.h"
#include "../editor/core/LspRuntimeController.h"
#include "../editor/core/LspRuntimeEventController.h"
#include "../editor/core/LocationNavigator.h"
#include "../editor/core/EditorChromeController.h"
#include "../editor/core/EditorWorkspacePresentationController.h"
#include "../editor/core/StatusBarController.h"
#include "../editor/core/ApplicationStyle.h"
#include "../editor/core/ApplicationThemeController.h"
#include "../editor/core/MainWindow.h"
#include "../editor/core/ShellCommandSurface.h"
#include "../editor/core/ShellDialogController.h"
#include "../editor/core/ShellTranslationController.h"
#include "../editor/core/LspCompletionRouter.h"
#include "../editor/core/LspRestartPolicy.h"
#include "../editor/core/WorkspaceSearch.h"
#include "../editor/core/WorkspaceSearchController.h"
#include "../editor/core/WorkspaceReplaceController.h"
#include "../editor/core/WorkspaceNavigationController.h"
#include "../editor/core/WorkspaceTaskOutputController.h"
#include "../editor/core/ZithRuntimeCatalog.h"
#include "../editor/core/ZithRuntimeAssetDownloader.h"
#include "../editor/core/ZithReleaseCatalog.h"
#include "../editor/core/ZithRuntimeInstaller.h"
#include "../editor/core/ZithRuntimeOverrideResolver.h"
#include "../editor/core/ShortcutCatalog.h"
#include "../editor/core/ShortcutTreePresenter.h"
#include "../editor/core/ZithRuntimeState.h"
#include "../editor/core/ZithRuntimeLifecycleCoordinator.h"
#include "../editor/panels/GitStatusListPresenter.h"
#include "../editor/panels/GitPanelPresentationModel.h"
#include "../editor/editor/LspClient.h"
#include "../editor/editor/EditorDiagnosticHighlighter.h"
#include "../editor/editor/EditorTextEditApplier.h"
#include "../editor/editor/EditorCompletionInsertionPolicy.h"
#include "../editor/editor/EditorLanguageFeatureController.h"
#include "../editor/editor/LspProcessTransport.h"
#include "../editor/editor/LspProtocolCodec.h"
#include "../editor/editor/LspRequestTracker.h"
#include "../editor/editor/LspRequestSender.h"
#include "../editor/editor/LspServerMessageDispatcher.h"
#include "../editor/editor/LspInitializationBuilder.h"
#include "../editor/editor/LspDocumentSync.h"
#include "../editor/editor/LspDocumentProtocol.h"
#include "../editor/editor/LspFeatureRequestRouter.h"
#include "../editor/editor/LspShutdownEscalation.h"
#include "../editor/editor/LspSessionLifecycle.h"
#include "../editor/editor/LspDocumentRegistry.h"
#include "../editor/editor/LspResultDecoder.h"
#include "../editor/panels/CompilerPanel.h"
#include "../editor/panels/BottomPanel.h"
#include "../editor/panels/DiagnosticsPanel.h"
#include "../editor/panels/ReferencesPanel.h"
#include "../editor/panels/SettingsPanel.h"
#include "../editor/panels/LspManagerDialog.h"
#include "../editor/panels/PreferencesDialog.h"
#include "../editor/panels/ShortcutsDialog.h"
#include "../editor/panels/VimHelpDialog.h"
#include "../editor/editor/Syntax.h"
#include "../editor/editor/BracketMatcher.h"
#include "../editor/editor/EditorTypingPolicy.h"
#include "../editor/editor/EditorDeletionPolicy.h"
#include "../editor/editor/EditorDocumentSyncController.h"
#include "../editor/editor/VimCharacterSearch.h"
#include "../editor/editor/VimCommandSession.h"
#include "../editor/editor/VimMotionController.h"
#include "../editor/editor/VimMotionResolver.h"
#include "../editor/editor/VimPendingOperationSession.h"
#include "../editor/editor/VimSearchSession.h"
#include "../editor/editor/CHighlighter.h"
#include "../editor/panels/SearchPanel.h"
#include "../editor/panels/FileTreePanel.h"
#include "../editor/panels/GitPanel.h"
#include "../editor/core/FileIcons.h"
#include "../editor/core/GitCommandRunner.h"
#include "../editor/core/GitStatusParser.h"
#include "../editor/core/GitRepositorySession.h"
#include "../editor/widgets/ProjectTreeModel.h"
#include "../editor/widgets/BreadcrumbsBar.h"
#include "../editor/widgets/ActivityBar.h"
#include "../editor/editor/Code.h"
#include "../editor/editor/EditorAppearanceController.h"
#include "../editor/editor/EditorContextMenuController.h"
#include "../editor/widgets/FindReplaceBar.h"
#include "../editor/core/SnippetManager.h"
#include "../editor/editor/LspCompletionModel.h"
#include "../editor/panels/OutlinePanel.h"
#include "../editor/panels/WelcomeWidget.h"

class FakeLspSettingsPersistence final : public LspSettingsPersistence
{
public:
    bool enabled = false;
    bool online = false;
    bool cFamilyEnabled = false;
    QString clangdPath;
    int enabledWrites = 0;
    int onlineWrites = 0;
    int cFamilyWrites = 0;
    int pathWrites = 0;

    bool lspEnabled() const override { return enabled; }
    bool useOnlineZithLsp() const override { return online; }
    bool cLspEnabled() const override { return cFamilyEnabled; }
    QString cLspPath() const override { return clangdPath; }

    void setLspEnabled(bool value) override
    {
        enabled = value;
        ++enabledWrites;
    }
    void setUseOnlineZithLsp(bool value) override
    {
        online = value;
        ++onlineWrites;
    }
    void setCLspEnabled(bool value) override
    {
        cFamilyEnabled = value;
        ++cFamilyWrites;
    }
    void setCLspPath(const QString &value) override
    {
        clangdPath = value;
        ++pathWrites;
    }
};

class TestHelios : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        QCoreApplication::setApplicationName("HeliosTest");
    }

    void testTomlSettingsStore() {
        auto &store = TomlSettingsStore::instance();
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        store.setConfigDirForTesting(tempDir.path());
        store.setFontSize(15);
        store.setTheme("helios-dark");
        store.setCustomThemePath(QString());
        store.setLocale("pt-BR");
        store.setSidebarVisible(false);
        store.setTreeMaxDepth(20);
        store.setUiFontFamily("Sans Serif");
        store.setUiFontSize(14);
        store.setEditorFontFamily("Monospace");
        store.setEditorFontSize(15);
        store.setRenderingStrategy("no-antialias");
        store.setUiScale(125);
        store.setVimMotionsEnabled(true);
        store.setUseOnlineZithLsp(true);
        store.setSearchTextExtensions({"zith", "cpp", "dockerfile"});
        store.setSearchExcludedDirs({".git", "vendor"});

        QStringList projects = {"/path/to/a", "/path/to/b"};
        store.setRecentProjects(projects);
        store.save();

        store.load();
        QCOMPARE(store.fontSize(), 14);
        QCOMPARE(store.theme(), QString("helios-dark"));
        QCOMPARE(store.locale(), QString("pt-BR"));
        QCOMPARE(store.sidebarVisible(), false);
        QCOMPARE(store.treeMaxDepth(), 20);
        QCOMPARE(store.recentProjects().size(), 2);
        QCOMPARE(store.recentProjects().first(), QString("/path/to/a"));
        QCOMPARE(store.uiFontSize(), 14);
        QCOMPARE(store.editorFontSize(), 15);
        QCOMPARE(store.uiScale(), 125);
        QVERIFY(store.vimMotionsEnabled());
        QVERIFY(store.useOnlineZithLsp());
        QCOMPARE(store.searchTextExtensions(), QStringList({"zith", "cpp", "dockerfile"}));
        QCOMPARE(store.searchExcludedDirs(), QStringList({".git", "vendor"}));
        QCOMPARE(store.customThemePath(), QString());
    }

    void testTomlSettingsStorePublishesEditorPreferenceChanges() {
        auto &store = TomlSettingsStore::instance();
        const bool initialVim = store.vimMotionsEnabled();
        const bool initialWordWrap = store.wordWrap();
        QSignalSpy preferenceSpy(
            &store, &TomlSettingsStore::editorPreferencesChanged);

        store.setVimMotionsEnabled(!initialVim);
        QCOMPARE(preferenceSpy.count(), 1);
        store.setVimMotionsEnabled(!initialVim);
        QCOMPARE(preferenceSpy.count(), 1);

        store.setWordWrap(!initialWordWrap);
        QCOMPARE(preferenceSpy.count(), 2);
        store.setWordWrap(!initialWordWrap);
        QCOMPARE(preferenceSpy.count(), 2);

        store.setVimMotionsEnabled(initialVim);
        store.setWordWrap(initialWordWrap);
    }

    void testTomlSettingsCodecRoundTripsSnapshot() {
        TomlSettingsSnapshot original;
        original.theme = "custom";
        original.customThemePath = "/tmp/helios theme.json";
        original.locale = "pt-PT";
        original.uiFontFamily = "Noto Sans";
        original.uiFontSize = 15;
        original.editorFontFamily = "JetBrains Mono";
        original.editorFontSize = 16;
        original.renderingStrategy = "no-antialias";
        original.uiScale = 125;
        original.vimMotionsEnabled = true;
        original.wordWrap = true;
        original.searchTextExtensions = {"cpp", "h", "zith"};
        original.searchExcludedDirs = {".git", "node_modules"};
        original.sidebarWidth = 360;
        original.sidebarVisible = false;
        original.outlineVisible = true;
        original.treeMaxDepth = 24;
        original.onboardingDismissed = true;
        original.lspEnabled = true;
        original.useOnlineZithLsp = true;
        original.cLspEnabled = false;
        original.cLspPath = "/opt/clangd";
        original.mainWindowGeometryBase64 = "Z2VvbWV0cnk=";
        original.mainWindowStateBase64 = "c3RhdGU=";
        original.recentProjects = {"/tmp/one", "/tmp/two"};

        const TomlSettingsSnapshot decoded =
            TomlSettingsCodec::parse(TomlSettingsCodec::serialize(original));

        QCOMPARE(decoded.theme, original.theme);
        QCOMPARE(decoded.customThemePath, original.customThemePath);
        QCOMPARE(decoded.locale, original.locale);
        QCOMPARE(decoded.uiFontFamily, original.uiFontFamily);
        QCOMPARE(decoded.uiFontSize, original.uiFontSize);
        QCOMPARE(decoded.editorFontFamily, original.editorFontFamily);
        QCOMPARE(decoded.editorFontSize, original.editorFontSize);
        QCOMPARE(decoded.renderingStrategy, original.renderingStrategy);
        QCOMPARE(decoded.uiScale, original.uiScale);
        QCOMPARE(decoded.vimMotionsEnabled, original.vimMotionsEnabled);
        QCOMPARE(decoded.wordWrap, original.wordWrap);
        QCOMPARE(decoded.searchTextExtensions, original.searchTextExtensions);
        QCOMPARE(decoded.searchExcludedDirs, original.searchExcludedDirs);
        QCOMPARE(decoded.sidebarWidth, original.sidebarWidth);
        QCOMPARE(decoded.sidebarVisible, original.sidebarVisible);
        QCOMPARE(decoded.outlineVisible, original.outlineVisible);
        QCOMPARE(decoded.treeMaxDepth, original.treeMaxDepth);
        QCOMPARE(decoded.onboardingDismissed, original.onboardingDismissed);
        QCOMPARE(decoded.lspEnabled, original.lspEnabled);
        QCOMPARE(decoded.useOnlineZithLsp, original.useOnlineZithLsp);
        QCOMPARE(decoded.cLspEnabled, original.cLspEnabled);
        QCOMPARE(decoded.cLspPath, original.cLspPath);
        QCOMPARE(decoded.mainWindowGeometryBase64,
                 original.mainWindowGeometryBase64);
        QCOMPARE(decoded.mainWindowStateBase64, original.mainWindowStateBase64);
        QCOMPARE(decoded.recentProjects, original.recentProjects);
    }

    void testTomlSettingsCodecMigratesLegacyAppearanceSettings() {
        const QString legacy = R"(
[ui]
fontFamily = "Legacy UI"
fontSize = 17
)";

        const TomlSettingsSnapshot decoded = TomlSettingsCodec::parse(legacy);

        QCOMPARE(decoded.uiFontFamily, QString("Legacy UI"));
        QCOMPARE(decoded.uiFontSize, 17);
        QCOMPARE(decoded.editorFontFamily, QString("Legacy UI"));
        QCOMPARE(decoded.editorFontSize, 17);
    }

    void testTomlSettingsCodecNormalizesInvalidValues() {
        const QString invalid = R"(
[editor]
uiFontSize = 5
editorFontSize = 0
[ui]
uiScale = 201
sidebarWidth = 49
treeMaxDepth = 65
)";

        const TomlSettingsSnapshot decoded = TomlSettingsCodec::parse(invalid);

        QCOMPARE(decoded.uiFontSize, 13);
        QCOMPARE(decoded.editorFontSize, 13);
        QCOMPARE(decoded.uiScale, 100);
        QCOMPARE(decoded.sidebarWidth, 280);
        QCOMPARE(decoded.treeMaxDepth, 12);
    }

    void testThemeManagerFallback() {
        auto &tm = ThemeManager::instance();
        bool ok = tm.loadTheme("invalid-theme-name-xyz");
        QCOMPARE(ok, false);
        QCOMPARE(tm.currentThemeName(), QString("invalid-theme-name-xyz"));
        QColor bg = tm.palette().color(QPalette::Window);
        QVERIFY(bg.isValid());

        QVERIFY(!tm.loadTheme("invalid-dark-theme-name-xyz"));
        QCOMPARE(tm.palette().color(QPalette::Window), QColor("#11131a"));
        QCOMPARE(tm.palette().color(QPalette::Base), QColor("#141720"));
        QCOMPARE(tm.customColor("editorBg"), QColor("#1b1e2a"));
        QCOMPARE(tm.customColor("editorSelection"), QColor("#354b83"));
        QVERIFY(tm.loadTheme("helios-dark"));
    }

    void testThemeDefinitionParser() {
        const ThemeDefinition fallback =
            ThemeDefinitionParser::fallback(true);
        QCOMPARE(fallback.palette.color(QPalette::Window),
                 QColor("#11131a"));
        QCOMPARE(fallback.customColors.value("editorBg"),
                 QColor("#1b1e2a"));
        QCOMPARE(fallback.syntaxStyles.value("comment").color,
                 QColor("#6A5A8A"));

        const QJsonDocument document(QJsonObject{
            {"palette", QJsonObject{{"window", "#123456"}}},
            {"custom", QJsonObject{{"accent", "#abcdef"},
                                    {"invalid", "not-a-color"}}},
            {"syntax", QJsonObject{
                            {"comment", QJsonObject{{"color", "#654321"},
                                                     {"italic", true}}}}}});

        const std::optional<ThemeDefinition> definition =
            ThemeDefinitionParser::parse(document, true);
        QVERIFY(definition.has_value());
        QCOMPARE(definition->palette.color(QPalette::Window),
                 QColor("#123456"));
        QCOMPARE(definition->customColors.value("accent"),
                 QColor("#abcdef"));
        QVERIFY(!definition->customColors.contains("invalid"));
        QCOMPARE(definition->syntaxStyles.value("comment").color,
                 QColor("#654321"));
        QVERIFY(definition->syntaxStyles.value("comment").italic);

        QVERIFY(!ThemeDefinitionParser::parse(QJsonDocument(), true)
                     .has_value());
    }

    void testThemeCatalog() {
        const QStringList themeIds = {
            "helios-dark",
            "helios-light",
            "colormind-midnight",
            "colormind-desert",
            "helios-violet-dark",
            "helios-red-dark",
            "helios-gold-dark",
            "helios-blue-dark",
            "helios-forest-dark",
            "helios-carbon-dark"
        };
        const QStringList paletteKeys = {
            "window", "windowText", "base", "alternateBase", "toolTipBase",
            "toolTipText", "text", "button", "buttonText", "brightText",
            "link", "highlight", "highlightedText"
        };
        const QStringList customKeys = {
            "sidebar", "sidebarBorder", "sidebarHover", "sidebarActive",
            "sidebarActiveBorder", "tabWidgetPane", "tabBarBg", "tabBg",
            "tabFg", "tabBorder", "tabSelectedFg", "tabHoverBg", "treeBg",
            "treeFg", "treeHover", "treeSelected", "treeSelectedFg",
            "treeBranch", "editorBg", "editorFg", "editorLineNumber",
            "editorCurrentLine", "editorSelection", "gutterBg", "gutterActive",
            "bracketBg", "bracketFg", "diagnosticError", "diagnosticWarning",
            "diagnosticInfo", "diagnosticUnknown"
        };
        const QStringList syntaxKeys = {
            "comment", "string", "number", "type", "control", "declaration",
            "storage", "async", "exception", "keyword", "literal",
            "jump", "preprocessor", "logicalOperator", "operator",
            "otherOperator", "bracket", "punctuation"
        };

        auto &tm = ThemeManager::instance();
        for (const QString &themeId : themeIds) {
            const QString path = QStringLiteral(":/appdata/themes/%1.json")
                                     .arg(themeId);
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(path));

            QJsonParseError error;
            const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
            QVERIFY2(!document.isNull() && document.isObject(),
                     qPrintable(error.errorString()));

            const QJsonObject root = document.object();
            QVERIFY(root.value("name").isString());
            const QJsonObject palette = root.value("palette").toObject();
            const QJsonObject custom = root.value("custom").toObject();
            const QJsonObject syntax = root.value("syntax").toObject();
            for (const QString &key : paletteKeys) {
                QVERIFY2(palette.value(key).isString(), qPrintable(themeId + ":" + key));
                QVERIFY(QColor(palette.value(key).toString()).isValid());
            }
            for (const QString &key : customKeys) {
                QVERIFY2(custom.value(key).isString(), qPrintable(themeId + ":" + key));
                QVERIFY(QColor(custom.value(key).toString()).isValid());
            }
            for (const QString &key : syntaxKeys) {
                const QJsonObject style = syntax.value(key).toObject();
                QVERIFY2(style.value("color").isString(), qPrintable(themeId + ":syntax:" + key));
                QVERIFY(QColor(style.value("color").toString()).isValid());
            }

            QVERIFY2(tm.loadTheme(themeId), qPrintable(themeId));
            QCOMPARE(tm.currentThemeName(), themeId);
            QVERIFY(tm.palette().color(QPalette::Window).isValid());
            QVERIFY(tm.customColor("editorBg").isValid());
            QVERIFY(tm.customColor("diagnosticError").isValid());
            QVERIFY(tm.syntaxStyle("comment").color.isValid());
            QVERIFY2(tm.loadTheme(themeId), qPrintable(themeId));
        }
    }

    void testSemanticColors() {
        auto &tm = ThemeManager::instance();

        QVERIFY(tm.loadTheme("helios-dark"));
        QVERIFY(tm.isDark());
        QCOMPARE(tm.semanticColor(ThemeManager::SemanticRole::Canvas),
                 tm.palette().color(QPalette::Window));
        QCOMPARE(tm.semanticColor(ThemeManager::SemanticRole::Text),
                 tm.palette().color(QPalette::Text));
        QCOMPARE(tm.semanticColor(ThemeManager::SemanticRole::InputBg),
                 tm.palette().color(QPalette::Base));
        QCOMPARE(tm.semanticColor(ThemeManager::SemanticRole::Accent),
                 tm.palette().color(QPalette::Link));
        QCOMPARE(tm.semanticColor(ThemeManager::SemanticRole::Success),
                 QColor("#a6d189"));

        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QFile themeFile(QDir(tempDir.path()).filePath("partial-theme.json"));
        QVERIFY(themeFile.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&themeFile);
        out << "{\n"
            << "  \"name\": \"Partial Theme\",\n"
            << "  \"palette\": {\n"
            << "    \"window\": \"#101418\",\n"
            << "    \"windowText\": \"#eef2f7\",\n"
            << "    \"base\": \"#0c0f13\",\n"
            << "    \"alternateBase\": \"#161b21\",\n"
            << "    \"toolTipBase\": \"#1c222a\",\n"
            << "    \"toolTipText\": \"#eef2f7\",\n"
            << "    \"text\": \"#d9deeb\",\n"
            << "    \"button\": \"#202638\",\n"
            << "    \"buttonText\": \"#e6e9f2\",\n"
            << "    \"brightText\": \"#ff7a90\",\n"
            << "    \"link\": \"#8fa2ff\",\n"
            << "    \"highlight\": \"#3b5ccc\",\n"
            << "    \"highlightedText\": \"#ffffff\"\n"
            << "  },\n"
            << "  \"custom\": { \"editorBg\": \"#10151f\" },\n"
            << "  \"syntax\": {\n"
            << "    \"comment\": { \"color\": \"#6A5A8A\" }\n"
            << "  }\n"
            << "}\n";
        themeFile.close();

        QVERIFY(tm.loadThemeFile(themeFile.fileName()));
        QCOMPARE(tm.semanticColor(ThemeManager::SemanticRole::Surface),
                 tm.customColor("sidebar", tm.palette().color(QPalette::Window)));
        QCOMPARE(tm.semanticColor(ThemeManager::SemanticRole::TextFaint),
                 tm.customColor("editorLineNumber",
                                tm.semanticColor(ThemeManager::SemanticRole::TextMuted)));

        QVERIFY(tm.loadTheme("helios-light"));
        QVERIFY(!tm.isDark());
        QCOMPARE(tm.semanticColor(ThemeManager::SemanticRole::Canvas),
                 tm.palette().color(QPalette::Window));
        QCOMPARE(tm.semanticColor(ThemeManager::SemanticRole::Text),
                 tm.palette().color(QPalette::Text));
        QCOMPARE(tm.semanticColor(ThemeManager::SemanticRole::Success),
                 QColor("#237a57"));
    }

    void testSyntaxThemeReload() {
        auto &tm = ThemeManager::instance();
        QVERIFY(tm.loadTheme("helios-dark"));
        QTextDocument document;
        SyntaxHighlighter highlighter(&document);
        document.setPlainText("fn value = 42 // note");
        highlighter.rehighlight();
        QVERIFY(tm.loadTheme("helios-light"));
        const QTextLayout::FormatRange range = document.firstBlock().layout()->formats().first();
        QVERIFY(range.format.foreground().color().isValid());
    }

    void testFileIconResolver() {
        QCOMPARE(fileIconResourceForSuffix("zith"),
                 QStringLiteral(":/icons/file-zith.svg"));
        QCOMPARE(fileIconResourceForSuffix("c"),
                 QStringLiteral(":/icons/file-c.svg"));
        QCOMPARE(fileIconResourceForSuffix("h"),
                 QStringLiteral(":/icons/file-h.svg"));
        QCOMPARE(fileIconResourceForSuffix("H"),
                 QStringLiteral(":/icons/file-h.svg"));
        QVERIFY(fileIconResourceForSuffix("cpp").isEmpty());
        QVERIFY(fileIconResourceForSuffix("").isEmpty());
        QVERIFY(!fileIconForSuffix("zith").isNull());
        QVERIFY(!fileIconForPath("/tmp/sample.c").isNull());
        QVERIFY(!fileIconForPath("/tmp/SAMPLE.H").isNull());
        QFile zithIcon(QStringLiteral(":/icons/file-zith.svg"));
        QFile cIcon(QStringLiteral(":/icons/file-c.svg"));
        QFile hIcon(QStringLiteral(":/icons/file-h.svg"));
        QVERIFY(zithIcon.open(QIODevice::ReadOnly));
        QVERIFY(cIcon.open(QIODevice::ReadOnly));
        QVERIFY(hIcon.open(QIODevice::ReadOnly));
        QVERIFY(zithIcon.size() > 0);
        QVERIFY(cIcon.size() > 0);
        QVERIFY(hIcon.size() > 0);
    }

    void testCHighlighterFormatsCSample() {
        auto &tm = ThemeManager::instance();
        QVERIFY(tm.loadTheme("helios-dark"));

        QTextDocument document;
        CHighlighter highlighter(&document);
        document.setPlainText(
            "#include <stdio.h>\n"
            "int main(void) { const char *msg = \"hello\"; return 0; }");
        highlighter.rehighlight();

        const QList<QTextLayout::FormatRange> formats =
            document.firstBlock().layout()->formats();
        QVERIFY(!formats.isEmpty());
        for (const QTextLayout::FormatRange &range : formats) {
            QVERIFY(range.format.foreground().color().isValid());
        }

        QTextDocument secondDoc;
        highlighter.setDocument(&secondDoc);
        secondDoc.setPlainText("// comment\n\"string\" 42");
        highlighter.rehighlight();

        const QColor commentColor = tm.syntaxStyle("comment").color;
        bool hasComment = false;
        for (const QTextLayout::FormatRange &range :
             secondDoc.firstBlock().layout()->formats()) {
            if (range.format.foreground().color() == commentColor)
                hasComment = true;
        }
        QVERIFY(hasComment);

        const QTextBlock stringBlock = secondDoc.findBlockByNumber(1);
        const QColor stringColor = tm.syntaxStyle("string").color;
        bool hasString = false;
        bool hasNumber = false;
        for (const QTextLayout::FormatRange &range :
             stringBlock.layout()->formats()) {
            if (range.format.foreground().color() == stringColor)
                hasString = true;
            if (range.format.foreground().color() ==
                tm.syntaxStyle("number").color)
                hasNumber = true;
        }
        QVERIFY(hasString);
        QVERIFY(hasNumber);

        QTextDocument thirdDoc;
        highlighter.setDocument(&thirdDoc);
        thirdDoc.setPlainText(
            "#include <stdio.h>\n"
            "void loop(void) { return; }\n"
            "int main(void) { goto done; break; continue; done: return 0; }");
        highlighter.rehighlight();

        const QColor jumpColor = tm.syntaxStyle("jump").color;
        const QColor preprocessorColor = tm.syntaxStyle("preprocessor").color;
        bool hasJump = false;
        bool hasPreprocessor = false;
        for (int block = 0; block < thirdDoc.blockCount(); ++block) {
            const QTextBlock textBlock = thirdDoc.findBlockByNumber(block);
            for (const QTextLayout::FormatRange &range :
                 textBlock.layout()->formats()) {
                if (range.format.foreground().color() == jumpColor)
                    hasJump = true;
                if (range.format.foreground().color() == preprocessorColor)
                    hasPreprocessor = true;
            }
        }
        QVERIFY(hasJump);
        QVERIFY(hasPreprocessor);
    }

    void testZithHighlighterStillHighlightsSample() {
        auto &tm = ThemeManager::instance();
        QVERIFY(tm.loadTheme("helios-dark"));

        QTextDocument document;
        SyntaxHighlighter highlighter(&document);
        document.setPlainText("#include <stdio.h>\nint main(void) { return 0; }");
        highlighter.rehighlight();
        const QList<QTextLayout::FormatRange> formats =
            document.firstBlock().layout()->formats();
        QVERIFY(!formats.isEmpty());
    }

    void testCustomThemeFileAndScaleIsolation() {
        auto &store = TomlSettingsStore::instance();
        store.setEditorFontSize(17);
        store.setUiFontSize(13);
        store.setUiScale(175);

        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QFile themeFile(QDir(tempDir.path()).filePath("custom-theme.json"));
        QVERIFY(themeFile.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&themeFile);
        out << "{\n"
            << "  \"name\": \"Custom Theme\",\n"
            << "  \"palette\": {\n"
            << "    \"window\": \"#1a1f2b\",\n"
            << "    \"windowText\": \"#f0f3f8\",\n"
            << "    \"base\": \"#131722\",\n"
            << "    \"alternateBase\": \"#1e2431\",\n"
            << "    \"toolTipBase\": \"#252b3a\",\n"
            << "    \"toolTipText\": \"#edf0fa\",\n"
            << "    \"text\": \"#d9deeb\",\n"
            << "    \"button\": \"#202638\",\n"
            << "    \"buttonText\": \"#e6e9f2\",\n"
            << "    \"brightText\": \"#ff7a90\",\n"
            << "    \"link\": \"#8fa2ff\",\n"
            << "    \"highlight\": \"#3b5ccc\",\n"
            << "    \"highlightedText\": \"#ffffff\"\n"
            << "  },\n"
            << "  \"custom\": { \"editorBg\": \"#10151f\" },\n"
            << "  \"syntax\": {\n"
            << "    \"comment\": { \"color\": \"#6A5A8A\" }\n"
            << "  }\n"
            << "}\n";
        themeFile.close();

        QVERIFY(AppearanceController::instance().setThemeFile(themeFile.fileName()));
        QCOMPARE(TomlSettingsStore::instance().theme(), QString("custom"));
        QCOMPARE(TomlSettingsStore::instance().customThemePath(), themeFile.fileName());
        QCOMPARE(AppearanceController::instance().editorFont().pointSize(), 17);
        QCOMPARE(AppearanceController::instance().uiFont().pointSize(),
                 qRound(13 * 1.75));
    }

    void testVimMotions() {
        QPlainTextEdit editor;
        editor.setPlainText("alpha beta\ngamma delta");
        VimMotionController controller(&editor);
        controller.setEnabled(true);
        auto press = [&controller](int key, const QString &text) {
            QKeyEvent event(QEvent::KeyPress, key, Qt::NoModifier, text);
            QVERIFY(controller.handleKeyPress(&event));
        };
        press(Qt::Key_2, "2");
        press(Qt::Key_L, "l");
        QCOMPARE(editor.textCursor().positionInBlock(), 2);
        press(Qt::Key_W, "w");
        QCOMPARE(editor.textCursor().positionInBlock(), 6);
        press(Qt::Key_F, "f");
        press(Qt::Key_A, "a");
        QCOMPARE(editor.textCursor().positionInBlock(), 9);
        press(Qt::Key_G, "g");
        press(Qt::Key_G, "g");
        QCOMPARE(editor.textCursor().blockNumber(), 0);
        press(Qt::Key_Semicolon, ";");
        QCOMPARE(editor.textCursor().positionInBlock(), 4);
        press(Qt::Key_Semicolon, ";");
        QCOMPARE(editor.textCursor().positionInBlock(), 9);
        press(Qt::Key_Comma, ",");
        QCOMPARE(editor.textCursor().positionInBlock(), 4);
        press(Qt::Key_W, "w");
        press(Qt::Key_E, "e");
        QCOMPARE(editor.textCursor().positionInBlock(), 10);
        press(Qt::Key_I, "i");
        QCOMPARE(controller.mode(), VimMotionController::Mode::Insert);
        QKeyEvent escape(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QVERIFY(controller.handleKeyPress(&escape));
        QCOMPARE(controller.mode(), VimMotionController::Mode::Normal);

        editor.setPlainText("alpha beta\ngamma delta\n   ");
        QTextCursor whitespaceLine(
            editor.document()->findBlockByNumber(2));
        editor.setTextCursor(whitespaceLine);
        QKeyEvent caret(QEvent::KeyPress, Qt::Key_AsciiCircum,
                        Qt::NoModifier, "^");
        QVERIFY(controller.handleKeyPress(&caret));
        QCOMPARE(editor.textCursor().positionInBlock(), 0);
    }

    void testVimMotionResolver() {
        const auto left = VimMotionResolver::resolve(QLatin1Char('h'));
        QVERIFY(left.has_value());
        QCOMPARE(left->operation, QTextCursor::Left);

        const auto nextWord = VimMotionResolver::resolve(QLatin1Char('W'));
        QVERIFY(nextWord.has_value());
        QCOMPARE(nextWord->operation, QTextCursor::NextWord);

        const auto previousWord = VimMotionResolver::resolve(QLatin1Char('B'));
        QVERIFY(previousWord.has_value());
        QCOMPARE(previousWord->operation, QTextCursor::PreviousWord);

        QVERIFY(!VimMotionResolver::resolve(QLatin1Char('$')).has_value());
        QVERIFY(!VimMotionResolver::resolve(QLatin1Char('g')).has_value());
    }

    void testVimCharacterSearch() {
        QPlainTextEdit editor;
        editor.setPlainText("alpha beta alpha");
        VimCharacterSearch search(&editor);

        QTextCursor start(editor.document());
        editor.setTextCursor(start);
        QVERIFY(search.find({'a', true, false, 2}));
        QCOMPARE(editor.textCursor().positionInBlock(), 9);

        QVERIFY(search.repeatLast(true, 1));
        QCOMPARE(editor.textCursor().positionInBlock(), 4);

        search.reset();
        QVERIFY(search.repeatLast(false, 1));
        QCOMPARE(editor.textCursor().positionInBlock(), 4);
    }

    void testVimCommandSession() {
        VimCommandSession session;
        QKeyEvent inactive(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier,
                           "a");
        QVERIFY(!session.handleKeyPress(&inactive));

        session.begin();
        QVERIFY(session.isActive());
        QKeyEvent w(QEvent::KeyPress, Qt::Key_W, Qt::NoModifier, "w");
        QKeyEvent q(QEvent::KeyPress, Qt::Key_Q, Qt::NoModifier, "q");
        QKeyEvent backspace(QEvent::KeyPress, Qt::Key_Backspace,
                            Qt::NoModifier);
        QVERIFY(session.handleKeyPress(&w));
        QVERIFY(session.handleKeyPress(&q));
        QVERIFY(session.handleKeyPress(&backspace));
        QVERIFY(session.handleKeyPress(&q));

        QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QVERIFY(session.handleKeyPress(&enter));
        QVERIFY(!session.isActive());
        const std::optional<QString> command =
            session.takeSubmittedCommand();
        QVERIFY(command.has_value());
        QCOMPARE(*command, QString("wq"));
        QVERIFY(!session.takeSubmittedCommand().has_value());

        session.begin();
        QKeyEvent escape(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QVERIFY(session.handleKeyPress(&escape));
        QVERIFY(!session.isActive());
        QVERIFY(!session.takeSubmittedCommand().has_value());
    }

    void testVimPendingOperationSession() {
        QPlainTextEdit editor;
        editor.setPlainText("alpha\nbeta\ngamma");
        VimCharacterSearch characterSearch(&editor);
        VimPendingOperationSession session(&editor, characterSearch);

        QTextCursor start(editor.document());
        editor.setTextCursor(start);
        session.begin({VimPendingOperation::Delete, 1, 0});
        QKeyEvent deleteLine(QEvent::KeyPress, Qt::Key_D, Qt::NoModifier,
                             "d");
        const VimPendingOperationResult deleted =
            session.handleKeyPress(&deleteLine);
        QVERIFY(deleted.handled);
        QVERIFY(!deleted.enterInsertMode);
        QCOMPARE(editor.toPlainText(), QString("beta\ngamma"));

        editor.setPlainText("alpha beta");
        editor.setTextCursor(QTextCursor(editor.document()));
        session.begin({VimPendingOperation::Change, 1, 0});
        QKeyEvent moveToWord(QEvent::KeyPress, Qt::Key_W, Qt::NoModifier,
                             "w");
        const VimPendingOperationResult changed =
            session.handleKeyPress(&moveToWord);
        QVERIFY(changed.handled);
        QVERIFY(changed.enterInsertMode);
        QCOMPARE(editor.toPlainText(), QString("beta"));
    }

    void testTranslationManagerFallback() {
        auto &tr = TranslationManager::instance();
        tr.loadLocale("pt-BR");
        QString val = tr.translate("invalid.key");
        QCOMPARE(val, QString("invalid.key"));

        QString validVal = tr.translate("menu.new_file");
        QCOMPARE(validVal, QString("Novo Arquivo"));
    }

    void testSettingsPanelPreferencesCta() {
        TranslationManager::instance().loadLocale("en-US");

        SettingsPanel panel;
        QSignalSpy spy(&panel, &SettingsPanel::openPreferencesRequested);

        auto *button = panel.findChild<QPushButton *>("openPreferencesButton");
        QVERIFY(button);
        QVERIFY(!button->text().isEmpty());

        button->click();
        QCOMPARE(spy.count(), 1);
    }

    void testLspProtocolCodecFramesIncrementally() {
        const QJsonObject first{{"jsonrpc", "2.0"},
                                {"method", "test/first"}};
        const QJsonObject second{{"jsonrpc", "2.0"},
                                 {"method", "test/second"}};
        QByteArray firstFrame;
        QByteArray secondFrame;
        QString error;
        QVERIFY(LspProtocolCodec::encode(first, firstFrame, error));
        QVERIFY(error.isEmpty());
        QVERIFY(LspProtocolCodec::encode(second, secondFrame, error));

        LspProtocolCodec codec;
        const qsizetype split = firstFrame.size() / 2;
        const auto partial = codec.consume(firstFrame.left(split));
        QVERIFY(partial.messages.isEmpty());
        QVERIFY(partial.errors.isEmpty());

        const auto completed =
            codec.consume(firstFrame.mid(split) + secondFrame);
        QCOMPARE(completed.messages.size(), 2);
        QCOMPARE(completed.messages.at(0), first);
        QCOMPARE(completed.messages.at(1), second);

        const auto malformed =
            codec.consume("Content-Length: invalid\r\n\r\n{}");
        QCOMPARE(malformed.messages.size(), 0);
        QCOMPARE(malformed.errors,
                 QStringList{QStringLiteral(
                     "Discarded malformed LSP frame header")});
    }

    void testLspRequestTrackerReplacesAndCancelsByUri() {
        LspRequestTracker tracker;

        QVERIFY(!tracker.track(
            {1, QStringLiteral("textDocument/hover"),
             QStringLiteral("file:///main.zith"), 1, true}));

        const auto replaced = tracker.track(
            {2, QStringLiteral("textDocument/hover"),
             QStringLiteral("file:///main.zith"), 2, true});
        QVERIFY(replaced.has_value());
        QCOMPARE(*replaced, qint64(1));
        QVERIFY(!tracker.take(1).has_value());

        const QList<qint64> cancelled =
            tracker.cancelForUri(QStringLiteral("file:///main.zith"));
        QCOMPARE(cancelled, QList<qint64>{qint64(2)});
        QVERIFY(!tracker.take(2).has_value());
    }

    void testLspInitializationBuilderSeparatesServerModes() {
        const QJsonObject zith = LspInitializationBuilder::build(
            {QStringLiteral("/tmp/zith-project"),
             QStringLiteral("/tmp/zith-stdlib"),
             QStringLiteral("zith")});
        QCOMPARE(zith.value("rootPath").toString(),
                 QStringLiteral("/tmp/zith-project"));
        QVERIFY(zith.value("rootUri").toString().startsWith(
            QStringLiteral("file:///tmp/zith-project")));
        QVERIFY(zith.value("capabilities").toObject()
                    .value("experimental").toObject()
                    .value("zith").toObject()
                    .value("requestSaveAll").toBool());
        QCOMPARE(zith.value("initializationOptions").toObject()
                     .value("stdlibPath").toString(),
                 QStringLiteral("/tmp/zith-stdlib"));

        const QJsonObject clangd = LspInitializationBuilder::build(
            {QStringLiteral("/tmp/cpp-project"), QStringLiteral("/ignored"),
             QStringLiteral("clangd")});
        QVERIFY(!clangd.value("capabilities").toObject().contains("experimental"));
        QVERIFY(!clangd.contains("initializationOptions"));
        QCOMPARE(clangd.value("rootPath").toString(),
                 QStringLiteral("/tmp/cpp-project"));
    }

    void testLspDocumentSyncBuildsIncrementalAndFullBatches() {
        LspDocumentSync sync;
        sync.setDocument(QStringLiteral("file:///main.zith"),
                         QStringLiteral("one\nx"), 7);
        sync.recordChange(
            {4, 1, QStringLiteral("y"), QStringLiteral("one\ny")});

        const auto incremental = sync.takeBatch(2);
        QVERIFY(incremental.has_value());
        QCOMPARE(incremental->version, 8);
        QVERIFY(!incremental->fullSync);
        QCOMPARE(incremental->fullText, QStringLiteral("one\ny"));
        QCOMPARE(incremental->changes.size(), 1);
        QCOMPARE(incremental->changes.first().range.start.line, 1);
        QCOMPARE(incremental->changes.first().range.start.character, 0);
        QCOMPARE(incremental->changes.first().range.end.line, 1);
        QCOMPARE(incremental->changes.first().range.end.character, 1);
        QCOMPARE(incremental->changes.first().text, QStringLiteral("y"));
        QVERIFY(!sync.takeBatch(2).has_value());

        sync.recordChange(
            {4, 0, QStringLiteral("!"), QStringLiteral("one\n!\ny")});
        const auto full = sync.takeBatch(1);
        QVERIFY(full.has_value());
        QCOMPARE(full->version, 9);
        QVERIFY(full->fullSync);
        QCOMPARE(full->fullText, QStringLiteral("one\n!\ny"));
        QCOMPARE(LspDocumentSync::positionForOffset(full->fullText, 6).line,
                 2);
        QCOMPARE(
            LspDocumentSync::positionForOffset(full->fullText, 6).character,
            0);
    }

    void testLspDocumentProtocolOwnsDocumentNotificationsAndVersions() {
        const QString uri = QStringLiteral("file:///main.zith");
        QList<QJsonObject> messages;
        QStringList events;
        bool ready = true;

        LspDocumentProtocol protocol(
            {[&]() { return ready; },
             [&](const QJsonObject &message) {
                 messages.append(message);
                 events.append(QStringLiteral("send"));
                 return true;
             },
             [&](const QString &cancelledUri) {
                 QCOMPARE(cancelledUri, uri);
                 events.append(QStringLiteral("cancel"));
             }});

        protocol.openDocument(uri, QStringLiteral("zith"),
                             QStringLiteral("one"), 3);
        QCOMPARE(messages.size(), 1);
        QCOMPARE(messages.first().value("method").toString(),
                 QStringLiteral("textDocument/didOpen"));
        QCOMPARE(protocol.documentVersion(uri), 3);
        QVERIFY(protocol.isCurrentDocument(uri, 3));

        events.clear();
        const LspTextChange change{
            LspRange{{0, 1}, {0, 2}}, QStringLiteral("y")};
        protocol.changeDocument(uri, {change}, 4);
        QCOMPARE(events, QStringList({QStringLiteral("cancel"),
                                      QStringLiteral("send")}));
        QCOMPARE(protocol.documentVersion(uri), 4);
        const QJsonObject changeParams =
            messages.last().value("params").toObject();
        QCOMPARE(changeParams.value("textDocument")
                     .toObject()
                     .value("version")
                     .toInt(),
                 4);
        QCOMPARE(changeParams.value("contentChanges")
                     .toArray()
                     .first()
                     .toObject()
                     .value("range")
                     .toObject()
                     .value("start")
                     .toObject()
                     .value("character")
                     .toInt(),
                 1);

        events.clear();
        protocol.changeDocumentFull(uri, QStringLiteral("full text"), 5);
        QCOMPARE(events, QStringList({QStringLiteral("cancel"),
                                      QStringLiteral("send")}));
        QCOMPARE(protocol.documentVersion(uri), 5);
        QCOMPARE(messages.last().value("params")
                     .toObject()
                     .value("contentChanges")
                     .toArray()
                     .first()
                     .toObject()
                     .value("text")
                     .toString(),
                 QStringLiteral("full text"));

        protocol.saveDocument(uri);
        QCOMPARE(messages.last().value("method").toString(),
                 QStringLiteral("textDocument/didSave"));

        protocol.closeDocument(uri);
        QCOMPARE(events.last(), QStringLiteral("send"));
        QVERIFY(!protocol.isCurrentDocument(uri, 4));
        QCOMPARE(protocol.documentVersion(uri), 1);

        ready = false;
        const int messageCount = messages.size();
        protocol.openDocument(uri, QStringLiteral("zith"),
                              QStringLiteral("ignored"), 8);
        protocol.saveDocument(uri);
        QCOMPARE(messages.size(), messageCount);
    }

    void testLspShutdownEscalationTerminatesThenKillsOnce() {
        LspShutdownEscalation escalation;

        QCOMPARE(escalation.nextAction(false),
                 LspShutdownEscalation::Action::None);
        QCOMPARE(escalation.nextAction(true),
                 LspShutdownEscalation::Action::Terminate);
        QCOMPARE(escalation.nextAction(true),
                 LspShutdownEscalation::Action::Kill);
        QCOMPARE(escalation.nextAction(true),
                 LspShutdownEscalation::Action::None);

        escalation.reset();
        QCOMPARE(escalation.nextAction(true),
                 LspShutdownEscalation::Action::Terminate);
    }

    void testEditorDocumentSyncControllerOwnsBatchingPolicy() {
        bool canSend = true;
        int syncKind = 2;
        QList<LspDocumentSyncBatch> incrementalBatches;
        QList<LspDocumentSyncBatch> fullBatches;
        bool callbackUriValid = true;
        bool transportAvailable = true;

        EditorDocumentSyncController controller(
            {[&canSend]() { return canSend; },
             [&syncKind]() { return syncKind; },
             [&incrementalBatches, &callbackUriValid, &transportAvailable](
                 const QString &uri, const QList<LspTextChange> &changes,
                 int version) {
                 incrementalBatches.append(
                     {version, changes, QString(), false});
                 callbackUriValid =
                     callbackUriValid &&
                     uri == QStringLiteral("file:///main.zith");
                 return callbackUriValid && transportAvailable;
             },
             [&fullBatches, &transportAvailable](
                 const QString &uri, const QString &text, int version) {
                 if (!transportAvailable)
                     return false;
                 fullBatches.append({version, {}, text, true});
                 return uri == QStringLiteral("file:///main.zith");
             }});

        controller.setDocument(QStringLiteral("file:///main.zith"),
                               QStringLiteral("abc"), 7);
        controller.recordChange({3, 0, QStringLiteral("d"),
                                 QStringLiteral("abcd")});
        controller.flush();

        QVERIFY(callbackUriValid);
        QCOMPARE(controller.version(), 8);
        QCOMPARE(incrementalBatches.size(), 1);
        QCOMPARE(incrementalBatches.first().version, 8);
        QCOMPARE(incrementalBatches.first().changes.size(), 1);
        QCOMPARE(incrementalBatches.first().changes.first().text,
                 QStringLiteral("d"));

        syncKind = 1;
        controller.recordChange({4, 0, QStringLiteral("e"),
                                 QStringLiteral("abcde")});
        controller.flush();

        QCOMPARE(controller.version(), 9);
        QCOMPARE(fullBatches.size(), 1);
        QCOMPARE(fullBatches.first().version, 9);
        QCOMPARE(fullBatches.first().fullText, QStringLiteral("abcde"));

        canSend = false;
        controller.recordChange({5, 0, QStringLiteral("f"),
                                 QStringLiteral("abcdef")});
        controller.flush();
        QCOMPARE(controller.version(), 9);

        canSend = true;
        controller.flush();
        QCOMPARE(controller.version(), 10);
        QCOMPARE(fullBatches.size(), 2);
        QCOMPARE(fullBatches.last().version, 10);
        QCOMPARE(fullBatches.last().fullText, QStringLiteral("abcdef"));

        transportAvailable = false;
        controller.recordChange({7, 0, QStringLiteral("h"),
                                 QStringLiteral("abcdefgh")});
        controller.flush();
        QCOMPARE(controller.version(), 10);
        QCOMPARE(fullBatches.size(), 2);

        transportAvailable = true;
        controller.flush();
        QCOMPARE(controller.version(), 11);
        QCOMPARE(fullBatches.size(), 3);
        QCOMPARE(fullBatches.last().version, 11);
        QCOMPARE(fullBatches.last().fullText, QStringLiteral("abcdefgh"));

        controller.recordChange({6, 0, QStringLiteral("g"),
                                 QStringLiteral("abcdefg")});
        controller.markDocumentSynchronized();
        controller.flush();
        QCOMPARE(controller.version(), 11);
        QCOMPARE(fullBatches.size(), 3);
    }

    void testEditorAppearanceControllerProjectsEditorTheme() {
        QPlainTextEdit editor;
        EditorAppearanceController controller(&editor);
        QSignalSpy appearanceSpy(
            &controller, &EditorAppearanceController::appearanceChanged);

        controller.apply();

        const EditorAppearance &appearance = controller.appearance();
        QCOMPARE(editor.palette().color(QPalette::Base),
                 appearance.background);
        QCOMPARE(editor.palette().color(QPalette::Text),
                 appearance.foreground);
        QVERIFY(editor.styleSheet().contains(appearance.background.name()));
        QVERIFY(editor.styleSheet().contains(appearance.selection.name()));
        QCOMPARE(appearanceSpy.count(), 1);
    }

    void testEditorContextMenuControllerBuildsCapabilityAwareMenu() {
        CodeEditor editor;
        editor.setPlainText(QStringLiteral("symbol"));
        EditorLanguageFeatureController features(&editor);
        EditorContextMenuController controller(&editor, &features);

        const std::unique_ptr<QMenu> menu = controller.createMenu();
        QVERIFY(menu);
        QCOMPARE(menu->actions().size(), 10);
        QVERIFY(menu->actions().at(4)->isSeparator());
        QVERIFY(menu->actions().at(7)->isSeparator());

        for (int index : {0, 1, 2, 3, 5, 6, 8})
            QVERIFY(!menu->actions().at(index)->isEnabled());
        QVERIFY(menu->actions().at(9)->isEnabled());
        QVERIFY(!menu->styleSheet().isEmpty());
    }

    void testLspSessionLifecycleOwnsInitializationAndPendingRestart() {
        bool hasProcess = false;
        bool running = false;
        int starts = 0;
        int clearCalls = 0;
        QList<QJsonObject> messages;
        QList<QString> requestMethods;
        QList<std::function<void(const QJsonObject &)>> responses;

        LspSessionLifecycle lifecycle(
            {[&messages](const QJsonObject &message) {
                 messages.append(message);
                 return true;
             },
             [&requestMethods, &responses](
                 const QString &method, const QJsonObject &,
                 const QString &, int, bool,
                 std::function<void(const QJsonObject &)> callback) {
                 requestMethods.append(method);
                 responses.append(std::move(callback));
                 return static_cast<qint64>(requestMethods.size());
             },
             [&hasProcess]() { return hasProcess; },
             [&running]() { return running; },
             [&hasProcess, &running, &starts](const QString &) {
                 hasProcess = true;
                 running = true;
                 ++starts;
                 return true;
             },
             []() {},
             []() {},
             [&clearCalls]() { ++clearCalls; },
             [](const QJsonObject &) {}});

        QSignalSpy initializedSpy(&lifecycle, &LspSessionLifecycle::initialized);
        QSignalSpy stoppedSpy(&lifecycle, &LspSessionLifecycle::processStopped);

        lifecycle.start(
            {QStringLiteral("/tmp/lsp"), QString(), QString(), QString()}, false);
        QCOMPARE(starts, 1);
        lifecycle.processStarted();
        QCOMPARE(requestMethods, QList<QString>{QStringLiteral("initialize")});
        QVERIFY(!responses.isEmpty());

        responses.first()(QJsonObject{
            {"result", QJsonObject{{"capabilities", QJsonObject{}}}}});
        QCOMPARE(initializedSpy.count(), 1);
        QVERIFY(lifecycle.isReady());

        lifecycle.start(
            {QStringLiteral("/tmp/lsp-next"), QString(), QString(), QString()},
            true);
        const QList<QString> expectedMethods{
            QStringLiteral("initialize"), QStringLiteral("shutdown")};
        QCOMPARE(requestMethods, expectedMethods);
        QVERIFY(!messages.isEmpty());
        QCOMPARE(messages.first().value("method").toString(),
                 QStringLiteral("initialized"));

        responses.last()(QJsonObject{{"result", QJsonValue::Null}});
        running = false;
        hasProcess = false;
        lifecycle.processFinished({0, false}, QString());

        QCOMPARE(stoppedSpy.count(), 1);
        QCOMPARE(starts, 2);
        QCOMPARE(clearCalls, 3);
    }

    void testLspResultDecoderCoversProtocolValueVariants() {
        const QJsonObject rangeJson{
            {"start", QJsonObject{{"line", 2}, {"character", 3}}},
            {"end", QJsonObject{{"line", 2}, {"character", 8}}}};
        const LspRange range = LspResultDecoder::range(rangeJson);
        QCOMPARE(range.start.line, 2);
        QCOMPARE(range.start.character, 3);
        QCOMPARE(range.end.character, 8);

        const QJsonObject locationLink{
            {"targetUri", "file:///other.zith"},
            {"targetSelectionRange", rangeJson}};
        const LspLocation location =
            LspResultDecoder::location(locationLink);
        QCOMPARE(location.uri, QStringLiteral("file:///other.zith"));
        QCOMPARE(location.range.start.line, 2);

        const QList<LspLocation> locations = LspResultDecoder::locations(
            QJsonArray{locationLink,
                       QJsonObject{{"uri", "file:///main.zith"},
                                   {"range", rangeJson}}});
        QCOMPARE(locations.size(), 2);

        const QList<LspCompletionItem> items =
            LspResultDecoder::completionItems(
                QJsonObject{{"items", QJsonArray{
                    QJsonObject{{"label", "print"},
                                {"insertText", "print()"}}}}});
        QCOMPARE(items.size(), 1);
        QCOMPARE(items.first().label, QStringLiteral("print"));
        QCOMPARE(items.first().insertText, QStringLiteral("print()"));

        const LspHoverInfo hover = LspResultDecoder::hover(
            QJsonObject{{"contents",
                         QJsonArray{"first",
                                    QJsonObject{{"value", "second"}}}},
                        {"range", rangeJson}});
        QCOMPARE(hover.contents, QStringLiteral("first\n\nsecond"));
        QCOMPARE(hover.range.end.character, 8);

        const LspSignatureHelp signature = LspResultDecoder::signatureHelp(
            QJsonObject{{"activeParameter", 1},
                        {"signatures", QJsonArray{
                            QJsonObject{
                                {"label", "fn(a, b)"},
                                {"parameters", QJsonArray{
                                    QJsonObject{{"label", "a"}},
                                    QJsonObject{{"label", "b"}}}}}}}});
        QCOMPARE(signature.activeSignature, QStringLiteral("fn(a, b)"));
        const QStringList expectedParameters{QStringLiteral("a"),
                                             QStringLiteral("b")};
        QCOMPARE(signature.parameters, expectedParameters);
        QCOMPARE(signature.activeParameter, 1);

        const QList<QPair<LspRange, QString>> edits =
            LspResultDecoder::textEdits(
                QJsonArray{QJsonObject{{"range", rangeJson},
                                       {"newText", "replacement"}}});
        QCOMPARE(edits.size(), 1);
        QCOMPARE(edits.first().second, QStringLiteral("replacement"));

        const QList<LspDiagnostic> diagnostics =
            LspResultDecoder::diagnostics(
                QJsonArray{QJsonObject{{"range", rangeJson},
                                       {"severity", 1},
                                       {"message", "broken"},
                                       {"source", "zith"}}});
        QCOMPARE(diagnostics.size(), 1);
        QCOMPARE(diagnostics.first().message, QStringLiteral("broken"));
    }

    void testGitStatusParserExtractsBranchAndPaths() {
        const GitStatusSnapshot snapshot = GitStatusParser::parse(
            QStringLiteral("## main...origin/main\n"
                           " M src/main.cpp\n"
                           "R  old_name.cpp -> new_name.cpp\n"
                           "?? notes.txt\n"));

        QCOMPARE(snapshot.branch, QStringLiteral("main...origin/main"));
        QCOMPARE(snapshot.entries.size(), 3);
        QCOMPARE(snapshot.entries.at(0).status, QStringLiteral(" M"));
        QCOMPARE(snapshot.entries.at(0).relativePath,
                 QStringLiteral("src/main.cpp"));
        QCOMPARE(snapshot.entries.at(1).status, QStringLiteral("R "));
        QCOMPARE(snapshot.entries.at(1).relativePath,
                 QStringLiteral("new_name.cpp"));
        QCOMPARE(snapshot.entries.at(2).status, QStringLiteral("??"));
        QCOMPARE(snapshot.entries.at(2).relativePath,
                 QStringLiteral("notes.txt"));
    }

    void testGitCommandRunnerCapturesResultAndStartFailure() {
        GitCommandRunner runner;
        runner.setProgram(QStringLiteral("/bin/sh"));

        bool received = false;
        GitCommandResult result;
        connect(&runner, &GitCommandRunner::finished,
                [&](const GitCommandResult &completed) {
                    result = completed;
                    received = true;
                });

        QVERIFY(runner.run({QStringLiteral("-c"),
                            QStringLiteral("printf output; printf error >&2")}));
        QTRY_VERIFY_WITH_TIMEOUT(received, 1000);
        QVERIFY(result.succeeded);
        QVERIFY(!result.failedToStart);
        QVERIFY(!result.timedOut);
        QCOMPARE(result.standardOutput, QStringLiteral("output"));
        QCOMPARE(result.standardError, QStringLiteral("error"));

        received = false;
        runner.setProgram(QStringLiteral("/path/that/does/not/exist"));
        QVERIFY(runner.run({}));
        QTRY_VERIFY_WITH_TIMEOUT(received, 1000);
        QVERIFY(!result.succeeded);
        QVERIFY(result.failedToStart);
        QVERIFY(!result.timedOut);
    }

    void testZithReleaseCatalogParsesAndSelectsPlatformAssets() {
#ifdef Q_OS_WIN
        const QString lspName =
            QSysInfo::currentCpuArchitecture().contains("arm64")
                ? QStringLiteral("zith-lsp-windows-arm64.exe")
                : QStringLiteral("zith-lsp-windows-amd64.exe");
        const QString stdlibName = QStringLiteral("zithc-stdlib-windows.zip");
#elif defined(Q_OS_MACOS)
        const QString lspName = QStringLiteral("zith-lsp-macos-universal");
        const QString stdlibName = QStringLiteral("zithc-stdlib-macos.tar.gz");
#else
        const QString lspName =
            QSysInfo::currentCpuArchitecture().contains("arm64") ||
                    QSysInfo::currentCpuArchitecture().contains("aarch64")
                ? QStringLiteral("zith-lsp-linux-arm64")
                : QStringLiteral("zith-lsp-linux-amd64");
        const QString stdlibName = QStringLiteral("zithc-stdlib-linux.tar.gz");
#endif

        const QByteArray payload = QJsonDocument(QJsonObject{
            {"tag_name", "v0.7.0"},
            {"assets", QJsonArray{
                QJsonObject{{"name", lspName},
                            {"browser_download_url", "https://example.test/lsp"}},
                QJsonObject{{"name", stdlibName},
                            {"browser_download_url", "https://example.test/stdlib"}},
                QJsonObject{{"name", "ignored-without-url"}}
            }}
        }).toJson(QJsonDocument::Compact);

        QString error;
        const auto release = ZithReleaseCatalog::parse(payload, &error);
        QVERIFY(release.has_value());
        QVERIFY(error.isEmpty());
        QCOMPARE(release->tag, QStringLiteral("v0.7.0"));
        QCOMPARE(release->assets.size(), 2);

        const auto lsp = ZithReleaseCatalog::findLspAsset(*release);
        const auto stdlib = ZithReleaseCatalog::findStdlibAsset(*release);
        QVERIFY(lsp.has_value());
        QVERIFY(stdlib.has_value());
        QCOMPARE(lsp->name, lspName);
        QCOMPARE(stdlib->name, stdlibName);
        QCOMPARE(lsp->downloadUrl,
                 QUrl(QStringLiteral("https://example.test/lsp")));

        const auto invalid = ZithReleaseCatalog::parse(
            QByteArrayLiteral("{not-json}"), &error);
        QVERIFY(!invalid.has_value());
        QCOMPARE(error, QStringLiteral(
            "Latest Zith release response was not valid JSON."));
    }

    void testZithRuntimeInstallerInstallsLspBinary() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        ZithRuntimeCatalog catalog;
        const QString cacheRoot = QDir(tempDir.path()).filePath("runtime-cache");
        catalog.setCacheRoot(cacheRoot);

        const QString sourcePath = QDir(tempDir.path()).filePath("downloaded-lsp");
        QFile source(sourcePath);
        QVERIFY(source.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QCOMPARE(source.write("fake-zith-lsp\n"), qint64(14));
        source.close();

        ZithRuntimeInstaller installer(catalog);
        QString error;
        QVERIFY(installer.install(
            {ZithRuntimeAssetKind::LspBinary, sourcePath, QStringLiteral("v0.7.0")},
            &error));
        QVERIFY(error.isEmpty());

        const QString installedPath = catalog.lspInstallPath("v0.7.0");
        QVERIFY(QFileInfo::exists(installedPath));
        QFile installed(installedPath);
        QVERIFY(installed.open(QIODevice::ReadOnly));
        QCOMPARE(installed.readAll(), QByteArray("fake-zith-lsp\n"));
        QVERIFY(QFileInfo(installedPath).isExecutable());
    }

    void testZithRuntimeAssetDownloaderRejectsIncompleteRequest() {
        ZithRuntimeAssetDownloader downloader;
        QSignalSpy failureSpy(&downloader,
                              &ZithRuntimeAssetDownloader::failed);

        downloader.start({});

        QCOMPARE(failureSpy.count(), 1);
        QCOMPARE(failureSpy.at(0).at(0).toString(),
                 QStringLiteral(
                     "Zith runtime asset download request was incomplete."));
        QVERIFY(!downloader.isActive());
    }

    void testZithRuntimeOverrideResolverClassifiesConfiguration() {
        const auto notConfigured =
            ZithRuntimeOverrideResolver::resolve({}, {});
        QCOMPARE(notConfigured.action,
                 ZithRuntimeOverrideResolver::Action::NotConfigured);

        const auto partial = ZithRuntimeOverrideResolver::resolve(
            QStringLiteral("/tmp/zith-lsp"), {});
        QCOMPARE(partial.action,
                 ZithRuntimeOverrideResolver::Action::IgnoreAndContinue);
        QVERIFY(partial.message.contains(QStringLiteral("partial")));
    }

    void testZithRuntimeOverrideResolverRejectsInvalidPaths() {
        const auto invalidLsp = ZithRuntimeOverrideResolver::resolve(
            QStringLiteral("/missing/zith-lsp"),
            QStringLiteral("/tmp"));
        QCOMPARE(invalidLsp.action, ZithRuntimeOverrideResolver::Action::Fail);
        QVERIFY(invalidLsp.message.contains(QStringLiteral("LSP_PATH")));

        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString lspPath =
            QDir(tempDir.path()).filePath(QStringLiteral("zith-lsp"));
        QFile lsp(lspPath);
        QVERIFY(lsp.open(QIODevice::WriteOnly));
        lsp.write("#!/bin/sh\nexit 0\n");
        lsp.close();
        QVERIFY(QFile::setPermissions(
            lspPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                         QFileDevice::ExeOwner));

        const auto invalidStdlib = ZithRuntimeOverrideResolver::resolve(
            lspPath,
            QDir(tempDir.path()).filePath(QStringLiteral("missing-stdlib")));
        QCOMPARE(invalidStdlib.action,
                 ZithRuntimeOverrideResolver::Action::Fail);
        QVERIFY(invalidStdlib.message.contains(QStringLiteral("STDLIB_PATH")));
    }

    void testZithRuntimeOverrideResolverResolvesValidPaths() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString lspPath =
            QDir(tempDir.path()).filePath(QStringLiteral("zith-lsp"));
        const QString stdlibPath =
            QDir(tempDir.path()).filePath(QStringLiteral("stdlib"));
        QFile lsp(lspPath);
        QVERIFY(lsp.open(QIODevice::WriteOnly));
        lsp.write("#!/bin/sh\nexit 0\n");
        lsp.close();
        QVERIFY(QFile::setPermissions(
            lspPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                         QFileDevice::ExeOwner));
        QVERIFY(QDir().mkpath(stdlibPath));

        const auto resolved =
            ZithRuntimeOverrideResolver::resolve(lspPath, stdlibPath);
        QCOMPARE(resolved.action, ZithRuntimeOverrideResolver::Action::Use);
        QCOMPARE(resolved.runtime.lspPath, lspPath);
        QCOMPARE(resolved.runtime.stdlibPath, stdlibPath);
    }

    void testShortcutCatalogProvidesSharedDefinitions() {
        const QList<ShortcutCategory> categories = ShortcutCatalog::categories();
        QCOMPARE(categories.size(), 4);
        QCOMPARE(categories.at(0).translationKey,
                 QStringLiteral("shortcut.cat.file"));
        QCOMPARE(categories.at(0).entries.first().translationKey,
                 QStringLiteral("shortcut.new_file"));
        QCOMPARE(categories.at(0).entries.first().keySequence,
                 QStringLiteral("Ctrl+N"));

        int entryCount = 0;
        for (const ShortcutCategory &category : categories)
            entryCount += category.entries.size();
        QCOMPARE(entryCount, 28);
    }

    void testShortcutTreePresenterProjectsCatalogAndTranslations() {
        QTreeWidget tree;
        tree.setColumnCount(2);

        ShortcutTreePresenter::populate(&tree);

        const QList<ShortcutCategory> categories = ShortcutCatalog::categories();
        QCOMPARE(tree.topLevelItemCount(), categories.size());
        QCOMPARE(tree.topLevelItem(0)->data(0, Qt::UserRole).toString(),
                 categories.first().translationKey);
        QCOMPARE(tree.topLevelItem(0)->childCount(),
                 categories.first().entries.size());
        QCOMPARE(tree.topLevelItem(0)->child(0)->data(1, Qt::UserRole)
                     .toString(),
                 QStringLiteral("Ctrl+N"));

        ShortcutTreePresenter::refreshTranslations(&tree);
        QCOMPARE(tree.headerItem()->text(0),
                 TranslationManager::instance().translate(
                     "shortcut.header_action"));
        QCOMPARE(tree.topLevelItem(0)->text(0),
                 TranslationManager::instance().translate(
                     "shortcut.cat.file"));
        QCOMPARE(tree.topLevelItem(0)->child(0)->text(0),
                 TranslationManager::instance().translate(
                     "shortcut.new_file"));
        QCOMPARE(tree.topLevelItem(0)->child(0)->text(1),
                 QStringLiteral("Ctrl+N"));
    }

    void testLspServerCapabilitiesDecode() {
        const LspServerCapabilities capabilities =
            LspServerCapabilities::fromJson(QJsonObject{
                {"completionProvider", QJsonObject{}},
                {"hoverProvider", true},
                {"signatureHelpProvider", true},
                {"definitionProvider", false},
                {"textDocumentSync", QJsonObject{{"change", 2}}}});

        QVERIFY(capabilities.completionProvider);
        QVERIFY(capabilities.hoverProvider);
        QVERIFY(capabilities.signatureHelpProvider);
        QVERIFY(!capabilities.definitionProvider);
        QCOMPARE(capabilities.documentSyncKind, 2);
        QVERIFY(!capabilities.renameProvider);
    }

    void testLspDocumentRegistryRejectsStaleResults() {
        LspDocumentRegistry registry;
        const QString uri = QStringLiteral("file:///workspace/main.zith");

        registry.open(uri, 3);
        QCOMPARE(registry.version(uri), 3);
        QVERIFY(registry.isCurrent(uri, 3));
        QVERIFY(!registry.isCurrent(uri, 2));
        QVERIFY(registry.isCurrent(uri, -1));

        registry.update(uri, 4);
        QVERIFY(!registry.isCurrent(uri, 3));
        QVERIFY(registry.isCurrent(uri, 4));

        registry.close(uri);
        QVERIFY(!registry.isCurrent(uri, 4));
        QCOMPARE(registry.version(uri), 1);
    }

    void testLspProcessTransportReadsFramedMessage() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QByteArray payload =
            R"({"jsonrpc":"2.0","method":"transport/test","params":{}})";
        const QString serverPath =
            QDir(tempDir.path()).filePath("transport-server");
        QFile server(serverPath);
        QVERIFY(server.open(QIODevice::WriteOnly));
        server.write("#!/bin/sh\nprintf 'Content-Length: " +
                     QByteArray::number(payload.size()) +
                     "\\r\\n\\r\\n" + payload + "'\n");
        server.close();
        QVERIFY(QFile::setPermissions(
            serverPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                            QFileDevice::ExeOwner));

        LspProcessTransport transport;
        QSignalSpy messageSpy(
            &transport, &LspProcessTransport::messageReceived);
        QSignalSpy finishedSpy(&transport, &LspProcessTransport::finished);

        QVERIFY(transport.start(serverPath));
        QTRY_COMPARE_WITH_TIMEOUT(messageSpy.count(), 1, 5000);
        QCOMPARE(messageSpy.at(0).at(0).toJsonObject().value("method"),
                 QJsonValue(QStringLiteral("transport/test")));
        QTRY_COMPARE_WITH_TIMEOUT(finishedSpy.count(), 1, 5000);
        const LspProcessResult result =
            qvariant_cast<LspProcessResult>(finishedSpy.at(0).at(0));
        QCOMPARE(result.exitCode, 0);
        QVERIFY(!result.crashed);
    }

    void testBracketMatcher() {
        const BracketMatch opening =
            BracketMatcher::find(QStringLiteral("([value])"), 0);
        QVERIFY(opening.isValid());
        QCOMPARE(opening.bracketPosition, 0);
        QCOMPARE(opening.matchingPosition, 8);

        const BracketMatch closing =
            BracketMatcher::find(QStringLiteral("([value])"), 8);
        QVERIFY(closing.isValid());
        QCOMPARE(closing.bracketPosition, 7);
        QCOMPARE(closing.matchingPosition, 1);

        QVERIFY(!BracketMatcher::find(QStringLiteral("(value"), 0).isValid());
        QVERIFY(!BracketMatcher::find(QStringLiteral("value"), 3).isValid());
    }

    void testEditorTypingPolicy() {
        QCOMPARE(EditorTypingPolicy::indentationForLine(
                     QStringLiteral("  if (ready) {")),
                 QStringLiteral("      "));
        QCOMPARE(EditorTypingPolicy::indentationForLine(
                     QStringLiteral("\treturn value;")),
                 QStringLiteral("\t"));
        QVERIFY(EditorTypingPolicy::isAutoCloseCharacter('('));
        QVERIFY(!EditorTypingPolicy::isAutoCloseCharacter('x'));

        const AutoCloseDecision insert =
            EditorTypingPolicy::autoCloseDecision('(', {}, {}, false);
        QCOMPARE(insert.action, AutoCloseDecision::Action::InsertPair);
        QCOMPARE(insert.closing, QChar(')'));

        const AutoCloseDecision jump =
            EditorTypingPolicy::autoCloseDecision('(', {}, ')', false);
        QCOMPARE(jump.action, AutoCloseDecision::Action::JumpOver);

        const AutoCloseDecision quoteInsideWord =
            EditorTypingPolicy::autoCloseDecision(
                '"', 'a', {}, false);
        QCOMPARE(quoteInsideWord.action, AutoCloseDecision::Action::Ignore);

        const AutoCloseDecision surround =
            EditorTypingPolicy::autoCloseDecision('(', 'a', 'b', true);
        QCOMPARE(surround.action,
                 AutoCloseDecision::Action::SurroundSelection);
    }

    void testEditorDiagnosticHighlighterClampsProtocolRanges() {
        QTextDocument document;
        document.setPlainText(QStringLiteral("first line\nsecond"));

        LspDiagnostic diagnostic;
        diagnostic.range.start = {-4, -2};
        diagnostic.range.end = {99, 99};
        diagnostic.severity = 1;
        const QList<LspDiagnostic> diagnostics{diagnostic};
        const QList<QTextEdit::ExtraSelection> selections =
            EditorDiagnosticHighlighter::selections(
                document, diagnostics,
                [](int) { return QColor(QStringLiteral("#ff0000")); });

        QCOMPARE(selections.size(), 1);
        QVERIFY(selections.first().cursor.selectionStart() >= 0);
        QVERIFY(selections.first().cursor.selectionEnd() <=
                document.characterCount());
        QCOMPARE(selections.first().format.underlineColor(),
                 QColor(QStringLiteral("#ff0000")));
    }

    void testEditorTextEditApplierOrdersAndClampsEdits() {
        QTextDocument document;
        document.setPlainText(QStringLiteral("abc\ndef"));

        QCOMPARE(EditorTextEditApplier::offsetForLspPosition(
                     document, {-10, -4}),
                 0);
        QCOMPARE(EditorTextEditApplier::offsetForLspPosition(
                     document, {99, 99}),
                 document.toPlainText().size());

        const QList<EditorTextEditApplier::TextEdit> edits = {
            {{{0, 1}, {0, 2}}, QStringLiteral("B")},
            {{{1, 1}, {1, 3}}, QStringLiteral("EF")}};
        EditorTextEditApplier::apply(document, edits);

        QCOMPARE(document.toPlainText(), QStringLiteral("aBc\ndEF"));
    }

    void testEditorCompletionInsertionPolicyReplacesCurrentWord() {
        const auto decision = EditorCompletionInsertionPolicy::prepare(
            QStringLiteral("pri"), 3, QStringLiteral("print"), 1);

        QVERIFY(decision.valid);
        QCOMPARE(decision.start, 0);
        QCOMPARE(decision.end, 3);
        QCOMPARE(decision.text, QStringLiteral("print"));
    }

    void testEditorCompletionInsertionPolicyPreservesPrefixBoundary() {
        const auto decision = EditorCompletionInsertionPolicy::prepare(
            QStringLiteral("value.pri"), 9, QStringLiteral("print"), 1);

        QVERIFY(decision.valid);
        QCOMPARE(decision.start, 6);
        QCOMPARE(decision.end, 9);
    }

    void testEditorCompletionInsertionPolicyExpandsSnippetTabstops() {
        const auto decision = EditorCompletionInsertionPolicy::prepare(
            QStringLiteral("fn"), 2, QStringLiteral("${1:name}($0)"), 2);

        QVERIFY(decision.valid);
        QCOMPARE(decision.start, 0);
        QCOMPARE(decision.end, 2);
        QCOMPARE(decision.text, QStringLiteral("name()"));
    }

    void testEditorCompletionInsertionPolicyRejectsInvalidCursor() {
        const auto decision = EditorCompletionInsertionPolicy::prepare(
            QStringLiteral("text"), 5, QStringLiteral("x"), 1);

        QVERIFY(!decision.valid);
    }

    void testLspClientParsesWorkDoneProgress() {
        LspClient client;
        QSignalSpy progressSpy(&client, &LspClient::workDoneProgressReceived);

        client.dispatchMessage(QJsonObject{
            {"jsonrpc", "2.0"},
            {"method", "$/progress"},
            {"params", QJsonObject{
                {"token", "zith-build-1"},
                {"value", QJsonObject{
                    {"kind", "begin"},
                    {"title", "Zith zith.build"},
                    {"cancellable", false}
                }}
            }}
        });
        client.dispatchMessage(QJsonObject{
            {"jsonrpc", "2.0"},
            {"method", "$/progress"},
            {"params", QJsonObject{
                {"token", "zith-build-1"},
                {"value", QJsonObject{
                    {"kind", "report"},
                    {"message", "Compiling"}
                }}
            }}
        });
        client.dispatchMessage(QJsonObject{
            {"jsonrpc", "2.0"},
            {"method", "$/progress"},
            {"params", QJsonObject{
                {"token", "zith-build-1"},
                {"value", QJsonObject{
                    {"kind", "end"},
                    {"message", "Finished"}
                }}
            }}
        });

        QCOMPARE(progressSpy.count(), 3);
        QCOMPARE(progressSpy.at(0).at(0).toString(),
                 QStringLiteral("zith-build-1"));
        QCOMPARE(progressSpy.at(0).at(1).toString(),
                 QStringLiteral("begin"));
        QCOMPARE(progressSpy.at(1).at(1).toString(),
                 QStringLiteral("report"));
        QCOMPARE(progressSpy.at(1).at(2).toString(),
                 QStringLiteral("Compiling"));
        QCOMPARE(progressSpy.at(2).at(1).toString(),
                 QStringLiteral("end"));
        QCOMPARE(progressSpy.at(2).at(2).toString(),
                 QStringLiteral("Finished"));
    }

    void testLspServerMessageDispatcher() {
        QList<QJsonObject> sentMessages;
        LspServerMessageDispatcher dispatcher(
            {[&sentMessages](const QJsonObject &message) {
                 sentMessages.append(message);
                 return true;
             },
             []() { return true; },
             [](const QString &uri, int version) {
                 return uri == QStringLiteral("file:///current.cpp") &&
                        version == 3;
             }});
        QSignalSpy diagnosticsSpy(
            &dispatcher, &LspServerMessageDispatcher::diagnosticsReceived);
        QSignalSpy progressSpy(
            &dispatcher,
            &LspServerMessageDispatcher::workDoneProgressReceived);
        QSignalSpy showMessageSpy(
            &dispatcher, &LspServerMessageDispatcher::showMessage);

        dispatcher.dispatch(
            {{"jsonrpc", "2.0"},
             {"id", 7},
             {"method", "window/workDoneProgress/create"},
             {"params", QJsonObject{{"token", "build"}}}});
        QCOMPARE(sentMessages.size(), 1);
        QCOMPARE(sentMessages.first().value("id").toInt(), 7);
        QVERIFY(sentMessages.first().contains("result"));

        dispatcher.dispatch(
            {{"jsonrpc", "2.0"},
             {"method", "textDocument/publishDiagnostics"},
             {"params",
              QJsonObject{
                  {"uri", "file:///stale.cpp"},
                  {"version", 2},
                  {"diagnostics", QJsonArray{}}}}});
        QCOMPARE(diagnosticsSpy.count(), 0);

        dispatcher.dispatch(
            {{"jsonrpc", "2.0"},
             {"method", "textDocument/publishDiagnostics"},
             {"params",
              QJsonObject{
                  {"uri", "file:///current.cpp"},
                  {"version", 3},
                  {"diagnostics",
                   QJsonArray{QJsonObject{
                       {"range",
                        QJsonObject{
                            {"start", QJsonObject{{"line", 1},
                                                  {"character", 2}}},
                            {"end", QJsonObject{{"line", 1},
                                                {"character", 5}}}}},
                       {"message", "unused value"}}}}}}});
        QCOMPARE(diagnosticsSpy.count(), 1);
        QCOMPARE(diagnosticsSpy.at(0).at(0).toString(),
                 QStringLiteral("file:///current.cpp"));
        QCOMPARE(diagnosticsSpy.at(0).at(2).value<QList<LspDiagnostic>>().size(),
                 1);

        dispatcher.dispatch(
            {{"jsonrpc", "2.0"},
             {"method", "window/showMessage"},
             {"params", QJsonObject{{"message", "Build finished"}}}});
        QCOMPARE(showMessageSpy.count(), 1);
        QCOMPARE(showMessageSpy.at(0).at(0).toString(),
                 QStringLiteral("Build finished"));

        dispatcher.dispatch(
            {{"jsonrpc", "2.0"},
             {"method", "$/progress"},
             {"params",
              QJsonObject{
                  {"token", "build"},
                  {"value",
                   QJsonObject{{"kind", "report"},
                                {"message", "Compiling"}}}}}});
        QCOMPARE(progressSpy.count(), 1);
        QCOMPARE(progressSpy.at(0).at(1).toString(),
                 QStringLiteral("report"));
    }

    void testLspRequestSender() {
        QList<QJsonObject> messages;
        QList<QJsonObject> responses;
        QStringList logMessages;
        LspRequestSender sender({
            [&messages](const QJsonObject &message) {
                messages.append(message);
                return true;
            },
            []() { return true; },
            [&logMessages](const QString &message) {
                logMessages.append(message);
            },
        });

        QCOMPARE(sender.send("textDocument/hover", {}, "file:///main.zith", 3,
                             true, false, {}),
                 qint64(-1));
        QCOMPARE(messages.size(), 0);

        const qint64 first = sender.send(
            "textDocument/hover", {}, "file:///main.zith", 3, true, true, {});
        QVERIFY(first > 0);
        QCOMPARE(messages.size(), 1);
        QCOMPARE(messages.at(0).value("method").toString(),
                 QStringLiteral("textDocument/hover"));
        QCOMPARE(messages.at(0).value("id").toVariant().toLongLong(), first);

        const qint64 second = sender.send(
            "textDocument/hover", {}, "file:///main.zith", 4, true, true,
            [&responses](const QJsonObject &response) {
                responses.append(response);
            });
        QVERIFY(second > first);
        QCOMPARE(messages.size(), 3);
        QCOMPARE(messages.at(1).value("method").toString(),
                 QStringLiteral("$/cancelRequest"));
        QCOMPARE(messages.at(1).value("params")
                     .toObject()
                     .value("id")
                     .toVariant()
                     .toLongLong(),
                 first);
        QCOMPARE(messages.at(2).value("id").toVariant().toLongLong(), second);

        sender.handleResponse(
            {{"jsonrpc", "2.0"},
             {"id", second},
             {"result", QJsonObject{{"ok", true}}}},
            [](const QString &, int version) { return version == 4; });
        QCOMPARE(responses.size(), 1);
        QVERIFY(responses.first().contains("result"));

        const qint64 stale = sender.send(
            "textDocument/hover", {}, "file:///main.zith", 5, true, true,
            [&responses](const QJsonObject &response) {
                responses.append(response);
            });
        sender.handleResponse(
            {{"jsonrpc", "2.0"},
             {"id", stale},
             {"result", QJsonObject{{"stale", true}}}},
            [](const QString &, int) { return false; });
        QCOMPARE(responses.size(), 1);

        const qint64 failed = sender.send(
            "textDocument/hover", {}, "file:///main.zith", 6, false, true,
            {});
        sender.handleResponse(
            {{"jsonrpc", "2.0"},
             {"id", failed},
             {"error", QJsonObject{{"code", -32000},
                                    {"message", "server unavailable"}}}},
            [](const QString &, int) { return true; });
        QCOMPARE(logMessages.size(), 1);
        QVERIFY(logMessages.first().contains("server unavailable"));
    }

    void testLspFeatureRequestRouterUsesCategorizedProtocolSeams() {
        QList<QString> methods;
        QList<QJsonObject> parameters;
        QList<std::function<void(const QJsonObject &)>> responses;
        LspFeatureRequestRouter router(
            {[&](const QString &method, const QJsonObject &params,
                 const QString &, int, bool,
                 std::function<void(const QJsonObject &)> callback) {
                methods.append(method);
                parameters.append(params);
                responses.append(std::move(callback));
                return static_cast<qint64>(methods.size());
            }});

        int definitionCount = 0;
        QList<LspLocation> definitions;
        QObject::connect(
            &router, &LspFeatureRequestRouter::definitionResult,
            [&definitionCount, &definitions](
                const QString &, int, const QList<LspLocation> &locations) {
                ++definitionCount;
                definitions = locations;
            });

        const QString uri = QStringLiteral("file:///main.cpp");
        router.requestPosition(
            LspFeatureRequestRouter::PositionFeature::Definition, uri, 7,
            {2, 4});
        QCOMPARE(methods, QList<QString>{QStringLiteral("textDocument/definition")});
        QCOMPARE(parameters.first().value("position")
                     .toObject()
                     .value("line")
                     .toInt(),
                 2);

        responses.last()(
            {{"result",
              QJsonObject{
                  {"uri", "file:///other.cpp"},
                  {"range",
                   QJsonObject{
                       {"start", QJsonObject{{"line", 8}, {"character", 1}}},
                       {"end", QJsonObject{{"line", 8}, {"character", 5}}}}}}}});
        QCOMPARE(definitionCount, 1);
        QCOMPARE(definitions.size(), 1);
        QCOMPARE(definitions.first().uri, QStringLiteral("file:///other.cpp"));
        QCOMPARE(definitions.first().range.start.line, 8);

        router.requestPosition(
            LspFeatureRequestRouter::PositionFeature::Definition, uri, 7,
            {2, 4});
        const QJsonArray multipleDefinitions = {
            QJsonObject{
                {"uri", "file:///first.cpp"},
                {"range",
                 QJsonObject{
                     {"start", QJsonObject{{"line", 1}, {"character", 2}}},
                     {"end", QJsonObject{{"line", 1}, {"character", 8}}}}}},
            QJsonObject{
                {"uri", "file:///second.cpp"},
                {"range",
                 QJsonObject{
                     {"start", QJsonObject{{"line", 9}, {"character", 3}}},
                     {"end", QJsonObject{{"line", 9}, {"character", 7}}}}}}};
        responses.last()(
            QJsonObject{{"result", multipleDefinitions}});
        QCOMPARE(definitionCount, 2);
        QCOMPARE(definitions.size(), 2);
        QCOMPARE(definitions.at(0).uri, QStringLiteral("file:///first.cpp"));
        QCOMPARE(definitions.at(1).uri, QStringLiteral("file:///second.cpp"));

        router.requestPosition(
            LspFeatureRequestRouter::PositionFeature::Definition, uri, 7,
            {2, 4});
        responses.last()({{"result", QJsonValue()}});
        QCOMPARE(definitionCount, 3);
        QVERIFY(definitions.isEmpty());

        router.requestDocument(
            LspFeatureRequestRouter::DocumentFeature::Formatting, uri, 7);
        QCOMPARE(methods.last(), QStringLiteral("textDocument/formatting"));
        QCOMPARE(parameters.last().value("options")
                     .toObject()
                     .value("tabSize")
                     .toInt(),
                 4);
        QVERIFY(!parameters.last().value("options")
                     .toObject()
                     .value("insertSpaces")
                     .isUndefined());
    }

    void testLspClientRepliesToWorkDoneProgressCreate() {
        LspClient client;
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString serverPath = QDir(tempDir.path()).filePath("lsp-server");
        QFile serverFile(serverPath);
        QVERIFY(serverFile.open(QIODevice::WriteOnly));
        serverFile.write("#!/bin/sh\nexit 0\n");
        serverFile.close();
        QVERIFY(QFile::setPermissions(
            serverPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                            QFileDevice::ExeOwner));

        client.start(serverPath);
        QTRY_VERIFY(client.isRunning());

        client.dispatchMessage(QJsonObject{
            {"jsonrpc", "2.0"},
            {"id", "wdp-1"},
            {"method", "window/workDoneProgress/create"},
            {"params", QJsonObject{{"token", "zith-build-1"}}}
        });

        client.stop();
        client.waitForFinishedForTesting(5000);
    }

    void testLspClientReturnsDocumentSymbolsAndOutlineRenders() {
        const QString serverPath =
            qEnvironmentVariable("HELIOS_ZITH_LSP_PATH");
        if (serverPath.isEmpty())
            QSKIP("Set HELIOS_ZITH_LSP_PATH to run the real zith-lsp test");
        QVERIFY2(QFileInfo(serverPath).isExecutable(),
                 qPrintable(QStringLiteral("Invalid HELIOS_ZITH_LSP_PATH: %1")
                                .arg(serverPath)));

        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString sourcePath = QDir(tempDir.path()).filePath("symbols.zith");
        const QByteArray source =
            "struct MyStruct {\n"
            "  field_a: i32,\n"
            "}\n"
            "fn my_func(): i32 { return 1; }\n";
        QFile sourceFile(sourcePath);
        QVERIFY(sourceFile.open(QIODevice::WriteOnly | QIODevice::Text));
        sourceFile.write(source);
        sourceFile.close();

        LspClient client;
        QSignalSpy initializedSpy(&client, &LspClient::initialized);
        QSignalSpy symbolsSpy(&client, &LspClient::documentSymbolsResult);
        QVERIFY(client.start(serverPath, {}, QDir(tempDir.path()).path()));
        QTRY_VERIFY_WITH_TIMEOUT(initializedSpy.count() > 0, 15000);
        QVERIFY(client.supports(LspClient::Capability::DocumentSymbol));

        const QString uri = QUrl::fromLocalFile(sourcePath).toString();
        client.openDocument(uri, "zith", QString::fromUtf8(source));
        client.requestDocumentSymbols(uri, 1);
        QTRY_VERIFY_WITH_TIMEOUT(symbolsSpy.count() > 0, 15000);

        const QJsonArray symbols = symbolsSpy.at(0).at(2).toJsonArray();
        QVERIFY(symbols.size() >= 2);
        QStringList labels;
        for (const QJsonValue &value : symbols)
            labels << value.toObject().value("name").toString();
        QVERIFY(labels.contains(QStringLiteral("MyStruct")));
        QVERIFY(labels.contains(QStringLiteral("my_func")));

        OutlinePanel panel;
        panel.setSymbols(symbols);
        auto *tree = panel.findChild<QTreeWidget *>();
        QVERIFY(tree);
        QVERIFY(tree->topLevelItemCount() >= 2);

        client.stop();
        client.waitForFinishedForTesting(5000);
    }

    void testLspClientExecutesWorkspaceCommand() {
        const QString serverPath =
            QStringLiteral("/home/diogo/zith-lsp/build/zith-lsp");
        if (!QFileInfo::exists(serverPath))
            QSKIP("zith-lsp build not available");

        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        LspClient client;
        QSignalSpy initializedSpy(&client, &LspClient::initialized);
        QSignalSpy commandSpy(&client, &LspClient::commandResult);
        QVERIFY(client.start(serverPath, {}, QDir(tempDir.path()).path()));
        QTRY_VERIFY_WITH_TIMEOUT(initializedSpy.count() > 0, 15000);
        QVERIFY(client.supports(LspClient::Capability::ExecuteCommand));

        CompilerPanel compilerPanel;
        LspClientEventSource events(&client);
        CodeEditor activeEditor;
        activeEditor.setFilePath(
            QDir(tempDir.path()).filePath(QStringLiteral("main.zith")));
        bool lspEnabled = true;
        WorkspaceCommandController::Callbacks commandCallbacks;
        commandCallbacks.lspEnabled = [&]() { return lspEnabled; };
        commandCallbacks.workspaceRoot = [&]() { return tempDir.path(); };
        commandCallbacks.currentEditor = [&]() { return &activeEditor; };
        commandCallbacks.isZithEditor = [](CodeEditor *) { return true; };
        WorkspaceCommandController::Dependencies dependencies;
        dependencies.client = &client;
        dependencies.events = &events;
        dependencies.compilerPanel = &compilerPanel;
        WorkspaceCommandController commandController(
            std::move(dependencies), std::move(commandCallbacks));
        const WorkspaceCommandState initialState = commandController.state();
        QVERIFY(initialState.canExecuteWorkspaceCommand);
        QVERIFY(initialState.activeZithEditor);
        QVERIFY(initialState.hasCurrentFile);
        QVERIFY(initialState.hasWorkspaceRoot);
        QVERIFY(!initialState.taskRunning);
        QCOMPARE(initialState.workspaceRoot, tempDir.path());
        QVERIFY(commandController.canExecuteWorkspaceCommand());
        QVERIFY(commandController.canExecute(ShellCommand::Build));
        QVERIFY(commandController.canExecute(ShellCommand::CheckFile));
        QVERIFY(commandController.canExecute(ShellCommand::Run));
        QVERIFY(!commandController.canExecute(ShellCommand::Stop));
        commandController.handleCommandResult(
            QStringLiteral("zith.run"), true,
            QJsonObject{{QStringLiteral("taskId"), QStringLiteral("task-1")}});
        const WorkspaceCommandState runningState = commandController.state();
        QVERIFY(runningState.taskRunning);
        QCOMPARE(runningState.runningTaskId, QStringLiteral("task-1"));
        QVERIFY(!commandController.canExecute(ShellCommand::Run));
        QVERIFY(commandController.canExecute(ShellCommand::Stop));
        emit client.processExitReceived(QStringLiteral("task-1"), 0);
        QVERIFY(!commandController.state().taskRunning);
        lspEnabled = false;
        QVERIFY(!commandController.canExecuteWorkspaceCommand());
        QVERIFY(!commandController.canExecute(ShellCommand::Build));
        lspEnabled = true;

        const QString projectName = QStringLiteral("cmd_proj");
        client.executeWorkspaceCommand(
            QStringLiteral("zith.new"),
            QJsonArray{projectName});
        QTRY_VERIFY_WITH_TIMEOUT(commandSpy.count() > 0, 15000);
        QCOMPARE(commandSpy.at(0).at(0).toString(),
                 QStringLiteral("zith.new"));
        QCOMPARE(commandSpy.at(0).at(1).toBool(), true);

        const QJsonObject result =
            QJsonValue::fromVariant(commandSpy.at(0).at(2)).toObject();
        QVERIFY(result.value(QStringLiteral("success")).toBool());
        const QString rootUri =
            result.value(QStringLiteral("rootUri")).toString();
        QVERIFY(rootUri.startsWith(QStringLiteral("file://")));
        const QString rootDir =
            QUrl(rootUri).toLocalFile();
        QVERIFY(QFileInfo::exists(
            QDir(rootDir).filePath(QStringLiteral("ZithProject.toml"))));
        QVERIFY(QFileInfo::exists(
            QDir(rootDir).filePath(QStringLiteral("src/main.zith"))));

        commandSpy.clear();
        client.executeWorkspaceCommand(
            QStringLiteral("zith.check"),
            QJsonArray{QUrl::fromLocalFile(
                           QDir(rootDir).filePath(QStringLiteral("src/main.zith")))
                           .toString()});
        QTRY_VERIFY_WITH_TIMEOUT(commandSpy.count() > 0, 30000);
        QCOMPARE(commandSpy.at(0).at(0).toString(),
                 QStringLiteral("zith.check"));
        QCOMPARE(commandSpy.at(0).at(1).toBool(), true);
        QVERIFY(QJsonValue::fromVariant(commandSpy.at(0).at(2))
                    .toObject()
                    .value(QStringLiteral("success"))
                    .toBool());

        client.stop();
        client.waitForFinishedForTesting(5000);
    }

    void testWorkspaceTaskOutputControllerProjectsProgressAndDiagnostics() {
        LspClient client;
        LspClientEventSource events(&client);
        CompilerPanel panel;
        WorkspaceTaskOutputController controller(
            &events, &panel, WorkspaceTaskOutputController::StateChanged{});
        controller.beginCommand();
        panel.startBuild(QStringLiteral("Build project /tmp/demo"));

        controller.handleWorkDoneProgress(QStringLiteral("token-1"),
                                          QStringLiteral("begin"), {});
        controller.handleWorkDoneProgress(QStringLiteral("token-1"),
                                          QStringLiteral("report"),
                                          QStringLiteral("Compiling"));
        controller.handleWorkDoneProgress(QStringLiteral("old-token"),
                                          QStringLiteral("end"),
                                          QStringLiteral("Finished"));
        QVERIFY(panel.outputText().contains(QStringLiteral("Compiling...")));

        controller.handleWorkDoneProgress(QStringLiteral("token-1"),
                                          QStringLiteral("end"),
                                          QStringLiteral("Finished"));
        panel.appendDiagnostics(QList<LspDiagnostic>{
            {{ {2, 3}, {2, 7} }, 1, QStringLiteral("type error"), QStringLiteral("zithc")}});

        const QString text = panel.outputText();
        QVERIFY(text.contains(QStringLiteral("Finished")));
        QVERIFY(text.contains(QStringLiteral("  line 3, col 4: type error")));
        QVERIFY(!text.contains(QStringLiteral("old-token")));
    }

    void testWorkspaceTaskOutputControllerIgnoresForeignProgress() {
        LspClient client;
        LspClientEventSource events(&client);
        CompilerPanel panel;
        WorkspaceTaskOutputController controller(
            &events, &panel, WorkspaceTaskOutputController::StateChanged{});
        controller.beginCommand();
        panel.startBuild(QStringLiteral("Build /tmp/other"));
        controller.handleWorkDoneProgress(QStringLiteral("current"),
                                          QStringLiteral("begin"), {});
        const int compilingMessages =
            panel.outputText().count(QStringLiteral("Compiling..."));
        const int finishedMessages =
            panel.outputText().count(QStringLiteral("Finished"));

        controller.handleWorkDoneProgress(QStringLiteral("stale"),
                                          QStringLiteral("begin"), {});
        controller.handleWorkDoneProgress(QStringLiteral("stale"),
                                          QStringLiteral("end"), {});

        QCOMPARE(panel.outputText().count(QStringLiteral("Compiling...")),
                 compilingMessages);
        QCOMPARE(panel.outputText().count(QStringLiteral("Finished")),
                 finishedMessages);
    }

    void testWorkspaceTaskOutputControllerOwnsProgressTokenLifecycle() {
        LspClient client;
        LspClientEventSource events(&client);
        CompilerPanel panel;
        WorkspaceTaskOutputController controller(
            &events, &panel, WorkspaceTaskOutputController::StateChanged{});
        controller.beginCommand();
        panel.startBuild(QStringLiteral("Build"));

        controller.handleWorkDoneProgress(QStringLiteral("token-1"),
                                          QStringLiteral("begin"), {});

        controller.handleWorkDoneProgress(QStringLiteral("token-2"),
                                          QStringLiteral("report"),
                                          QStringLiteral("stale"));
        QVERIFY(!panel.outputText().contains(QStringLiteral("stale")));

        controller.handleWorkDoneProgress(QStringLiteral("token-1"),
                                          QStringLiteral("end"),
                                          QStringLiteral("Finished"));
        QVERIFY(panel.outputText().contains(QStringLiteral("Finished")));
    }

    void testRunOutputCollectorBuffersUntilTaskKnown() {
        RunOutputCollector collector;
        collector.buffer(QStringLiteral("task-1"), QStringLiteral("hello"));
        collector.buffer(QStringLiteral("task-2"), QStringLiteral("other"));
        collector.buffer(QStringLiteral("task-1"), QStringLiteral(" world"));

        const QStringList task1 = collector.takeFor(QStringLiteral("task-1"));
        QCOMPARE(task1, QStringList({QStringLiteral("hello"),
                                     QStringLiteral(" world")}));
        QVERIFY(collector.takeFor(QStringLiteral("task-1")).isEmpty());
        QVERIFY(!collector.takeExitFor(QStringLiteral("task-1")).has_value());

        const QStringList task2 = collector.takeFor(QStringLiteral("task-2"));
        QCOMPARE(task2, QStringList({QStringLiteral("other")}));

        RunOutputCollector earlyExitCollector;
        earlyExitCollector.buffer(QStringLiteral("task-3"),
                                  QStringLiteral("done\n"));
        earlyExitCollector.bufferExit(QStringLiteral("task-3"), 0);
        QCOMPARE(earlyExitCollector.takeFor(QStringLiteral("task-3")),
                 QStringList({QStringLiteral("done\n")}));
        const auto earlyExit =
            earlyExitCollector.takeExitFor(QStringLiteral("task-3"));
        QVERIFY(earlyExit.has_value());
        QCOMPARE(*earlyExit, 0);
        QVERIFY(!earlyExitCollector.takeExitFor(QStringLiteral("task-3"))
                     .has_value());
    }

    void testRunOutputCollectorDiscardKeepsUnrelatedTasks() {
        RunOutputCollector collector;
        collector.buffer(QStringLiteral("stale"), QStringLiteral("old\n"));
        collector.bufferExit(QStringLiteral("stale"), 1);
        collector.buffer(QStringLiteral("current"), QStringLiteral("live\n"));

        collector.discardFor(QStringLiteral("stale"));

        QVERIFY(collector.takeFor(QStringLiteral("stale")).isEmpty());
        QVERIFY(!collector.takeExitFor(QStringLiteral("stale")).has_value());
        QCOMPARE(collector.takeFor(QStringLiteral("current")),
                 QStringList({QStringLiteral("live\n")}));
    }

    void testRunOutputCollectorEarlyOutputAndExitsCoexistByTask() {
        RunOutputCollector collector;
        collector.buffer(QStringLiteral("task-b"),
                         QStringLiteral("second output\n"));
        collector.bufferExit(QStringLiteral("task-b"), 1);
        collector.buffer(QStringLiteral("task-a"),
                         QStringLiteral("first output\n"));
        collector.bufferExit(QStringLiteral("task-a"), 0);
        collector.bufferExit(QStringLiteral("task-a"), 2);

        QCOMPARE(collector.takeFor(QStringLiteral("task-a")),
                 QStringList({QStringLiteral("first output\n")}));
        const auto firstExit = collector.takeExitFor(QStringLiteral("task-a"));
        QVERIFY(firstExit.has_value());
        QCOMPARE(*firstExit, 0);
        const auto secondExit = collector.takeExitFor(QStringLiteral("task-a"));
        QVERIFY(secondExit.has_value());
        QCOMPARE(*secondExit, 2);
        QVERIFY(!collector.takeExitFor(QStringLiteral("task-a")).has_value());

        QCOMPARE(collector.takeFor(QStringLiteral("task-b")),
                 QStringList({QStringLiteral("second output\n")}));
        const auto exitB = collector.takeExitFor(QStringLiteral("task-b"));
        QVERIFY(exitB.has_value());
        QCOMPARE(*exitB, 1);
        QVERIFY(!collector.takeExitFor(QStringLiteral("task-b")).has_value());
    }

    void testWorkspaceCommandResultDecoder() {
        const WorkspaceCommandResult textResult =
            WorkspaceCommandResultDecoder::decode(
                false, QJsonValue(QStringLiteral("network failure")));
        QVERIFY(!textResult.success);
        QCOMPARE(textResult.text, QStringLiteral("network failure"));
        QVERIFY(!textResult.hasServerDetails);

        const WorkspaceCommandResult serverResult =
            WorkspaceCommandResultDecoder::decode(
                true,
                QJsonObject{
                    {"success", false},
                    {"programUri", "file:///tmp/program"},
                    {"taskId", "task-7"},
                    {"codegenAvailable", true},
                    {"message", "compiler failed"},
                });
        QVERIFY(!serverResult.success);
        QVERIFY(serverResult.hasServerDetails);
        QCOMPARE(serverResult.programUri,
                 QStringLiteral("file:///tmp/program"));
        QCOMPARE(serverResult.taskId, QStringLiteral("task-7"));
        QVERIFY(serverResult.hasCodegenAvailable);
        QVERIFY(serverResult.codegenAvailable);
        QCOMPARE(serverResult.serverMessage,
                 QStringLiteral("compiler failed"));

        const WorkspaceCommandResult successfulResult =
            WorkspaceCommandResultDecoder::decode(
                true, QJsonObject{{"taskId", "task-8"}});
        QVERIFY(successfulResult.success);
        QCOMPARE(successfulResult.taskId, QStringLiteral("task-8"));

        const WorkspaceCommandResult transportFailure =
            WorkspaceCommandResultDecoder::decode(
                false, QJsonObject{{"success", true}});
        QVERIFY(!transportFailure.success);
    }

    void testWorkspaceCommandControllerAttachesEarlyRunOutput() {
        LspClient client;
        LspClientEventSource events(&client);
        CompilerPanel panel;
        WorkspaceCommandController::Dependencies dependencies;
        dependencies.client = &client;
        dependencies.events = &events;
        dependencies.compilerPanel = &panel;
        WorkspaceCommandController controller(
            std::move(dependencies), WorkspaceCommandController::Callbacks{});

        emit client.processOutputReceived(QStringLiteral("task-1"),
                                          QStringLiteral("hello from process\n"));
        controller.handleCommandResult(
            QStringLiteral("zith.run"), true,
            QJsonObject{{QStringLiteral("taskId"), QStringLiteral("task-1")},
                        {QStringLiteral("programUri"),
                         QStringLiteral("file:///tmp/helios-demo")}});

        QVERIFY(panel.outputText().contains(
            QStringLiteral("hello from process")));
        QVERIFY(panel.outputText().contains(
            QStringLiteral("Run started (task task-1)")));
        QCOMPARE(controller.state().runningTaskId, QStringLiteral("task-1"));
    }

    void testWorkspaceCommandControllerSurfacesServerFailure() {
        LspClient client;
        LspClientEventSource events(&client);
        CompilerPanel panel;
        WorkspaceCommandController::Dependencies dependencies;
        dependencies.client = &client;
        dependencies.events = &events;
        dependencies.compilerPanel = &panel;
        WorkspaceCommandController controller(
            std::move(dependencies), WorkspaceCommandController::Callbacks{});

        controller.handleCommandResult(
            QStringLiteral("zith.build"), false,
            QJsonObject{{QStringLiteral("message"),
                         QStringLiteral("compiler unavailable")},
                        {QStringLiteral("codegenAvailable"), false}});

        QVERIFY(panel.outputText().contains(
            QStringLiteral("compiler unavailable")));
        QVERIFY(panel.outputText().contains(
            QStringLiteral("codegenAvailable: false")));
    }

    void testWorkspaceCommandControllerOwnsProcessStopCleanup() {
        LspClient client;
        LspClientEventSource events(&client);
        CompilerPanel panel;
        WorkspaceCommandController::Dependencies dependencies;
        dependencies.client = &client;
        dependencies.events = &events;
        dependencies.compilerPanel = &panel;
        WorkspaceCommandController controller(
            std::move(dependencies), WorkspaceCommandController::Callbacks{});

        controller.handleCommandResult(
            QStringLiteral("zith.run"), true,
            QJsonObject{{QStringLiteral("taskId"), QStringLiteral("task-1")}});
        emit client.workDoneProgressReceived(QStringLiteral("progress-1"),
                                             QStringLiteral("begin"), {});

        emit client.processStopped(false);

        QVERIFY(!controller.state().taskRunning);
        QVERIFY(panel.outputText().contains(
            QStringLiteral("LSP stopped; any active process is no longer "
                           "being tracked.")));
    }

    void testWorkspaceCommandControllerOwnsClientEventWiring() {
        LspClient client;
        LspClientEventSource events(&client);
        CompilerPanel panel;
        bool saveRequested = false;
        WorkspaceCommandController::Callbacks callbacks;
        callbacks.saveAll = [&]() { saveRequested = true; };
        WorkspaceCommandController::Dependencies dependencies;
        dependencies.client = &client;
        dependencies.events = &events;
        dependencies.compilerPanel = &panel;
        WorkspaceCommandController controller(
            std::move(dependencies), std::move(callbacks));

        emit client.saveAllRequested();
        QVERIFY(saveRequested);

        emit client.processOutputReceived(
            QStringLiteral("task-1"), QStringLiteral("early output\n"));
        emit client.commandResult(
            QStringLiteral("zith.run"), true,
            QJsonObject{{QStringLiteral("taskId"), QStringLiteral("task-1")}});

        QVERIFY(panel.outputText().contains(
            QStringLiteral("early output")));
        QVERIFY(controller.handleShellCommand(ShellCommand::Stop));
        QVERIFY(!controller.handleShellCommand(ShellCommand::Find));
    }

    void testWorkspaceCommandControllerRejectsNonZithWorkspaceCommands() {
        LspClient client;
        LspClientEventSource events(&client);
        CompilerPanel panel;
        CodeEditor editor;
        editor.setFilePath(QStringLiteral("/tmp/main.c"));

        WorkspaceCommandController::Callbacks callbacks;
        callbacks.currentEditor = [&]() { return &editor; };
        callbacks.isZithEditor = [](CodeEditor *) { return false; };
        WorkspaceCommandController::Dependencies dependencies;
        dependencies.client = &client;
        dependencies.events = &events;
        dependencies.compilerPanel = &panel;
        WorkspaceCommandController controller(
            std::move(dependencies), std::move(callbacks));

        controller.runBuild();
        QVERIFY(panel.outputText().contains(
            QStringLiteral("Open an active Zith file to build the project.")));
        panel.clearOutput();

        controller.runCheckFile();
        QVERIFY(panel.outputText().contains(
            QStringLiteral("Check File requires an active Zith file.")));
        panel.clearOutput();

        controller.runProject();
        QVERIFY(panel.outputText().contains(
            QStringLiteral("Open an active Zith file to run the project.")));
        QVERIFY(!controller.state().taskRunning);
    }

    void testProjectTreeModelLimitAndPagination() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString rootPath = tempDir.path();
        
        // 1. Create nested structure of depth 15
        QString currentPath = rootPath;
        for (int i = 0; i < 15; ++i) {
            currentPath = QDir(currentPath).filePath(QString("level_%1").arg(i));
            QDir().mkpath(currentPath);
        }

        // 2. Create folder with 120 files to test chunked pagination
        QString pagDir = QDir(rootPath).filePath("paginated");
        QDir().mkpath(pagDir);
        for (int i = 0; i < 120; ++i) {
            QFile file(QDir(pagDir).filePath(QString("file_%1.zith").arg(i)));
            if (file.open(QIODevice::WriteOnly)) {
                file.close();
            }
        }

        ProjectTreeModel model;
        model.setMaxDepth(12);

        QSignalSpy spy(&model, &ProjectTreeModel::loadingFinished);
        model.setRootPath(rootPath);
        model.rowCount(QModelIndex()); // This will trigger startScan for root!

        // Wait for root node scan to complete
        QVERIFY(spy.wait(2000));

        // Find and expand "paginated"
        QModelIndex pagIdx;
        int rc = model.rowCount(QModelIndex());
        for (int i = 0; i < rc; ++i) {
            QModelIndex childIdx = model.index(i, 0, QModelIndex());
            if (model.filePath(childIdx).endsWith("paginated")) {
                pagIdx = childIdx;
                break;
            }
        }
        
        QVERIFY(pagIdx.isValid());
        
        // Clear spy signals and trigger scanning for "paginated" folder
        spy.clear();
        // Accessing rows triggers startScan
        model.rowCount(pagIdx);
        QVERIFY(spy.wait(2000));

        int pagCount = model.rowCount(pagIdx);
        // It should contain 100 entries + 1 "Load more..." node
        QCOMPARE(pagCount, 101);

        // Find "Load more..." and trigger loadMore
        QModelIndex moreIdx = model.index(100, 0, pagIdx);
        QVERIFY(moreIdx.isValid());
        QCOMPARE(model.data(moreIdx, Qt::DisplayRole).toString(), QString("Load more..."));

        model.loadMore(moreIdx);
        
        // Remaining 20 should be loaded, no "Load more..." node left
        int pagCountAfter = model.rowCount(pagIdx);
        QCOMPARE(pagCountAfter, 120);
    }

    void testProjectTreeFileIcons() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QStringList names = {"sample.zith", "sample.c", "sample.h"};
        for (const QString &name : names) {
            QFile file(QDir(tempDir.path()).filePath(name));
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.close();
        }

        ProjectTreeModel model;
        QSignalSpy spy(&model, &ProjectTreeModel::loadingFinished);
        model.setRootPath(tempDir.path());
        model.rowCount(QModelIndex());
        QVERIFY(spy.wait(2000));

        bool foundZith = false;
        bool foundC = false;
        bool foundH = false;
        for (int i = 0; i < model.rowCount(QModelIndex()); ++i) {
            const QModelIndex index = model.index(i, 0, QModelIndex());
            const QVariant icon = model.data(index, Qt::DecorationRole);
            QVERIFY(icon.canConvert<QIcon>());
            QVERIFY(!icon.value<QIcon>().isNull());
            const QString suffix =
                QFileInfo(model.filePath(index)).suffix().toLower();
            if (suffix == "zith")
                foundZith = true;
            else if (suffix == "c")
                foundC = true;
            else if (suffix == "h")
                foundH = true;
        }
        QVERIFY(foundZith);
        QVERIFY(foundC);
        QVERIFY(foundH);
    }

    void testZithRuntimeCatalogRejectsLocalCacheAsRelease() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        ZithRuntimeCatalog catalog;
        const QString cacheRoot = QDir(tempDir.path()).filePath("zith-runtime");
        catalog.setCacheRoot(cacheRoot);
        QDir rootDir(cacheRoot);
        QVERIFY(rootDir.mkpath("."));

        auto createRuntime = [&rootDir](const QString &tag, const QString &marker) {
            const QString releaseRoot = rootDir.filePath(tag);
            QVERIFY2(QDir().mkpath(QDir(releaseRoot).filePath("stdlib")),
                     qPrintable(releaseRoot));

            QFile lsp(QDir(releaseRoot).filePath("zith-lsp"));
            QVERIFY2(lsp.open(QIODevice::WriteOnly | QIODevice::Truncate),
                     qPrintable(lsp.fileName()));
            lsp.write("fake-lsp\n");
            lsp.close();
            QVERIFY2(QFile::setPermissions(lsp.fileName(),
                                           QFileDevice::ReadOwner |
                                               QFileDevice::WriteOwner |
                                               QFileDevice::ExeOwner),
                     qPrintable(lsp.fileName()));

            QFile stdlibFile(QDir(releaseRoot).filePath("stdlib/std.zith"));
            QVERIFY2(stdlibFile.open(QIODevice::WriteOnly | QIODevice::Truncate),
                     qPrintable(stdlibFile.fileName()));
            stdlibFile.write(marker.toUtf8());
            stdlibFile.close();
        };

        createRuntime("local", "outdated");
        createRuntime("v1.2.3", "release");
        createRuntime("v1.10.0", "newer");

        const auto newest = catalog.resolveNewestInstalledRelease();
        QVERIFY(newest.has_value());
        QCOMPARE(newest->tag, QStringLiteral("v1.10.0"));
        QVERIFY(!newest->paths.lspPath.contains("/local/"));
        QVERIFY(!newest->paths.stdlibPath.contains("/local/"));

        QVERIFY(!catalog.resolveInstalledRelease("local").has_value());
    }

    void testZithRuntimeStateTracksIdentitySeparatelyFromStatus() {
        ZithRuntimeState state;
        state.setStatusText("Resolving");
        state.activate("/tmp/zith-lsp", "/tmp/stdlib", "v1.2.3",
                       "/workspace/project");

        QCOMPARE(state.statusText(), QString("Resolving"));
        QCOMPARE(state.tag(), QString("v1.2.3"));
        QCOMPARE(state.lspPath(), QString("/tmp/zith-lsp"));
        QCOMPARE(state.stdlibPath(), QString("/tmp/stdlib"));
        QCOMPARE(state.workspaceRoot(), QString("/workspace/project"));
        QVERIFY(state.matches("/tmp/zith-lsp", "/tmp/stdlib",
                              "/workspace/project"));
        QVERIFY(!state.matches("/tmp/other-lsp", "/tmp/stdlib",
                               "/workspace/project"));

        state.clearRuntime();

        QCOMPARE(state.statusText(), QString("Resolving"));
        QVERIFY(state.tag().isEmpty());
        QVERIFY(state.lspPath().isEmpty());
        QVERIFY(state.stdlibPath().isEmpty());
        QVERIFY(state.workspaceRoot().isEmpty());
        QVERIFY(!state.matches("/tmp/zith-lsp", "/tmp/stdlib",
                               "/workspace/project"));
    }

    void testZithRuntimeLifecycleCoordinatorOwnsDisableAndCacheTransitions() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        ZithRuntimeLifecycleCoordinator coordinator;
        coordinator.setCacheRootForTesting(
            QDir(tempDir.path()).filePath("runtime-cache"));
        coordinator.setWorkspaceRoot("/workspace/project");
        const QString cacheRoot = coordinator.runtimeCacheRootPath();
        QVERIFY(QDir().mkpath(QDir(cacheRoot).filePath("v1.0.0")));
        QFile cachedMarker(QDir(cacheRoot).filePath("v1.0.0/marker"));
        QVERIFY(cachedMarker.open(QIODevice::WriteOnly));
        cachedMarker.write("cached");
        cachedMarker.close();

        QSignalSpy stateSpy(
            &coordinator,
            &ZithRuntimeLifecycleCoordinator::stateChanged);
        coordinator.setEnabled(true);
        QVERIFY(coordinator.isEnabled());
        QVERIFY(coordinator.client() != nullptr);

        coordinator.setEnabled(false);

        QVERIFY(!coordinator.isEnabled());
        QCOMPARE(coordinator.state().statusText(), QString("Disabled"));
        QVERIFY(coordinator.state().tag().isEmpty());
        QVERIFY(stateSpy.count() > 0);

        coordinator.setEnabled(true);
        QString errorMessage;
        QVERIFY(coordinator.clearCachedRuntime(&errorMessage));
        QVERIFY(errorMessage.isEmpty());
        QVERIFY(!QFileInfo::exists(cacheRoot));
        QVERIFY(coordinator.state().statusText().contains(
            "Runtime cache cleared"));
    }

    void testLspRuntimeControllerOwnsDisableTransition() {
        ZithRuntimeLifecycleCoordinator runtime;
        LspClient clangd;
        DiagnosticsPanel diagnostics;
        LspDiagnostic diagnostic;
        diagnostic.severity = 1;
        diagnostic.message = QStringLiteral("failure");
        diagnostics.setDiagnostics(
            QStringLiteral("file:///workspace/main.zith"), 1,
            {diagnostic});
        QCOMPARE(diagnostics.errorCount(), 1);

        int reconciliations = 0;
        int actionRefreshes = 0;
        int restartActionUpdates = 0;
        bool restartActionEnabled = true;
        FakeLspSettingsPersistence persistence;

        LspRuntimeController::Callbacks callbacks;
        callbacks.reconcileClangd = [&]() { ++reconciliations; };
        callbacks.refreshActions = [&]() { ++actionRefreshes; };
        callbacks.setRestartActionEnabled = [&](bool enabled) {
            ++restartActionUpdates;
            restartActionEnabled = enabled;
        };

        LspRuntimeController::Dependencies dependencies;
        dependencies.zithRuntime = &runtime;
        dependencies.clangdClient = &clangd;
        dependencies.diagnosticsPanel = &diagnostics;
        dependencies.settingsPersistence = &persistence;
        LspRuntimeController controller(std::move(dependencies),
                                        std::move(callbacks));

        controller.initialize(false);
        QVERIFY(!controller.isEnabled());
        QVERIFY(!runtime.isEnabled());
        QVERIFY(controller.handleShellCommand(ShellCommand::RestartLsp));
        QVERIFY(!controller.handleShellCommand(ShellCommand::FormatDocument));
        QCOMPARE(reconciliations, 1);
        QCOMPARE(actionRefreshes, 1);
        QCOMPARE(restartActionUpdates, 1);
        QVERIFY(!restartActionEnabled);

        controller.recordError(QStringLiteral("runtime failed"));
        QCOMPARE(controller.lastError(), QStringLiteral("runtime failed"));
        controller.clearError();
        QVERIFY(controller.lastError().isEmpty());

        controller.setEnabled(false);
        QVERIFY(!controller.isEnabled());
        QVERIFY(diagnostics.allDiagnostics().isEmpty());
        QCOMPARE(diagnostics.errorCount(), 0);
        QVERIFY(!persistence.enabled);
        QCOMPARE(persistence.enabledWrites, 1);
        QCOMPARE(reconciliations, 2);
        QCOMPARE(actionRefreshes, 2);
        QCOMPARE(restartActionUpdates, 2);
    }

    void testLspRuntimeControllerOwnsSettingsEventWiring() {
        SettingsPanel settingsPanel;
        LspManagerDialog managerDialog;
        ZithRuntimeLifecycleCoordinator runtime;
        LspClient clangd;

        int reconciliations = 0;
        FakeLspSettingsPersistence persistence;

        LspRuntimeController::Callbacks callbacks;
        callbacks.reconcileClangd = [&]() { ++reconciliations; };

        LspRuntimeController::Dependencies dependencies;
        dependencies.settingsPanel = &settingsPanel;
        dependencies.lspManagerDialog = &managerDialog;
        dependencies.zithRuntime = &runtime;
        dependencies.clangdClient = &clangd;
        dependencies.settingsPersistence = &persistence;
        LspRuntimeController controller(std::move(dependencies),
                                        std::move(callbacks));

        emit managerDialog.useOnlineZithLspChanged(true);
        emit managerDialog.cLspEnabledChanged(true);
        emit managerDialog.cLspPathChanged(QStringLiteral("/usr/bin/clangd"));

        QVERIFY(persistence.online);
        QVERIFY(persistence.cFamilyEnabled);
        QCOMPARE(persistence.clangdPath, QStringLiteral("/usr/bin/clangd"));
        QCOMPARE(persistence.onlineWrites, 1);
        QCOMPARE(persistence.cFamilyWrites, 1);
        QCOMPARE(persistence.pathWrites, 1);
        QCOMPARE(reconciliations, 2);
    }

    void testZithToolchainPreferOnlineSkipsLocalCheckout() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        ZithToolchainManager manager;
        const QString cacheRoot = QDir(tempDir.path()).filePath("zith-runtime");
        manager.setCacheRootForTesting(cacheRoot);

        auto createRuntime = [&cacheRoot](const QString &tag, const QString &marker) {
            const QString releaseRoot = QDir(cacheRoot).filePath(tag);
            QVERIFY2(QDir().mkpath(QDir(releaseRoot).filePath("stdlib")),
                     qPrintable(releaseRoot));

            QFile lsp(QDir(releaseRoot).filePath("zith-lsp"));
            QVERIFY2(lsp.open(QIODevice::WriteOnly | QIODevice::Truncate),
                     qPrintable(lsp.fileName()));
            lsp.write("fake-lsp\n");
            lsp.close();
            QVERIFY2(QFile::setPermissions(lsp.fileName(),
                                           QFileDevice::ReadOwner |
                                               QFileDevice::WriteOwner |
                                               QFileDevice::ExeOwner),
                     qPrintable(lsp.fileName()));

            QFile stdlibFile(QDir(releaseRoot).filePath("stdlib/std.zith"));
            QVERIFY2(stdlibFile.open(QIODevice::WriteOnly | QIODevice::Truncate),
                     qPrintable(stdlibFile.fileName()));
            stdlibFile.write(marker.toUtf8());
            stdlibFile.close();
        };

        createRuntime("v0.6.2", "online");
        manager.setPreferOnline(true);

        QString lspPath;
        QString stdlibPath;
        QString tag;
        QVERIFY(manager.resolveNewestInstalledRelease(&lspPath, &stdlibPath, &tag));
        QCOMPARE(tag, QStringLiteral("v0.6.2"));
        QVERIFY(lspPath.contains("/v0.6.2/zith-lsp"));
        QVERIFY(stdlibPath.contains("/v0.6.2/stdlib"));

        QSignalSpy readySpy(&manager, &ZithToolchainManager::ready);
        manager.ensureLatest(true);
        QTRY_VERIFY(readySpy.count() > 0);
        QCOMPARE(readySpy.at(0).at(0).toString(), lspPath);
    }

    void testSearchPanelScansConfiguredExtensions() {
        const QStringList extensions = {"zith", "cpp", "dockerfile"};
        const QStringList excludedDirs = {".git", "vendor"};
        const WorkspaceSearch::ScanPolicy policy{extensions, excludedDirs};

        QVERIFY(WorkspaceSearch::shouldScanFile(
            "/tmp/project/src/main.zith", policy));
        QVERIFY(WorkspaceSearch::shouldScanFile(
            "/tmp/project/Dockerfile", policy));
        QVERIFY(WorkspaceSearch::shouldScanFile(
            "/tmp/project/notes.txt", policy) == false);
        QVERIFY(WorkspaceSearch::shouldScanFile(
            "/tmp/project/vendor/lib.cpp", policy) == false);
        QVERIFY(WorkspaceSearch::shouldScanFile(
            "/tmp/project/.git/config", policy) == false);
        QVERIFY(WorkspaceSearch::shouldScanFile(
            "/tmp/project/src/main.cpp", policy));
        QVERIFY(WorkspaceSearch::shouldScanFile(
            "/tmp/project/dockerfile", policy));
    }

    void testWorkspaceSearchControllerOwnsAsyncScanAndPreview() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString sourcePath =
            tempDir.filePath(QStringLiteral("main.zith"));
        QFile source(sourcePath);
        QVERIFY(source.open(QIODevice::WriteOnly | QIODevice::Text));
        source.write("needle first\nignored\nneedle second\n");
        source.close();

        const QString ignoredPath =
            tempDir.filePath(QStringLiteral("main.bin"));
        QFile ignored(ignoredPath);
        QVERIFY(ignored.open(QIODevice::WriteOnly | QIODevice::Text));
        ignored.write("needle should not be scanned\n");
        ignored.close();

        WorkspaceSearchController controller(
            [=]() {
                return WorkspaceSearch::ScanPolicy{
                    {QStringLiteral("zith")}, {}};
            });
        controller.setRootPath(tempDir.path());

        QVector<SearchResult> results;
        int finishedCount = 0;
        int totalResults = -1;
        bool truncated = true;
        connect(&controller, &WorkspaceSearchController::resultsReady,
                [&](const QVector<SearchResult> &batch) {
                    results += batch;
                });
        connect(&controller, &WorkspaceSearchController::searchFinished,
                [&](int total, bool wasTruncated) {
                    ++finishedCount;
                    totalResults = total;
                    truncated = wasTruncated;
                });

        controller.search(QStringLiteral("needle"));
        QTRY_VERIFY(finishedCount > 0);
        QCOMPARE(totalResults, 2);
        QVERIFY(!truncated);
        QCOMPARE(results.size(), 2);
        QCOMPARE(results.at(0).path, sourcePath);

        QVector<WorkspaceSearch::SearchReplaceTarget> targets;
        int previewCount = 0;
        connect(&controller,
                &WorkspaceSearchController::replaceAllPreviewReady,
                [&](const QString &needle, const QString &replacement,
                    const QVector<WorkspaceSearch::SearchReplaceTarget>
                        &previewTargets) {
                    QCOMPARE(needle, QStringLiteral("needle"));
                    QCOMPARE(replacement, QStringLiteral("replacement"));
                    targets = previewTargets;
                    ++previewCount;
                });

        controller.previewReplace(QStringLiteral("needle"),
                                  QStringLiteral("replacement"));
        QTRY_VERIFY(previewCount > 0);
        QCOMPARE(targets.size(), 1);
        QCOMPARE(targets.first().path, sourcePath);
        QCOMPARE(targets.first().matches, 2);
    }

    void testSearchPanelPreservesReplacementWhitespace() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QFile source(tempDir.filePath(QStringLiteral("main.zith")));
        QVERIFY(source.open(QIODevice::WriteOnly | QIODevice::Text));
        source.write("needle\n");
        source.close();

        SearchPanel panel([=]() {
            return WorkspaceSearch::ScanPolicy{{QStringLiteral("zith")}, {}};
        });
        panel.setRootPath(tempDir.path());

        const QList<QLineEdit *> inputs = panel.findChildren<QLineEdit *>();
        QCOMPARE(inputs.size(), 2);
        inputs.at(0)->setText(QStringLiteral("needle"));
        inputs.at(1)->setText(QStringLiteral("  replacement  "));

        QString receivedReplacement;
        connect(&panel, &SearchPanel::replaceAllPreviewReady,
                [&](const QString &, const QString &replacement,
                    const QVector<WorkspaceSearch::SearchReplaceTarget> &) {
                    receivedReplacement = replacement;
                });

        QPushButton *replaceButton = nullptr;
        for (QPushButton *button : panel.findChildren<QPushButton *>()) {
            if (button->text() == QStringLiteral("Replace All")) {
                replaceButton = button;
                break;
            }
        }
        QVERIFY(replaceButton);
        replaceButton->click();

        QTRY_COMPARE(receivedReplacement, QStringLiteral("  replacement  "));
    }

    void testFindReplaceBarFindAndWrap() {
        CodeEditor editor;
        editor.setInitialDocumentText("alpha beta\nbeta gamma\nbeta");

        FindReplaceBar bar;
        bar.setEditor(&editor);
        bar.showFind();

        auto *findInput = bar.findChild<QLineEdit *>();
        QVERIFY(findInput);
        findInput->setText("beta");

        QTextCursor first = editor.textCursor();
        QVERIFY(first.hasSelection());
        QCOMPARE(first.selectedText(), QString("beta"));
        QCOMPARE(editor.textCursor().selectionStart(), 6);

        bar.findNext();
        QCOMPARE(editor.textCursor().selectionStart(), 11);
        bar.findNext();
        QCOMPARE(editor.document()->findBlock(editor.textCursor().selectionStart()).blockNumber(), 2);
        QCOMPARE(editor.textCursor().selectionStart(), 22);

        bar.findNext();
        QCOMPARE(editor.textCursor().selectionStart(), 6);

        bar.findPrevious();
        QCOMPARE(editor.document()->findBlock(editor.textCursor().selectionStart()).blockNumber(), 2);
        QCOMPARE(editor.textCursor().selectionStart(), 22);
        bar.findPrevious();
        QCOMPARE(editor.textCursor().selectionStart(), 11);
    }

    void testFindReplaceBarReplaceAndReplaceAll() {
        CodeEditor editor;
        editor.setInitialDocumentText("alpha beta\nbeta gamma\nbeta");

        FindReplaceBar bar;
        bar.setEditor(&editor);
        bar.showReplace();

        const QList<QLineEdit *> inputs = bar.findChildren<QLineEdit *>();
        QCOMPARE(inputs.size(), 2);
        inputs[0]->setText("beta");

        const QList<QPushButton *> buttons = bar.findChildren<QPushButton *>();
        QPushButton *replaceBtn = nullptr;
        QPushButton *replaceAllBtn = nullptr;
        for (QPushButton *button : buttons) {
            if (button->text() == "Replace")
                replaceBtn = button;
            else if (button->text() == "All")
                replaceAllBtn = button;
        }
        QVERIFY(replaceBtn);
        QVERIFY(replaceAllBtn);

        inputs[1]->setText("X");
        replaceBtn->click();
        QCOMPARE(editor.toPlainText(), QString("alpha X\nbeta gamma\nbeta"));

        QTextCursor cursor(editor.document());
        cursor.movePosition(QTextCursor::Start);
        editor.setTextCursor(cursor);
        replaceAllBtn->click();
        QCOMPARE(editor.toPlainText(), QString("alpha X\nX gamma\nX"));
        QVERIFY(editor.document()->isModified());
    }

    void testVimSearchSession() {
        QPlainTextEdit editor;
        editor.setPlainText("alpha beta\ngamma delta\nalpha gamma");
        VimSearchSession search(&editor);

        auto press = [&search](int key, const QString &text) {
            QKeyEvent event(QEvent::KeyPress, key, Qt::NoModifier, text);
            QVERIFY(search.handleKeyPress(&event));
        };

        QTextCursor start(editor.document());
        editor.setTextCursor(start);
        search.begin(true);
        press(Qt::Key_G, "g");
        press(Qt::Key_A, "a");
        press(Qt::Key_M, "m");
        press(Qt::Key_M, "m");
        press(Qt::Key_A, "a");
        press(Qt::Key_Backspace, "");
        press(Qt::Key_A, "a");
        press(Qt::Key_Return, "");
        QCOMPARE(editor.textCursor().blockNumber(), 1);
        QVERIFY(!search.isActive());

        search.repeat(true, 1);
        QCOMPARE(editor.textCursor().blockNumber(), 2);

        editor.setTextCursor(QTextCursor(editor.document()));
        search.begin(true);
        press(Qt::Key_X, "x");
        QKeyEvent escape(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QVERIFY(search.handleKeyPress(&escape));
        QVERIFY(!search.isActive());
        search.repeat(true, 1);
        QCOMPARE(editor.textCursor().blockNumber(), 1);
    }

    void testVimExpandedCommands() {
        QPlainTextEdit editor;
        editor.setPlainText("alpha beta\ngamma delta\nepsilon");
        VimMotionController controller(&editor);
        controller.setEnabled(true);
        QStringList commands;
        connect(&controller, &VimMotionController::commandEntered,
                [&commands](const QString &command) { commands << command; });
        auto press = [&controller](int key, const QString &text) {
            QKeyEvent event(QEvent::KeyPress, key, Qt::NoModifier, text);
            QVERIFY(controller.handleKeyPress(&event));
        };

        editor.setTextCursor(QTextCursor(editor.document()));
        press(Qt::Key_0, "0");
        press(Qt::Key_L, "l");
        press(Qt::Key_X, "x");
        QCOMPARE(editor.toPlainText(),
                 QString("apha beta\ngamma delta\nepsilon"));

        press(Qt::Key_U, "u");
        QCOMPARE(editor.toPlainText(),
                 QString("alpha beta\ngamma delta\nepsilon"));

        press(Qt::Key_W, "w");
        press(Qt::Key_W, "w");
        press(Qt::Key_W, "w");
        QCOMPARE(editor.textCursor().positionInBlock(), 0);

        press(Qt::Key_R, "r");
        press(Qt::Key_Z, "z");
        QCOMPARE(editor.textCursor().positionInBlock(), 1);

        press(Qt::Key_Slash, "/");
        press(Qt::Key_G, "g");
        press(Qt::Key_A, "a");
        press(Qt::Key_M, "m");
        press(Qt::Key_M, "m");
        press(Qt::Key_Return, "");
        QCOMPARE(editor.textCursor().blockNumber(), 1);

        press(Qt::Key_0, "0");
        press(Qt::Key_V, "v");
        press(Qt::Key_E, "e");
        press(Qt::Key_X, "x");
        QCOMPARE(editor.textCursor().positionInBlock(), 0);

        QTextCursor start(editor.document());
        start.movePosition(QTextCursor::Start);
        editor.setTextCursor(start);
        press(Qt::Key_D, "d");
        press(Qt::Key_D, "d");
        QCOMPARE(editor.document()->blockCount(), 2);

        editor.setPlainText("alpha beta\ngamma delta\nepsilon");
        QTextCursor opCursor(editor.document());
        editor.setTextCursor(opCursor);
        press(Qt::Key_D, "d");
        press(Qt::Key_F, "f");
        press(Qt::Key_A, "a");
        QCOMPARE(editor.toPlainText(),
                 QString("a beta\ngamma delta\nepsilon"));
        QTextCursor opCursor2(editor.document());
        editor.setTextCursor(opCursor2);
        press(Qt::Key_D, "d");
        press(Qt::Key_D, "d");
        QCOMPARE(editor.toPlainText(),
                 QString("gamma delta\nepsilon"));

        editor.setPlainText("alpha beta\ngamma delta\nepsilon");
        editor.setTextCursor(QTextCursor(editor.document()));
        press(Qt::Key_C, "c");
        press(Qt::Key_C, "c");
        QCOMPARE(editor.toPlainText(),
                 QString("\ngamma delta\nepsilon"));
        QCOMPARE(editor.textCursor().positionInBlock(), 0);

        QKeyEvent exitInsert(QEvent::KeyPress, Qt::Key_Escape,
                             Qt::NoModifier);
        QVERIFY(controller.handleKeyPress(&exitInsert));
        QCOMPARE(controller.mode(), VimMotionController::Mode::Normal);
        press(Qt::Key_Colon, ":");
        press(Qt::Key_W, "w");
        press(Qt::Key_Return, "");
        QCOMPARE(commands.count(), 1);
        QCOMPARE(commands.last(), QString("w"));
    }

    void testTranslationKeysMatch() {
        auto &tr = TranslationManager::instance();
        tr.loadLocale("en-US");
        QCOMPARE(tr.translate("bottom.close"), QString("Close"));
        tr.loadLocale("pt-BR");
        QCOMPARE(tr.translate("bottom.close"), QString("Fechar"));
        QCOMPARE(tr.translate("bottom.clear"), QString("Limpar"));
    }

    void testWorkspaceReplaceEditsAndApply() {
        const QString text = QString("alpha beta\nbeta gamma\nlast beta\n");

        const auto edits = WorkspaceSearch::replaceEdits(text, {"beta", "X"});
        QCOMPARE(edits.size(), 3);

        const auto &first = edits.at(0);
        const auto &second = edits.at(1);
        const auto &third = edits.at(2);
        QCOMPARE(first.first.start.line, 0);
        QCOMPARE(first.first.start.character, 6);
        QCOMPARE(first.first.end.character, 10);
        QCOMPARE(second.first.start.line, 1);
        QCOMPARE(second.first.start.character, 0);
        QCOMPARE(second.first.end.character, 4);
        QCOMPARE(third.first.start.line, 2);
        QCOMPARE(third.first.start.character, 5);
        QCOMPARE(third.first.end.character, 9);

        QCOMPARE(WorkspaceSearch::applyReplaceEdits(text, edits),
                 QString("alpha X\nX gamma\nlast X\n"));
    }

    void testWorkspaceReplaceEditsIgnoresEmptyNeedleAndSyncsOpenFile() {
        QVERIFY(WorkspaceSearch::replaceEdits(
                    "no changes", {QString(), "X"}).isEmpty());

        CodeEditor editor;
        editor.setInitialDocumentText("alpha beta\nbeta gamma\n");
        const auto edits = WorkspaceSearch::replaceEdits(
            editor.toPlainText(), {"beta", "X"});
        const QString replaced =
            WorkspaceSearch::applyReplaceEdits(editor.toPlainText(), edits);
        QCOMPARE(replaced, QString("alpha X\nX gamma\n"));
        editor.setInitialDocumentText(replaced);
        QCOMPARE(editor.toPlainText(), replaced);
    }

    void testWorkspaceReplaceControllerUpdatesOpenAndClosedTargets() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString closedPath = tempDir.filePath(QStringLiteral("closed.zith"));
        QFile closedFile(closedPath);
        QVERIFY(closedFile.open(QIODevice::WriteOnly | QIODevice::Text));
        closedFile.write("alpha beta\n");
        closedFile.close();

        const QString openPath = tempDir.filePath(QStringLiteral("open.zith"));
        CodeEditor openEditor;
        openEditor.setFilePath(openPath);
        openEditor.setInitialDocumentText(QStringLiteral("beta gamma\n"));

        QStringList statusMessages;
        bool confirmCalled = false;
        int confirmedMatches = 0;
        int confirmedFiles = 0;
        QString confirmedNeedle;
        QString confirmedReplacement;
        bool searchRefreshed = false;
        WorkspaceReplaceController::Callbacks callbacks;
        callbacks.findOpenEditor = [&](const QString &path) {
            return path == openPath ? &openEditor : nullptr;
        };
        callbacks.confirmReplacement =
            [&](const WorkspaceReplaceController::ReplacementConfirmation
                    &confirmation) {
                confirmCalled = true;
                confirmedMatches = confirmation.totalMatches;
                confirmedFiles = confirmation.fileCount;
                confirmedNeedle = confirmation.needle;
                confirmedReplacement = confirmation.replacement;
                return true;
            };
        callbacks.showStatus = [&](const QString &message, int) {
            statusMessages.append(message);
        };
        callbacks.refreshSearch = [&]() { searchRefreshed = true; };

        WorkspaceReplaceController controller(std::move(callbacks));
        controller.replaceAll(
            QStringLiteral("beta"), QStringLiteral("X"),
            {{closedPath, 1}, {openPath, 1}});

        QVERIFY(confirmCalled);
        QCOMPARE(confirmedMatches, 2);
        QCOMPARE(confirmedFiles, 2);
        QCOMPARE(confirmedNeedle, QStringLiteral("beta"));
        QCOMPARE(confirmedReplacement, QStringLiteral("X"));
        QVERIFY(searchRefreshed);
        QVERIFY(openEditor.document()->isModified());
        QCOMPARE(openEditor.toPlainText(), QStringLiteral("X gamma\n"));
        QVERIFY(!statusMessages.isEmpty());
        QCOMPARE(statusMessages.last(),
                 QStringLiteral("Replaced 2 matches in workspace."));

        QFile updatedClosedFile(closedPath);
        QVERIFY(updatedClosedFile.open(QIODevice::ReadOnly | QIODevice::Text));
        QCOMPARE(QString::fromUtf8(updatedClosedFile.readAll()),
                 QStringLiteral("alpha X\n"));
    }

    void testWorkspaceEditRejectsUnsupportedForms() {
        QString error;

        QJsonObject documentChanges;
        documentChanges.insert("documentChanges", QJsonArray{});
        QVERIFY(!WorkspaceEdit::fromJson(documentChanges, &error));
        QCOMPARE(error, QString("Unsupported workspace edit from LSP."));

        error.clear();
        QJsonObject remoteChanges;
        QJsonObject changes;
        QJsonArray edits;
        QJsonObject edit;
        QJsonObject range;
        range.insert("start", QJsonObject{{"line", 0}, {"character", 0}});
        range.insert("end", QJsonObject{{"line", 0}, {"character", 0}});
        edit.insert("range", range);
        edit.insert("newText", "updated");
        edits.append(edit);
        changes.insert("https://example.com/file.zith", edits);
        remoteChanges.insert("changes", changes);

        QVERIFY(!WorkspaceEdit::fromJson(remoteChanges, &error));
        QCOMPARE(error, QString("Workspace edit contains a non-local URI."));
    }

    void testWorkspaceEditParsesLocalTargets() {
        QJsonObject root;
        QJsonObject changes;
        QJsonArray edits;
        QJsonObject edit;
        QJsonObject range;
        range.insert("start", QJsonObject{{"line", 0}, {"character", 1}});
        range.insert("end", QJsonObject{{"line", 0}, {"character", 3}});
        edit.insert("range", range);
        edit.insert("newText", "X");
        edits.append(edit);
        changes.insert(QUrl::fromLocalFile("/tmp/sample.zith").toString(), edits);
        root.insert("changes", changes);

        QString error;
        const auto parsed = WorkspaceEdit::fromJson(root, &error);
        QVERIFY2(parsed.has_value(), qPrintable(error));
        QCOMPARE(parsed->targets().size(), 1);
        QCOMPARE(parsed->targets().first().uri,
                 QUrl::fromLocalFile("/tmp/sample.zith").toString());
        QCOMPARE(parsed->targets().first().edits.size(), 1);
        QCOMPARE(parsed->targets().first().edits.first().second, QString("X"));
    }

    void testWorkspaceEditAppliesDescendingRanges() {
        const QList<QPair<LspRange, QString>> edits = {
            {{{0, 1}, {0, 2}}, "A"},
            {{{0, 3}, {0, 5}}, "BC"},
        };

        QString error;
        const auto updated = WorkspaceEdit::applyToText("012345", edits, &error);
        QVERIFY2(updated.has_value(), qPrintable(error));
        QCOMPARE(*updated, QString("0A2BC5"));
    }

    void testWorkspaceEditValidatesAllRangesBeforeApplying() {
        const QList<QPair<LspRange, QString>> edits = {
            {{{0, 0}, {0, 1}}, "safe"},
            {{{4, 0}, {4, 1}}, "invalid"},
        };

        QString error;
        const auto updated = WorkspaceEdit::applyToText("text", edits, &error);
        QVERIFY(!updated.has_value());
        QCOMPARE(error, QString("Workspace edit contains an invalid range."));
    }

    void testLanguageIdentityClassifiesPaths() {
        QCOMPARE(LanguageIdentity::forPath(QStringLiteral("/tmp/main.zith")),
                 LanguageIdentity::Language::Zith);
        QCOMPARE(LanguageIdentity::forPath(QStringLiteral("/tmp/main.CPP")),
                 LanguageIdentity::Language::CFamily);
        QCOMPARE(LanguageIdentity::forPath(QStringLiteral("/tmp/header.hxx")),
                 LanguageIdentity::Language::CFamily);
        QCOMPARE(LanguageIdentity::forPath(QStringLiteral("/tmp/README")),
                 LanguageIdentity::Language::PlainText);
    }

    void testLanguageIdentityProvidesStableLspIds() {
        QCOMPARE(LanguageIdentity::lspId(LanguageIdentity::Language::Zith),
                 QStringLiteral("zith"));
        QCOMPARE(LanguageIdentity::lspId(LanguageIdentity::Language::CFamily),
                 QStringLiteral("cpp"));
        QVERIFY(LanguageIdentity::lspId(LanguageIdentity::Language::PlainText)
                    .isEmpty());
    }

    void testLspDocumentCoordinatorRoutesLanguageServices() {
        LspClient zithClient;
        LspClient cFamilyClient;
        LspDocumentCoordinator coordinator;
        coordinator.setClient(LanguageIdentity::Language::Zith, &zithClient);
        coordinator.setClient(LanguageIdentity::Language::CFamily,
                              &cFamilyClient);
        coordinator.setEnabled(LanguageIdentity::Language::Zith, true);
        coordinator.setEnabled(LanguageIdentity::Language::CFamily, false);

        QCOMPARE(coordinator.clientForPath("/tmp/main.zith"), &zithClient);
        QCOMPARE(coordinator.clientForPath("/tmp/main.cpp"),
                 &cFamilyClient);
        QVERIFY(coordinator.shouldUseForPath("/tmp/main.zith"));
        QVERIFY(!coordinator.shouldUseForPath("/tmp/main.cpp"));
        QVERIFY(!coordinator.shouldUseForPath("/tmp/README"));

        CodeEditor editor;
        editor.setFilePath("/tmp/main.zith");
        QVERIFY(coordinator.isEditorLanguage(
            &editor, LanguageIdentity::Language::Zith));
        QVERIFY(!coordinator.isEditorLanguage(
            &editor, LanguageIdentity::Language::CFamily));
    }

    void testLanguageServiceWorkspaceControllerCoordinatesRouting() {
        LspClient zithClient;
        LspClient cFamilyClient;
        LspDocumentCoordinator documents;
        documents.setClient(LanguageIdentity::Language::Zith, &zithClient);
        documents.setClient(LanguageIdentity::Language::CFamily,
                            &cFamilyClient);

        ClangdLifecycleCoordinator clangd;
        QTabWidget tabs;
        auto *editor = new CodeEditor;
        editor->setFilePath(QStringLiteral("/tmp/main.cpp"));
        tabs.addTab(editor, QStringLiteral("main.cpp"));

        bool lspEnabled = true;
        bool cFamilyEnabled = false;
        bool presentationChanged = false;
        QString resolvedClangdPath;
        int configurationReads = 0;
        LanguageServiceWorkspaceController::Callbacks callbacks;
        callbacks.configuration = [&]() {
            ++configurationReads;
            LanguageServiceWorkspaceController::Configuration configuration;
            configuration.lspEnabled = lspEnabled;
            configuration.cFamilyEnabled = cFamilyEnabled;
            configuration.clangdPath = resolvedClangdPath;
            configuration.workspaceRoot =
                QStringLiteral("/tmp/workspace");
            return configuration;
        };
        callbacks.presentationChanged = [&]() { presentationChanged = true; };

        LanguageServiceWorkspaceController controller(
            &tabs, &documents, &clangd, std::move(callbacks));

        QVERIFY(controller.shouldUseForPath(QStringLiteral("/tmp/main.zith")));
        QVERIFY(!controller.shouldUseForPath(
            QStringLiteral("/tmp/main.cpp")));
        QVERIFY(controller.isEditorLanguage(
            editor, LanguageIdentity::Language::CFamily));

        const int beforeReconcile = configurationReads;
        controller.reconcile();
        QCOMPARE(configurationReads, beforeReconcile + 1);
        QCOMPARE(clangd.state(),
                 ClangdLifecycleCoordinator::State::Disabled);
        QVERIFY(presentationChanged);

        cFamilyEnabled = true;
        controller.reconcile();
        QCOMPARE(clangd.state(),
                 ClangdLifecycleCoordinator::State::MissingPath);
        QVERIFY(!controller.shouldUseForPath(QStringLiteral("/tmp/main.cpp")));

        resolvedClangdPath = QStringLiteral("/usr/bin/clangd");
        QVERIFY(controller.shouldUseForPath(QStringLiteral("/tmp/main.cpp")));
    }

    void testClangdExecutableResolverPrefersConfigurationAndSearchesPath() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString executablePath = directory.filePath("clangd");
        QFile executable(executablePath);
        QVERIFY(executable.open(QIODevice::WriteOnly));
        executable.close();
        QVERIFY(executable.setPermissions(
            QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));

        QCOMPARE(ClangdExecutableResolver::resolve(
                     QStringLiteral("  /configured/clangd  "),
                     directory.path()),
                 QStringLiteral("/configured/clangd"));
        QCOMPARE(ClangdExecutableResolver::resolve(
                     {}, directory.path()),
                 executablePath);
        QCOMPARE(ClangdExecutableResolver::resolve(
                     {}, QStringLiteral("/missing")),
                 QString());
    }

    void testClangdLifecycleCoordinatorStates() {
        ClangdLifecycleCoordinator coordinator;
        ClangdLifecycleCoordinator::Configuration configuration;
        configuration.lspEnabled = true;
        configuration.cFamilyEnabled = true;
        configuration.hasCFamilyDocuments = true;

        coordinator.reconcile(configuration);
        QCOMPARE(
            coordinator.state(),
            ClangdLifecycleCoordinator::State::MissingPath);
        QCOMPARE(coordinator.error(),
                 QStringLiteral("clangd was not found in PATH."));

        configuration.serverPath =
            QStringLiteral("/path/that/does/not/exist/clangd");
        coordinator.reconcile(configuration);
        QCOMPARE(coordinator.state(),
                 ClangdLifecycleCoordinator::State::Error);
        QVERIFY(!coordinator.error().isEmpty());
        QVERIFY(coordinator.activeWorkspaceRoot().isEmpty());

        configuration.lspEnabled = false;
        coordinator.reconcile(configuration);
        QCOMPARE(coordinator.state(),
                 ClangdLifecycleCoordinator::State::Disabled);
        QVERIFY(coordinator.error().isEmpty());
        QVERIFY(coordinator.activeWorkspaceRoot().isEmpty());
    }

    void testClangdLifecycleCoordinatorRestartsWhenWorkspaceRootChanges() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString serverPath =
            QDir(tempDir.path()).filePath(QStringLiteral("clangd-stub"));
        QFile serverFile(serverPath);
        QVERIFY(serverFile.open(QIODevice::WriteOnly));
        serverFile.write(
            "#!/bin/sh\n"
            "while IFS= read -r line; do\n"
            "  case \"$line\" in\n"
            "    *exit*) exit 0 ;;\n"
            "  esac\n"
            "done\n");
        serverFile.close();
        QVERIFY(QFile::setPermissions(
            serverPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                            QFileDevice::ExeOwner));

        LspClient client;
        ClangdLifecycleCoordinator coordinator;
        coordinator.setClient(&client);

        ClangdLifecycleCoordinator::Configuration configuration;
        configuration.lspEnabled = true;
        configuration.cFamilyEnabled = true;
        configuration.hasCFamilyDocuments = true;
        configuration.serverPath = serverPath;
        configuration.workspaceRoot =
            QDir(tempDir.path()).filePath(QStringLiteral("first"));

        QSignalSpy stoppedSpy(&client, &LspClient::processStopped);
        coordinator.reconcile(configuration);
        QTRY_VERIFY_WITH_TIMEOUT(client.isRunning(), 5000);
        QCOMPARE(coordinator.activeWorkspaceRoot(),
                 configuration.workspaceRoot);

        configuration.workspaceRoot =
            QDir(tempDir.path()).filePath(QStringLiteral("second"));
        coordinator.reconcile(configuration);
        QCOMPARE(coordinator.activeWorkspaceRoot(),
                 configuration.workspaceRoot);
        QTRY_VERIFY_WITH_TIMEOUT(stoppedSpy.count() > 0, 5000);
        QTRY_VERIFY_WITH_TIMEOUT(client.isRunning(), 5000);

        configuration.lspEnabled = false;
        coordinator.reconcile(configuration);
        client.waitForFinishedForTesting(5000);
    }

    void testEditorSyntaxControllerRoutesHighlighters() {
        CodeEditor editor;
        EditorSyntaxController controller;

        controller.apply(&editor, LanguageIdentity::Language::Zith);
        QVERIFY(qobject_cast<SyntaxHighlighter *>(
            controller.highlighterFor(&editor)));

        controller.apply(&editor, LanguageIdentity::Language::CFamily);
        QVERIFY(qobject_cast<CHighlighter *>(
            controller.highlighterFor(&editor)));

        controller.apply(&editor, LanguageIdentity::Language::PlainText);
        QVERIFY(qobject_cast<SyntaxHighlighter *>(
            controller.highlighterFor(&editor)));

        controller.remove(&editor);
        QCOMPARE(controller.highlighterFor(&editor), nullptr);
    }

    void testEditorSessionControllerCapturesAndRestoresDocuments() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString firstPath =
            QDir(tempDir.path()).filePath(QStringLiteral("first.zith"));
        const QString secondPath =
            QDir(tempDir.path()).filePath(QStringLiteral("second.cpp"));
        for (const auto &entry :
             {qMakePair(firstPath, QStringLiteral("zith")),
              qMakePair(secondPath, QStringLiteral("cpp"))}) {
            QFile file(entry.first);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
            file.write(entry.second.toUtf8());
            file.close();
        }

        QTabWidget tabs;
        SnippetManager snippets;
        LspCompletionModel completionModel;
        LspCompleter completer(&completionModel);
        DiagnosticsPanel diagnostics;
        EditorSyntaxController syntax;
        LspDocumentCoordinator documents;
        EditorSessionController::Callbacks callbacks;
        int routingRefreshes = 0;
        callbacks.connectEditorSignals = [](CodeEditor *) {};
        callbacks.updateCentralWidgetState = []() {};
        callbacks.refreshLspRouting = [&]() { ++routingRefreshes; };

        EditorSessionController::Dependencies dependencies;
        dependencies.tabWidget = &tabs;
        dependencies.snippetManager = &snippets;
        dependencies.completer = &completer;
        dependencies.completionModel = &completionModel;
        dependencies.diagnosticsPanel = &diagnostics;
        dependencies.syntaxController = &syntax;
        dependencies.documentCoordinator = &documents;
        EditorSessionController session(std::move(dependencies),
                                        std::move(callbacks));
        QSignalSpy documentSetSpy(
            &session, &EditorSessionController::documentSetChanged);

        QVERIFY(session.createTab(true) != nullptr);
        QVERIFY(session.openFilePath(firstPath));
        QTRY_VERIFY(routingRefreshes > 0);
        auto *untitled = session.createTab(false);
        QVERIFY(untitled != nullptr);
        QVERIFY(session.assignPath(untitled, secondPath));
        QCOMPARE(untitled->filePath(), secondPath);
        tabs.setCurrentWidget(untitled);
        const EditorSessionState captured = session.captureState();
        QCOMPARE(captured.openFiles,
                 QStringList({firstPath, secondPath}));
        QCOMPARE(captured.currentTab, 1);

        const int beforeFirstRestore = documentSetSpy.count();
        session.restoreState(captured);
        QCOMPARE(documentSetSpy.count(), beforeFirstRestore + 1);
        QCOMPARE(tabs.count(), 2);
        auto *restoredActive =
            qobject_cast<CodeEditor *>(tabs.currentWidget());
        QVERIFY(restoredActive != nullptr);
        QCOMPARE(restoredActive->filePath(), secondPath);

        const int beforeSecondRestore = documentSetSpy.count();
        session.restoreState(
            EditorSessionState{QStringList({secondPath}), 0});
        QCOMPARE(documentSetSpy.count(), beforeSecondRestore + 1);
        QCOMPARE(tabs.count(), 1);
        auto *restored = qobject_cast<CodeEditor *>(tabs.widget(0));
        QVERIFY(restored != nullptr);
        QCOMPARE(restored->filePath(), secondPath);
        QCOMPARE(restored->toPlainText(), QStringLiteral("cpp"));
    }

    void testEditorSessionControllerAppliesEditorPreferences() {
        QTabWidget tabs;
        SnippetManager snippets;
        LspCompletionModel completionModel;
        LspCompleter completer(&completionModel);
        DiagnosticsPanel diagnostics;
        EditorSyntaxController syntax;
        LspDocumentCoordinator documents;

        EditorSessionController::Dependencies dependencies;
        dependencies.tabWidget = &tabs;
        dependencies.snippetManager = &snippets;
        dependencies.completer = &completer;
        dependencies.completionModel = &completionModel;
        dependencies.diagnosticsPanel = &diagnostics;
        dependencies.syntaxController = &syntax;
        dependencies.documentCoordinator = &documents;
        EditorSessionController session(
            std::move(dependencies), EditorSessionController::Callbacks{});

        QFont font;
        font.setPointSize(17);
        session.setEditorPreferences({font, true, true});
        auto *editor = session.createTab();
        QVERIFY(editor != nullptr);
        QCOMPARE(editor->font().pointSize(), 17);
        QCOMPARE(editor->lineWrapMode(), QPlainTextEdit::WidgetWidth);
        QVERIFY(editor->vimMotionsEnabled());

        font.setPointSize(13);
        session.setEditorPreferences({font, false, false});
        QCOMPARE(editor->font().pointSize(), 13);
        QCOMPARE(editor->lineWrapMode(), QPlainTextEdit::NoWrap);
        QVERIFY(!editor->vimMotionsEnabled());
    }

    void testEditorSessionControllerSkipsMissingFilesWithoutLosingActiveFile() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString firstPath =
            QDir(tempDir.path()).filePath(QStringLiteral("first.zith"));
        const QString secondPath =
            QDir(tempDir.path()).filePath(QStringLiteral("second.zith"));
        for (const QString &path : {firstPath, secondPath}) {
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
            file.write(path == firstPath ? "first" : "second");
            file.close();
        }

        QTabWidget tabs;
        EditorSessionController::Callbacks callbacks;
        EditorSessionController::Dependencies dependencies;
        dependencies.tabWidget = &tabs;
        EditorSessionController session(std::move(dependencies),
                                        std::move(callbacks));

        const QString missingPath =
            QDir(tempDir.path()).filePath(QStringLiteral("missing.zith"));
        session.restoreState(
            EditorSessionState{{missingPath, secondPath, firstPath}, 1});

        QCOMPARE(tabs.count(), 2);
        QCOMPARE(qobject_cast<CodeEditor *>(tabs.currentWidget())->filePath(),
                 secondPath);
        QCOMPARE(qobject_cast<CodeEditor *>(tabs.widget(0))->filePath(),
                 secondPath);
        QCOMPARE(qobject_cast<CodeEditor *>(tabs.widget(1))->filePath(),
                 firstPath);
    }

    void testEditorSessionControllerFallsBackWhenActiveFileIsMissing() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString firstPath =
            QDir(tempDir.path()).filePath(QStringLiteral("first.zith"));
        const QString secondPath =
            QDir(tempDir.path()).filePath(QStringLiteral("second.zith"));
        for (const QString &path : {firstPath, secondPath}) {
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
            file.write("content");
            file.close();
        }

        QTabWidget tabs;
        EditorSessionController::Callbacks callbacks;
        EditorSessionController::Dependencies dependencies;
        dependencies.tabWidget = &tabs;
        EditorSessionController session(std::move(dependencies),
                                        std::move(callbacks));

        const QString missingPath =
            QDir(tempDir.path()).filePath(QStringLiteral("missing.zith"));
        session.restoreState(
            EditorSessionState{{firstPath, missingPath, secondPath}, 1});

        QCOMPARE(tabs.count(), 2);
        QCOMPARE(qobject_cast<CodeEditor *>(tabs.currentWidget())->filePath(),
                 firstPath);
    }

    void testEditorSessionControllerOwnsSavePolicy() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QTabWidget tabs;
        EditorSessionController::Callbacks callbacks;
        EditorSessionController::Dependencies dependencies;
        dependencies.tabWidget = &tabs;
        EditorSessionController session(std::move(dependencies),
                                        std::move(callbacks));
        QSignalSpy documentSetSpy(
            &session, &EditorSessionController::documentSetChanged);

        auto *untitled = session.createTab(true);
        QVERIFY(untitled != nullptr);
        untitled->setPlainText(QStringLiteral("saved content"));
        const QString untitledPath =
            QDir(tempDir.path()).filePath(QStringLiteral("saved.zith"));

        QCOMPARE(session.saveEditor(untitled, untitledPath),
                 EditorSessionController::SaveResult::Saved);
        QCOMPARE(untitled->filePath(), untitledPath);
        QVERIFY(!untitled->document()->isModified());
        QCOMPARE(documentSetSpy.count(), 1);

        QFile savedFile(untitledPath);
        QVERIFY(savedFile.open(QIODevice::ReadOnly | QIODevice::Text));
        QCOMPARE(QString::fromUtf8(savedFile.readAll()),
                 QStringLiteral("saved content"));

        untitled->setPlainText(QStringLiteral("updated content"));
        QCOMPARE(session.saveEditor(untitled),
                 EditorSessionController::SaveResult::Saved);
        QVERIFY(!untitled->document()->isModified());

        QVERIFY(session.saveEditor(session.createTab(false)) ==
                EditorSessionController::SaveResult::MissingPath);
    }

    void testEditorSessionControllerPublishesDocumentIdentityTransitions() {
        QTabWidget tabs;
        EditorSessionController::Callbacks callbacks;
        EditorSessionController::Dependencies dependencies;
        dependencies.tabWidget = &tabs;
        EditorSessionController session(std::move(dependencies),
                                        std::move(callbacks));
        QSignalSpy documentSetSpy(
            &session, &EditorSessionController::documentSetChanged);

        auto *editor = session.createTab(true);
        QVERIFY(editor != nullptr);
        const QString path = QStringLiteral("/tmp/assigned.zith");
        QVERIFY(session.assignPath(editor, path));
        QCOMPARE(documentSetSpy.count(), 1);
        QCOMPARE(editor->filePath(), path);

        CodeEditor detachedEditor;
        session.releaseEditor(&detachedEditor);
        QCOMPARE(documentSetSpy.count(), 1);

        session.releaseEditor(editor);
        QCOMPARE(documentSetSpy.count(), 2);
    }

    void testEditorFileControllerOwnsFileCommandPolicy() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString path =
            QDir(tempDir.path()).filePath(QStringLiteral("main.zith"));
        QTabWidget tabs;
        EditorSessionController::Dependencies dependencies;
        dependencies.tabWidget = &tabs;
        EditorSessionController session(
            std::move(dependencies), EditorSessionController::Callbacks{});
        auto *editor = session.createTab(true);
        QVERIFY(editor != nullptr);
        editor->setFilePath(path);
        editor->setPlainText(QStringLiteral("fn main() {}"));

        QString statusMessage;
        int statusTimeout = 0;
        int availabilityRefreshes = 0;
        EditorFileController::Callbacks callbacks;
        callbacks.workspaceRoot = [&]() { return tempDir.path(); };
        callbacks.showStatus =
            [&](const QString &message, int timeout) {
                statusMessage = message;
                statusTimeout = timeout;
            };
        callbacks.refreshCommandAvailability =
            [&]() { ++availabilityRefreshes; };
        EditorFileController files(nullptr, &tabs, &session,
                                   std::move(callbacks));

        QVERIFY(files.handleShellCommand(ShellCommand::SaveFile));
        QFile saved(path);
        QVERIFY(saved.open(QIODevice::ReadOnly | QIODevice::Text));
        QCOMPARE(QString::fromUtf8(saved.readAll()),
                 QStringLiteral("fn main() {}"));
        QCOMPARE(statusMessage, QStringLiteral("File saved"));
        QCOMPARE(statusTimeout, 2000);
        QCOMPARE(availabilityRefreshes, 1);

        QVERIFY(files.handleShellCommand(ShellCommand::NewFile));
        QCOMPARE(tabs.count(), 2);
        QVERIFY(!files.handleShellCommand(ShellCommand::Exit));
    }

    void testEditorInteractionControllerInterpretsVimCommands() {
        QTabWidget tabs;
        CodeEditor editor;
        tabs.addTab(&editor, QStringLiteral("Untitled"));

        int saveCount = 0;
        int closeCount = 0;
        int centralStateUpdates = 0;
        QString statusMessage;
        QString vimLabel;

        EditorInteractionController::Callbacks callbacks;
        callbacks.currentEditor = [&editor]() { return &editor; };
        callbacks.saveCurrent = [&]() {
            ++saveCount;
            editor.document()->setModified(false);
            return true;
        };
        callbacks.closeEditor = [&](CodeEditor *) { ++closeCount; };
        callbacks.updateCentralWidgetState =
            [&]() { ++centralStateUpdates; };
        callbacks.showStatus = [&](const QString &message, int) {
            statusMessage = message;
        };
        callbacks.setVimModeLabel =
            [&](const QString &text) { vimLabel = text; };

        EditorInteractionController::Dependencies dependencies;
        EditorInteractionController controller(std::move(dependencies),
                                                std::move(callbacks));
        controller.attach(&editor);
        editor.vimModeChanged(QStringLiteral("INSERT"));
        QCOMPARE(vimLabel, QStringLiteral("VIM: INSERT"));

        editor.setPlainText(QStringLiteral("unsaved"));
        editor.document()->setModified(true);
        controller.handleVimCommand(QStringLiteral("q"));
        QCOMPARE(statusMessage,
                 QStringLiteral(
                     "No write since last change (use :q! to force)."));
        QCOMPARE(closeCount, 0);

        controller.handleVimCommand(QStringLiteral("wq"));
        QCOMPARE(saveCount, 1);
        QCOMPARE(closeCount, 1);
        QCOMPARE(centralStateUpdates, 1);
    }

    void testEditorTabCloseControllerPreservesSaveOrdering() {
        QTabWidget tabs;
        CodeEditor editor;
        tabs.addTab(&editor, QStringLiteral("main.zith"));

        EditorTabCloseController::SaveDecision decision =
            EditorTabCloseController::SaveDecision::Cancel;
        int saveCount = 0;
        int releaseCount = 0;

        EditorTabCloseController::Callbacks callbacks;
        callbacks.requestSaveDecision =
            [&](CodeEditor *) { return decision; };
        callbacks.saveEditor = [&](CodeEditor *target) {
            ++saveCount;
            target->document()->setModified(false);
            return true;
        };
        callbacks.releaseEditor = [&](CodeEditor *) { ++releaseCount; };
        EditorTabCloseController controller(&tabs, std::move(callbacks));

        editor.document()->setModified(true);
        decision = EditorTabCloseController::SaveDecision::Cancel;
        emit tabs.tabCloseRequested(0);
        QCOMPARE(saveCount, 0);
        QCOMPARE(releaseCount, 0);

        decision = EditorTabCloseController::SaveDecision::Discard;
        emit tabs.tabCloseRequested(0);
        QCOMPARE(saveCount, 0);
        QCOMPARE(releaseCount, 1);

        editor.document()->setModified(true);
        decision = EditorTabCloseController::SaveDecision::Save;
        emit tabs.tabCloseRequested(0);
        QCOMPARE(saveCount, 1);
        QCOMPARE(releaseCount, 2);
        QVERIFY(!editor.document()->isModified());
    }

    void testEditorTabCloseControllerKeepsTabWhenSaveFails() {
        QTabWidget tabs;
        CodeEditor editor;
        tabs.addTab(&editor, QStringLiteral("main.zith"));

        EditorTabCloseController::Callbacks callbacks;
        callbacks.requestSaveDecision = [](CodeEditor *) {
            return EditorTabCloseController::SaveDecision::Save;
        };
        callbacks.saveEditor = [](CodeEditor *) { return false; };
        int releaseCount = 0;
        callbacks.releaseEditor = [&](CodeEditor *) { ++releaseCount; };
        EditorTabCloseController controller(&tabs, std::move(callbacks));

        editor.document()->setModified(true);
        emit tabs.tabCloseRequested(0);

        QCOMPARE(releaseCount, 0);
        QCOMPARE(tabs.count(), 1);
        QVERIFY(editor.document()->isModified());
    }

    void testEditorInteractionControllerKeepsEditorOnWqSaveFailure() {
        CodeEditor editor;
        int closeCount = 0;
        int saveCount = 0;

        EditorInteractionController::Callbacks callbacks;
        callbacks.currentEditor = [&editor]() { return &editor; };
        callbacks.saveCurrent = [&]() {
            ++saveCount;
            return false;
        };
        callbacks.closeEditor = [&](CodeEditor *) { ++closeCount; };

        EditorInteractionController::Dependencies dependencies;
        EditorInteractionController controller(std::move(dependencies),
                                                std::move(callbacks));
        editor.setPlainText(QStringLiteral("unsaved"));
        editor.document()->setModified(true);
        controller.handleVimCommand(QStringLiteral("wq"));

        QCOMPARE(saveCount, 1);
        QCOMPARE(closeCount, 0);
        QVERIFY(editor.document()->isModified());
    }

    void testLspEditorLifecycleDetachesOnlyOwnedEditors() {
        QTabWidget tabs;
        CodeEditor zithEditor;
        CodeEditor cppEditor;
        CodeEditor unrelatedEditor;
        LspClient zithClient;
        LspClient clangdClient;

        zithEditor.setFilePath(QStringLiteral("/tmp/main.zith"));
        cppEditor.setFilePath(QStringLiteral("/tmp/main.cpp"));
        unrelatedEditor.setFilePath(QStringLiteral("/tmp/notes.txt"));
        zithEditor.setLspClient(&zithClient);
        cppEditor.setLspClient(&clangdClient);

        tabs.addTab(&zithEditor, QStringLiteral("main.zith"));
        tabs.addTab(&cppEditor, QStringLiteral("main.cpp"));
        tabs.addTab(&unrelatedEditor, QStringLiteral("notes.txt"));

        LspDocumentCoordinator documents;
        LspEditorLifecycleController lifecycle(&tabs, &documents);
        lifecycle.detachDocumentsFor(&zithClient);

        QCOMPARE(zithEditor.lspClient(), nullptr);
        QCOMPARE(cppEditor.lspClient(), &clangdClient);
        QCOMPARE(unrelatedEditor.lspClient(), nullptr);
    }

    void testEditorLanguageFeaturesPreserveDocumentBindingAndDetach() {
        CodeEditor editor;
        LspClient client;
        editor.setFilePath(QStringLiteral("/tmp/main.zith"));
        editor.setInitialDocumentText(QStringLiteral("main\n"), 3);
        editor.setLspClient(&client);

        const LspDiagnostic diagnostic{
            LspRange{{0, 0}, {0, 4}}, 1, QStringLiteral("broken"),
            QStringLiteral("zith")};
        emit client.diagnosticsReceived(editor.fileUri(), 2, {diagnostic});
        QVERIFY(editor.diagnostics().isEmpty());

        emit client.diagnosticsReceived(editor.fileUri(), 3, {diagnostic});
        QCOMPARE(editor.diagnostics().size(), 1);

        QSignalSpy navigationSpy(&editor, &CodeEditor::navigateToLocation);
        const LspLocation location{
            QStringLiteral("file:///tmp/other.zith"),
            LspRange{{4, 2}, {4, 6}}};
        emit client.definitionResult(editor.fileUri(), 2, {location});
        QCOMPARE(navigationSpy.count(), 0);
        emit client.definitionResult(editor.fileUri(), 3, {location});
        QCOMPARE(navigationSpy.count(), 1);
        QCOMPARE(navigationSpy.at(0).at(0).toString(), location.uri);
        QCOMPARE(navigationSpy.at(0).at(1).toInt(), 4);
        QCOMPARE(navigationSpy.at(0).at(2).toInt(), 2);

        editor.setReadOnly(true);
        editor.clearDiagnostics();
        const QList<LspRange> highlights = {
            {LspPosition{0, 0}, LspPosition{0, 4}}};
        emit client.documentHighlightsResult(editor.fileUri(), 2, highlights);
        QCOMPARE(editor.extraSelections().size(), 0);
        emit client.documentHighlightsResult(editor.fileUri(), 3, highlights);
        QCOMPARE(editor.extraSelections().size(), 1);

        editor.detachLspClient();
        emit client.diagnosticsReceived(editor.fileUri(), 3, {});
        emit client.definitionResult(editor.fileUri(), 3, {location});
        emit client.documentHighlightsResult(editor.fileUri(), 3, highlights);
        QVERIFY(editor.diagnostics().isEmpty());
        QCOMPARE(editor.extraSelections().size(), 0);
        QCOMPARE(navigationSpy.count(), 1);
    }

    void testEditorLanguageFeatureAvailabilityCentralizesCapabilities() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString serverPath =
            QDir(tempDir.path()).filePath(QStringLiteral("lsp-stub"));
        QFile serverFile(serverPath);
        QVERIFY(serverFile.open(QIODevice::WriteOnly));
        serverFile.write(
            "#!/bin/sh\n"
            "while IFS= read -r line; do\n"
            "  case \"$line\" in\n"
            "    *exit*) exit 0 ;;\n"
            "  esac\n"
            "done\n");
        serverFile.close();
        QVERIFY(QFile::setPermissions(
            serverPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                            QFileDevice::ExeOwner));

        CodeEditor editor;
        LspClient client;
        EditorLanguageFeatureController controller(&editor);

        QVERIFY(!controller.isAvailable(
            EditorLanguageFeatureController::Feature::Definition));

        controller.setClient(&client);
        QVERIFY(!controller.isAvailable(
            EditorLanguageFeatureController::Feature::Definition));

        QVERIFY(client.start(serverPath));
        QTRY_VERIFY_WITH_TIMEOUT(client.isRunning(), 5000);
        client.setReadyForTesting(true);
        client.m_capabilities.definitionProvider = true;
        client.m_capabilities.hoverProvider = true;
        client.m_capabilities.signatureHelpProvider = false;
        client.m_capabilities.formattingProvider = false;

        QVERIFY(controller.isAvailable(
            EditorLanguageFeatureController::Feature::Definition));
        QVERIFY(controller.isAvailable(
            EditorLanguageFeatureController::Feature::Hover));
        QVERIFY(!controller.isAvailable(
            EditorLanguageFeatureController::Feature::SignatureHelp));
        QVERIFY(!controller.isAvailable(
            EditorLanguageFeatureController::Feature::Formatting));

        client.setReadyForTesting(false);
        client.stop();
        client.waitForFinishedForTesting(5000);
    }

    void testEditorLanguageRequestContextOwnsDocumentInvariants() {
        CodeEditor editor;

        QVERIFY(!editor.currentLanguageRequest().isValid());

        editor.setFilePath(QStringLiteral("/tmp/main.zith"));
        editor.setInitialDocumentText(QStringLiteral("main\n"), 7);
        QTextCursor cursor = editor.textCursor();
        cursor.setPosition(2);
        editor.setTextCursor(cursor);

        const EditorLanguageRequestContext request =
            editor.currentLanguageRequest();
        QVERIFY(request.isValid());
        QCOMPARE(request.uri, editor.fileUri());
        QCOMPARE(request.version, 7);
        QCOMPARE(request.position.line, 0);
        QCOMPARE(request.position.character, 2);
    }

    void testLspRestartPolicyUsesBoundedExponentialBackoff() {
        LspRestartPolicy policy;

        QCOMPARE(policy.decide(1000, true, true, true).action,
                 LspRestartPolicy::Action::Ignore);
        QCOMPARE(policy.decide(1000, false, false, true).action,
                 LspRestartPolicy::Action::Ignore);
        QCOMPARE(policy.decide(1000, false, true, false).action,
                 LspRestartPolicy::Action::Ignore);

        const auto first = policy.decide(1000, false, true, true);
        QCOMPARE(first.action, LspRestartPolicy::Action::Restart);
        QCOMPARE(first.attempt, 1);
        QCOMPARE(first.delaySeconds, 1);

        const auto second = policy.decide(2000, false, true, true);
        QCOMPARE(second.attempt, 2);
        QCOMPARE(second.delaySeconds, 2);

        const auto third = policy.decide(3000, false, true, true);
        QCOMPARE(third.attempt, 3);
        QCOMPARE(third.delaySeconds, 4);

        QCOMPARE(policy.decide(4000, false, true, true).action,
                 LspRestartPolicy::Action::GiveUp);
        const auto afterWindow =
            policy.decide(65000, false, true, true);
        QCOMPARE(afterWindow.action, LspRestartPolicy::Action::Restart);
        QCOMPARE(afterWindow.attempt, 1);
        QCOMPARE(afterWindow.delaySeconds, 1);
    }

    void testEditorLspActionsRejectUnavailableClients() {
        QTabWidget tabs;
        CodeEditor editor;
        LspClient client;
        editor.setFilePath(QStringLiteral("/tmp/main.zith"));
        editor.setLspClient(&client);
        tabs.addTab(&editor, QStringLiteral("main.zith"));

        EditorLspActionController actions(&tabs);
        const LspPosition position{1, 2};
        const LspRange range{position, position};

        QVERIFY(!actions.requestRename(&editor, editor.fileUri(), 1,
                                       position, QStringLiteral("renamed")));
        QVERIFY(!actions.requestCodeActions(&editor, editor.fileUri(), 1,
                                            range));
        QVERIFY(actions.handleShellCommand(ShellCommand::FormatDocument));
        QVERIFY(!actions.handleShellCommand(ShellCommand::RestartLsp));
    }

    void testLspEditorResultRouterRejectsStaleDocumentResults() {
        QTabWidget tabs;
        OutlinePanel outline;
        CodeEditor editor;
        LspClient client;

        editor.setFilePath(QStringLiteral("/tmp/main.zith"));
        editor.setInitialDocumentText(QStringLiteral("old\n"), 3);
        tabs.addTab(&editor, QStringLiteral("main.zith"));

        LspEditorResultRouter router(&tabs, &outline);
        router.attach(&client);

        const QList<QPair<LspRange, QString>> edits = {
            {LspRange{{0, 0}, {0, 3}}, QStringLiteral("new")}};
        emit client.formattingResult(editor.fileUri(), 2, edits);
        QCOMPARE(editor.toPlainText(), QStringLiteral("old\n"));

        emit client.formattingResult(editor.fileUri(), 3, edits);
        QCOMPARE(editor.toPlainText(), QStringLiteral("new\n"));

        QJsonArray symbols{
            QJsonObject{{QStringLiteral("name"), QStringLiteral("main")},
                        {QStringLiteral("kind"), 12},
                        {QStringLiteral("range"),
                         QJsonObject{
                             {QStringLiteral("start"),
                              QJsonObject{{QStringLiteral("line"), 0},
                                          {QStringLiteral("character"), 0}}}}}}};
        emit client.documentSymbolsResult(editor.fileUri(), 2, symbols);
        auto *tree = outline.findChild<QTreeWidget *>();
        QVERIFY(tree != nullptr);
        QCOMPARE(tree->topLevelItemCount(), 0);

        emit client.documentSymbolsResult(editor.fileUri(), 3, symbols);
        QCOMPARE(tree->topLevelItemCount(), 1);
        QCOMPARE(tree->topLevelItem(0)->text(0), QStringLiteral("main"));
    }

    void testLspReferencesRouterRejectsStaleAndInactiveResults() {
        QTabWidget tabs;
        ReferencesPanel references;
        CodeEditor first;
        CodeEditor second;
        LspClient client;
        int showCount = 0;

        first.setFilePath(QStringLiteral("/tmp/first.zith"));
        first.setInitialDocumentText(QStringLiteral("first\n"), 4);
        second.setFilePath(QStringLiteral("/tmp/second.zith"));
        second.setInitialDocumentText(QStringLiteral("second\n"), 2);
        tabs.addTab(&first, QStringLiteral("first.zith"));
        tabs.addTab(&second, QStringLiteral("second.zith"));
        tabs.setCurrentWidget(&first);

        LspReferencesRouter::Dependencies dependencies;
        dependencies.tabWidget = &tabs;
        dependencies.referencesPanel = &references;
        LspReferencesRouter router(
            std::move(dependencies), [&showCount]() { ++showCount; });
        router.attach(&client);

        const LspLocation location{
            QUrl::fromLocalFile(QStringLiteral("/tmp/target.zith")).toString(),
            LspRange{{2, 3}, {2, 3}}};
        emit client.referencesResult(first.fileUri(), 3, {location});
        QCOMPARE(references.findChild<QListWidget *>()->count(), 0);
        QCOMPARE(showCount, 0);

        emit client.referencesResult(second.fileUri(), 2, {location});
        QCOMPARE(references.findChild<QListWidget *>()->count(), 0);
        QCOMPARE(showCount, 0);

        emit client.referencesResult(first.fileUri(), 4, {location});
        QCOMPARE(references.findChild<QListWidget *>()->count(), 1);
        QCOMPARE(showCount, 1);
    }

    void testLspCodeActionRouterBuildsAndExecutesEditActions() {
        QWidget parent;
        LspClient client;
        QJsonObject appliedEdit;
        int presentedActions = 0;
        QString presentedTitle;

        LspCodeActionRouter::Callbacks callbacks;
        callbacks.applyWorkspaceEdit =
            [&appliedEdit](const QJsonObject &edit) { appliedEdit = edit; };
        callbacks.executeCommand = [](LspClient *, const QJsonObject &) {};
        callbacks.presentMenu =
            [&presentedActions, &presentedTitle](QMenu *menu) {
                presentedActions = menu->actions().size();
                if (presentedActions == 1) {
                    presentedTitle = menu->actions().first()->text();
                    menu->actions().first()->trigger();
                }
            };
        LspCodeActionRouter router(&parent, std::move(callbacks));
        router.attach(&client);

        const QJsonObject edit{{QStringLiteral("changes"),
                                QJsonObject{{QStringLiteral("file:///tmp/a.zith"),
                                             QJsonArray{}}}}};
        emit client.codeActionsResult(
            QStringLiteral("file:///tmp/main.zith"), 1,
            QJsonArray{QJsonObject{{QStringLiteral("title"),
                                    QStringLiteral("Extract method")},
                                   {QStringLiteral("edit"), edit}}});

        QCOMPARE(presentedActions, 1);
        QCOMPARE(presentedTitle, QStringLiteral("Extract method"));
        QCOMPARE(appliedEdit, edit);
    }

    void testLocationNavigatorNormalizesLocalUrisAndRejectsRemoteTargets() {
        CodeEditor editor;
        const QString path = QStringLiteral("/tmp/navigation.zith");
        editor.setFilePath(path);
        editor.setInitialDocumentText(QStringLiteral("one\ntarget line\n"));

        QString openedPath;
        LocationNavigator navigator(
            [&openedPath](const QString &opened) { openedPath = opened; },
            [&editor]() { return &editor; });

        QVERIFY(navigator.navigateToUri(QUrl::fromLocalFile(path).toString(),
                                       1, 3));
        QCOMPARE(openedPath, path);
        QCOMPARE(editor.textCursor().blockNumber(), 1);
        QCOMPARE(editor.textCursor().positionInBlock(), 3);

        openedPath.clear();
        QVERIFY(!navigator.navigateToUri(QStringLiteral("https://example.com/a"),
                                         0, 0));
        QVERIFY(openedPath.isEmpty());
    }

    void testLocationNavigatorAttachesOutlineSelection() {
        CodeEditor editor;
        editor.setInitialDocumentText(QStringLiteral("zero\none\ntwo\n"));
        OutlinePanel outline;

        LocationNavigator navigator(
            [](const QString &) {},
            [&editor]() { return &editor; });
        navigator.attach(&outline);

        emit outline.symbolSelected(2, 1);

        QCOMPARE(editor.textCursor().blockNumber(), 2);
        QCOMPARE(editor.textCursor().positionInBlock(), 1);
    }

    void testWorkspaceNavigationControllerRoutesWorkspaceIntents() {
        FileTreePanel fileTree;
        WelcomeWidget welcome;
        GitPanel git;

        QString openedPath;
        QString openedInNewTabPath;
        QString selectedRoot;
        int openFolderRequests = 0;
        int newProjectRequests = 0;

        WorkspaceNavigationController::Callbacks callbacks;
        callbacks.openFile = [&openedPath](const QString &path) {
            openedPath = path;
        };
        callbacks.openFileInNewTab =
            [&openedInNewTabPath](const QString &path) {
                openedInNewTabPath = path;
            };
        callbacks.selectWorkspaceRoot =
            [&selectedRoot](const QString &path) { selectedRoot = path; };
        callbacks.openFolder = [&openFolderRequests]() {
            ++openFolderRequests;
        };
        callbacks.newProject = [&newProjectRequests]() {
            ++newProjectRequests;
        };

        WorkspaceNavigationController controller(
            &fileTree, &welcome, &git, std::move(callbacks));

        emit fileTree.fileActivated(QStringLiteral("/tmp/tree.zith"));
        emit fileTree.fileActivatedInNewTab(
            QStringLiteral("/tmp/tree-new-tab.zith"));
        emit fileTree.projectRootChanged(QStringLiteral("/tmp/tree-root"));
        emit welcome.openFolderRequested();
        emit welcome.newProjectRequested();
        emit welcome.projectSelected(QStringLiteral("/tmp/recent-root"));
        emit git.fileActivated(QStringLiteral("/tmp/git.zith"));

        QCOMPARE(openedPath, QStringLiteral("/tmp/git.zith"));
        QCOMPARE(openedInNewTabPath,
                 QStringLiteral("/tmp/tree-new-tab.zith"));
        QCOMPARE(selectedRoot, QStringLiteral("/tmp/recent-root"));
        QCOMPARE(openFolderRequests, 1);
        QCOMPARE(newProjectRequests, 1);

        QVERIFY(controller.handleShellCommand(ShellCommand::OpenFolder));
        QVERIFY(controller.handleShellCommand(ShellCommand::NewProject));
        QCOMPARE(openFolderRequests, 2);
        QCOMPARE(newProjectRequests, 2);
        QVERIFY(!controller.handleShellCommand(ShellCommand::OpenFile));
    }

    void testGitPanelReplacesStatusSnapshotAndRendersThemeAgain() {
        GitPanel panel;
        panel.setRootPath(QStringLiteral("/tmp/helios-git"));

        auto *session = panel.findChild<GitRepositorySession *>();
        auto *list = panel.findChild<QListWidget *>();
        QVERIFY(session);
        QVERIFY(list);

        GitStatusSnapshot snapshot;
        snapshot.branch = QStringLiteral("main");
        snapshot.entries = {
            {QStringLiteral(" M"), QStringLiteral("editor/Code.cpp")},
            {QStringLiteral("??"), QStringLiteral("notes.md")}};

        emit session->stateChanged(
            GitRepositoryState{QStringLiteral("/tmp/helios-git"),
                               snapshot.branch,
                               snapshot.entries,
                               true,
                               false,
                               false});
        QCOMPARE(list->count(), 2);
        list->item(0)->setSelected(true);

        emit session->stateChanged(
            GitRepositoryState{QStringLiteral("/tmp/helios-git"),
                               snapshot.branch,
                               snapshot.entries,
                               true,
                               false,
                               false});
        QCOMPARE(list->count(), 2);
        QVERIFY(list->item(0)->isSelected());

        panel.applyTheme();
        QCOMPARE(list->count(), 2);
        QVERIFY(list->item(0)->isSelected());
        QCOMPARE(list->item(0)->data(Qt::UserRole + 1).toString(),
                 QStringLiteral("editor/Code.cpp"));
    }

    void testGitStatusListPresenterProjectsSnapshotAndSelection() {
        QListWidget list;
        list.setSelectionMode(QAbstractItemView::ExtendedSelection);
        GitStatusListPresenter presenter(&list);

        const GitRepositoryState state{
            QStringLiteral("/tmp/helios-git"),
            QStringLiteral("main"),
            {{QStringLiteral(" M"), QStringLiteral("editor/Code.cpp")},
             {QStringLiteral("??"), QStringLiteral("notes.md")}},
            true,
            false,
            false};
        presenter.render(state);

        QCOMPARE(list.count(), 2);
        list.item(1)->setSelected(true);
        QCOMPARE(presenter.selectedRelativePaths(),
                 QStringList{QStringLiteral("notes.md")});

        presenter.render(state);
        QCOMPARE(list.count(), 2);
        QVERIFY(list.item(1)->isSelected());
        QCOMPARE(list.item(0)->data(Qt::UserRole + 1).toString(),
                 QStringLiteral("editor/Code.cpp"));

        presenter.applyTheme();
        QCOMPARE(list.count(), 2);
        QVERIFY(list.item(1)->isSelected());

        presenter.clear();
        QCOMPARE(list.count(), 0);
        QVERIFY(presenter.selectedRelativePaths().isEmpty());
    }

    void testGitPanelPresentationModelProjectsRepositoryState() {
        const GitPanelPresentationState noRepository =
            GitPanelPresentationModel::fromRepositoryState(
                GitRepositoryState{QStringLiteral("/tmp/project"),
                                   QString(), {}, false, false, false});
        QCOMPARE(noRepository.branchLabel, QStringLiteral("No Repo"));
        QCOMPARE(noRepository.summaryMessage,
                 QStringLiteral("Open a project inside a Git repository."));
        QVERIFY(noRepository.showInitializeButton);
        QVERIFY(!noRepository.showConnectButton);
        QVERIFY(!noRepository.showStatusList);

        const GitPanelPresentationState changedRepository =
            GitPanelPresentationModel::fromRepositoryState(
                GitRepositoryState{QStringLiteral("/tmp/project"),
                                   QStringLiteral("main"),
                                   {{QStringLiteral(" M"),
                                     QStringLiteral("main.cpp")}},
                                   true, false, true});
        QCOMPARE(changedRepository.branchLabel, QStringLiteral("main"));
        QCOMPARE(changedRepository.summaryMessage,
                 QStringLiteral("1 changed file(s). Select files to stage."));
        QVERIFY(!changedRepository.showInitializeButton);
        QVERIFY(changedRepository.showConnectButton);
        QVERIFY(changedRepository.showStatusList);
        QVERIFY(changedRepository.busy);

        const GitPanelPresentationState connectedRepository =
            GitPanelPresentationModel::fromRepositoryState(
                GitRepositoryState{QStringLiteral("/tmp/project"),
                                   QStringLiteral("main"), {}, true, true,
                                   false});
        QCOMPARE(connectedRepository.summaryMessage,
                 QStringLiteral("Repository is clean."));
        QVERIFY(!connectedRepository.showConnectButton);
    }

    void testEditorChromeControllerUpdatesAndClearsEditorChrome() {
        BreadcrumbsBar breadcrumbs;
        FindReplaceBar findReplace;
        OutlinePanel outline;
        QString position;
        QString language;
        CodeEditor editor;
        editor.setFilePath(QStringLiteral("/tmp/main.zith"));
        editor.setInitialDocumentText(QStringLiteral("first\nsecond\n"));
        editor.goToLine(1, 2);

        QString title;
        EditorChromeController chrome(
            &breadcrumbs, &findReplace, &outline,
            [](const QString &path) {
                return path.endsWith(QStringLiteral(".zith"))
                           ? QStringLiteral("Zith")
                           : QStringLiteral("Plain Text");
            },
            [&editor]() { return &editor; },
            [](const QString &, int) {},
            [&position](int line, int column) {
                position = QStringLiteral("Ln %1, Col %2").arg(line).arg(column);
            },
            [&language](const QString &value) { language = value; },
            [&title](const QString &value) { title = value; });

        chrome.update(&editor);
        QCOMPARE(findReplace.editor(), &editor);
        QCOMPARE(language, QStringLiteral("Zith"));
        QCOMPARE(position, QStringLiteral("Ln 2, Col 3"));
        QCOMPARE(title, QStringLiteral("main.zith — Helios"));

        chrome.update(nullptr);
        QCOMPARE(findReplace.editor(), nullptr);
        QCOMPARE(position, QStringLiteral("Ln 1, Col 1"));
        QCOMPARE(language, QString());
        QCOMPARE(title, QStringLiteral("Helios"));
    }

    void testLanguageServiceFeedbackControllerProjectsClientEvents() {
        DiagnosticsPanel diagnostics;
        LspClient zithClient;
        LspClientEventSource zithEvents(&zithClient);
        LspEventSource fakeEvents;

        QJsonObject appliedEdit;
        QString statusMessage;
        int statusTimeout = 0;
        int errorCount = -1;
        int warningCount = -1;

        LanguageServiceFeedbackController::Callbacks callbacks;
        callbacks.applyWorkspaceEdit =
            [&](const QJsonObject &edit) { appliedEdit = edit; };
        callbacks.showStatus = [&](const QString &message, int timeout) {
            statusMessage = message;
            statusTimeout = timeout;
        };
        callbacks.updateDiagnosticCounts =
            [&](int errors, int warnings) {
                errorCount = errors;
                warningCount = warnings;
            };

        LanguageServiceFeedbackController controller(&diagnostics,
                                                     std::move(callbacks));
        controller.attach(&zithEvents);
        controller.attach(&fakeEvents);

        const LspDiagnostic error{
            LspRange{{1, 2}, {1, 3}}, 1, QStringLiteral("broken"),
            QStringLiteral("zith")};
        emit zithClient.diagnosticsReceived(
            QStringLiteral("file:///tmp/main.zith"), 1, {error});
        QCOMPARE(diagnostics.errorCount(), 1);
        QCOMPARE(diagnostics.warningCount(), 0);
        QCOMPARE(errorCount, 1);
        QCOMPARE(warningCount, 0);

        const QJsonObject edit{{QStringLiteral("changes"), QJsonObject{}}};
        emit fakeEvents.renameResult(QStringLiteral("file:///tmp/main.cpp"), 1,
                                     edit);
        QCOMPARE(appliedEdit, edit);

        emit fakeEvents.showMessage(QStringLiteral("server notice"));
        QCOMPARE(statusMessage, QStringLiteral("server notice"));
        QCOMPARE(statusTimeout, 5000);
    }

    void testStatusBarControllerOwnsPresentationState() {
        QStatusBar statusBar;
        StatusBarController controller(&statusBar, true);

        controller.setContext(1, 3);
        controller.setLspStatus(QStringLiteral("LSP ○"),
                                QStringLiteral("#123456"));
        controller.setVimMode(QStringLiteral("VIM: INSERT"));
        controller.setDiagnostics(2, 4);
        controller.setEditorPosition(7, 9);
        controller.setLanguage(QStringLiteral("C/C++"));

        const auto labels = statusBar.findChildren<QLabel *>();
        QVERIFY(labels.size() >= 8);
        bool foundContext = false;
        bool foundLsp = false;
        bool foundVim = false;
        bool foundDiagnostics = false;
        bool foundPosition = false;
        bool foundLanguage = false;
        for (const QLabel *label : labels) {
            foundContext |= label->text() == QStringLiteral("2/3");
            foundLsp |= label->text() == QStringLiteral("LSP ○");
            foundVim |= label->text() == QStringLiteral("VIM: INSERT");
            foundDiagnostics |= label->text() == QStringLiteral("✕2  ⚠4");
            foundPosition |= label->text() == QStringLiteral("Ln 7, Col 9");
            foundLanguage |= label->text() == QStringLiteral("C/C++");
        }
        QVERIFY(foundContext);
        QVERIFY(foundLsp);
        QVERIFY(foundVim);
        QVERIFY(foundDiagnostics);
        QVERIFY(foundPosition);
        QVERIFY(foundLanguage);
    }

    void testLspRuntimePresentationProjectsFrontendEvents() {
        QStatusBar statusBar;
        StatusBarController statusBarController(&statusBar, true);
        SettingsPanel settingsPanel;
        LspManagerDialog managerDialog;
        LspLogPresenter logPresenter(
            &settingsPanel, &managerDialog, []() { return true; });
        LspRuntimePresentationController::Dependencies dependencies;
        dependencies.settingsPanel = &settingsPanel;
        dependencies.lspManagerDialog = &managerDialog;
        dependencies.logPresenter = &logPresenter;
        dependencies.statusBar = &statusBarController;
        LspRuntimePresentationController controller(
            std::move(dependencies),
            []() { return true; }, []() { return false; },
            []() { return QString(); });

        controller.presentRuntimeStatus(
            LspRuntimePresentationController::RuntimeStatus::Starting);
        bool foundStartingStatus = false;
        for (const QLabel *label : statusBar.findChildren<QLabel *>())
            foundStartingStatus |= label->text() == QStringLiteral("LSP ○");
        QVERIFY(foundStartingStatus);

        QJsonObject frontendStatus;
        frontendStatus.insert(QStringLiteral("state"), QStringLiteral("ready"));
        frontendStatus.insert(QStringLiteral("message"),
                              QStringLiteral("frontend ready"));
        controller.presentFrontendStatus(frontendStatus);

        bool foundReadyStatus = false;
        for (const QLabel *label : statusBar.findChildren<QLabel *>())
            foundReadyStatus |= label->text() == QStringLiteral("LSP ⬤");
        QVERIFY(foundReadyStatus);

        QJsonObject metrics;
        metrics.insert(QStringLiteral("requests"), 3);
        controller.presentMetrics(metrics);

        bool foundSettingsLog = false;
        for (const QPlainTextEdit *log :
             settingsPanel.findChildren<QPlainTextEdit *>())
            foundSettingsLog |= log->toPlainText().contains(QStringLiteral("Metrics:"));
        QVERIFY(foundSettingsLog);

        bool foundManagerLog = false;
        for (const QPlainTextEdit *log :
             managerDialog.findChildren<QPlainTextEdit *>())
            foundManagerLog |= log->toPlainText().contains(QStringLiteral("Metrics:"));
        QVERIFY(foundManagerLog);
    }

    void testLspRuntimePresentationModelUsesTranslatedFallbacks() {
        auto &tr = TranslationManager::instance();
        const QString previousLocale = tr.currentLocale();
        QVERIFY(tr.loadLocale("pt-BR"));

        LspRuntimePresentationModel model;
        model.setRuntimeInfo({});
        model.setDiagnostics({});
        model.setClangdInfo({});

        const LspRuntimeInfo runtime = model.displayRuntimeInfo();
        QCOMPARE(runtime.status, QStringLiteral("Indisponível"));
        QCOMPARE(runtime.cachePath, QStringLiteral("Indisponível"));

        const LspDiagnosticsInfo diagnostics = model.displayDiagnostics();
        QCOMPARE(diagnostics.connection, QStringLiteral("Desconhecido"));
        QCOMPARE(diagnostics.lastError, QStringLiteral("Nenhum"));

        const ClangdInfo clangd = model.displayClangdInfo();
        QCOMPARE(clangd.status, QStringLiteral("Indisponível"));
        QCOMPARE(clangd.resolvedPath, QStringLiteral("Indisponível"));

        QVERIFY(tr.loadLocale(previousLocale.isEmpty()
                                  ? QStringLiteral("en-US")
                                  : previousLocale));
    }

    void testLspRuntimePresentationModelAcceptsTranslationPolicy() {
        LspRuntimePresentationModel model(
            [](const QString &key) {
                return QStringLiteral("translated:") + key;
            });
        model.setRuntimeInfo({});
        model.setDiagnostics({});
        model.setClangdInfo({});

        QCOMPARE(model.displayRuntimeInfo().status,
                 QStringLiteral("translated:lsp.val_unavailable"));
        QCOMPARE(model.displayDiagnostics().connection,
                 QStringLiteral("translated:lsp.val_unknown"));
        QCOMPARE(model.displayDiagnostics().lastError,
                 QStringLiteral("translated:lsp.val_none"));
        QCOMPARE(model.displayClangdInfo().resolvedPath,
                 QStringLiteral("translated:lsp.val_unavailable"));
    }

    void testLspRuntimePanelsRefreshFallbacksWhenLocaleChanges() {
        auto &tr = TranslationManager::instance();
        const QString previousLocale = tr.currentLocale();
        QVERIFY(tr.loadLocale("en-US"));

        SettingsPanel settingsPanel;
        LspManagerDialog managerDialog;
        settingsPanel.setRuntimeInfo({});
        settingsPanel.setLspDiagnostics({});
        settingsPanel.setCLspInfo({});
        managerDialog.setRuntimeInfo({});
        managerDialog.setLspDiagnostics({});
        managerDialog.setCLspInfo({});

        QVERIFY(tr.loadLocale("pt-BR"));

        const auto countText = [](const QWidget *root,
                                  const QString &text) {
            int count = 0;
            for (const QLabel *label : root->findChildren<QLabel *>())
                count += label->text() == text ? 1 : 0;
            return count;
        };

        for (const QWidget *panel : {
                 static_cast<const QWidget *>(&settingsPanel),
                 static_cast<const QWidget *>(&managerDialog)}) {
            QVERIFY(countText(panel, QStringLiteral("Indisponível")) >= 7);
            QVERIFY(countText(panel, QStringLiteral("Desconhecido")) >= 2);
            QVERIFY(countText(panel, QStringLiteral("Nenhum")) >= 1);
        }

        QVERIFY(tr.loadLocale(previousLocale.isEmpty()
                                  ? QStringLiteral("en-US")
                                  : previousLocale));
    }

    void testLspRuntimePanelsProjectNamedPresentationState() {
        SettingsPanel settingsPanel;
        LspManagerDialog managerDialog;

        const LspRuntimeInfo runtimeInfo{
            QStringLiteral("Ready"),
            QStringLiteral("v0.8.0"),
            QStringLiteral("/tmp/zith-lsp"),
            QStringLiteral("/tmp/zith-stdlib"),
            QStringLiteral("/tmp/zith-cache")};
        const ClangdInfo clangdInfo{
            QStringLiteral("Connected"),
            QStringLiteral("/usr/bin/clangd"),
            QStringLiteral("compile commands loaded")};
        const LspDiagnosticsInfo diagnosticsInfo{
            QStringLiteral("Connected"),
            QStringLiteral("Incremental"),
            QStringLiteral("No errors")};

        settingsPanel.setRuntimeInfo(runtimeInfo);
        settingsPanel.setCLspInfo(clangdInfo);
        settingsPanel.setLspDiagnostics(diagnosticsInfo);
        managerDialog.setRuntimeInfo(runtimeInfo);
        managerDialog.setCLspInfo(clangdInfo);
        managerDialog.setLspDiagnostics(diagnosticsInfo);

        const auto containsText = [](const QWidget *root,
                                     const QString &text) {
            for (const QLabel *label : root->findChildren<QLabel *>()) {
                if (label->text() == text)
                    return true;
            }
            return false;
        };

        for (const QWidget *panel : {static_cast<const QWidget *>(&settingsPanel),
                                     static_cast<const QWidget *>(&managerDialog)}) {
            QVERIFY(containsText(panel, runtimeInfo.status));
            QVERIFY(containsText(panel, runtimeInfo.tag));
            QVERIFY(containsText(panel, runtimeInfo.lspPath));
            QVERIFY(containsText(panel, runtimeInfo.stdlibPath));
            QVERIFY(containsText(panel, runtimeInfo.cachePath));
            QVERIFY(containsText(panel, clangdInfo.status));
            QVERIFY(containsText(panel, clangdInfo.resolvedPath));
            QVERIFY(containsText(panel, clangdInfo.message));
            QVERIFY(containsText(panel, diagnosticsInfo.connection));
            QVERIFY(containsText(panel, diagnosticsInfo.syncMode));
            QVERIFY(containsText(panel, diagnosticsInfo.lastError));
        }
    }

    void testLspLogPresenterProjectsAccordingToLspEnablement() {
        SettingsPanel settingsPanel;
        LspManagerDialog managerDialog;
        bool enabled = false;
        LspLogPresenter presenter(
            &settingsPanel, &managerDialog, [&]() { return enabled; });

        presenter.append(QStringLiteral("disabled message"));

        bool settingsReceivedDisabled = false;
        for (const QPlainTextEdit *log :
             settingsPanel.findChildren<QPlainTextEdit *>())
            settingsReceivedDisabled |=
                log->toPlainText().contains(QStringLiteral("disabled message"));
        QVERIFY(!settingsReceivedDisabled);

        bool managerReceivedDisabled = false;
        for (const QPlainTextEdit *log :
             managerDialog.findChildren<QPlainTextEdit *>())
            managerReceivedDisabled |=
                log->toPlainText().contains(QStringLiteral("disabled message"));
        QVERIFY(managerReceivedDisabled);

        enabled = true;
        presenter.append(QStringLiteral("enabled message"));

        bool settingsReceivedEnabled = false;
        for (const QPlainTextEdit *log :
             settingsPanel.findChildren<QPlainTextEdit *>())
            settingsReceivedEnabled |=
                log->toPlainText().contains(QStringLiteral("enabled message"));
        QVERIFY(settingsReceivedEnabled);
    }

    void testLspRuntimeEventControllerProjectsLifecycleEvents() {
        QStatusBar statusBar;
        StatusBarController statusBarController(&statusBar, true);
        ZithRuntimeLifecycleCoordinator runtime;
        LspClient clangd;
        LspClientEventSource zithEvents(runtime.client());
        LspClientEventSource clangdEvents(&clangd);
        ClangdLifecycleCoordinator clangdLifecycle;
        clangdLifecycle.setClient(&clangd);
        LspManagerDialog managerDialog;
        LspLogPresenter logPresenter(
            nullptr, &managerDialog, []() { return true; });
        LspRuntimePresentationController::Dependencies presentationDependencies;
        presentationDependencies.logPresenter = &logPresenter;
        presentationDependencies.statusBar = &statusBarController;
        presentationDependencies.zithRuntime = &runtime;
        presentationDependencies.zithLspClient = runtime.client();
        presentationDependencies.clangdLifecycle = &clangdLifecycle;
        LspRuntimePresentationController presentation(
            std::move(presentationDependencies),
            []() { return true; },
            []() { return false; }, []() { return QString(); });

        LspRuntimeController::Callbacks runtimeCallbacks;
        LspRuntimeController::Dependencies runtimeDependencies;
        runtimeDependencies.zithRuntime = &runtime;
        runtimeDependencies.clangdClient = &clangd;
        runtimeDependencies.presentation = &presentation;
        LspRuntimeController runtimeController(
            std::move(runtimeDependencies), std::move(runtimeCallbacks));

        int actionRefreshes = 0;
        QString lastLog;
        LspRuntimeEventController::Callbacks callbacks;
        callbacks.lspEnabled = []() { return true; };
        callbacks.refreshActions = [&]() { ++actionRefreshes; };

        LspRuntimeEventController::Dependencies dependencies;
        dependencies.zithRuntime = &runtime;
        dependencies.zithClient = runtime.client();
        dependencies.zithEvents = &zithEvents;
        dependencies.clangdClient = &clangd;
        dependencies.clangdEvents = &clangdEvents;
        dependencies.clangdLifecycle = &clangdLifecycle;
        dependencies.runtimeController = &runtimeController;
        dependencies.presentation = &presentation;
        dependencies.logPresenter = &logPresenter;
        dependencies.statusBar = &statusBarController;
        LspRuntimeEventController events(
            std::move(dependencies), std::move(callbacks));
        events.attach();

        emit runtime.connected();
        QVERIFY(actionRefreshes > 0);

        emit runtime.serverError(QStringLiteral("runtime failed"));
        QCOMPARE(runtimeController.lastError(),
                 QStringLiteral("runtime failed"));
        bool foundErrorLog = false;
        for (const QPlainTextEdit *log :
             managerDialog.findChildren<QPlainTextEdit *>())
            foundErrorLog |=
                log->toPlainText().contains(QStringLiteral("[error] runtime failed"));
        QVERIFY(foundErrorLog);

        emit runtime.stopped();
        QVERIFY(actionRefreshes > 1);
    }

    void testApplicationStyleContainsShellAndInputRules() {
        const QString style = ApplicationStyle::globalStyleSheet();

        QVERIFY(style.contains(QStringLiteral("QMainWindow, QWidget")));
        QVERIFY(style.contains(QStringLiteral("QStatusBar")));
        QVERIFY(style.contains(QStringLiteral("QDockWidget")));
        QVERIFY(style.contains(QStringLiteral("QScrollBar::handle")));
        QVERIFY(style.contains(QStringLiteral("QLineEdit:focus")));
    }

    void testApplicationThemeControllerProjectsThemeToShell() {
        QWidget window;
        QTabWidget tabs;
        QSplitter splitter(Qt::Horizontal);
        BreadcrumbsBar breadcrumbs;
        QMenuBar menuBar;
        QStatusBar statusBar;
        StatusBarController statusController(&statusBar, true);

        ApplicationThemeController controller(
            &window, &tabs, &splitter, &breadcrumbs, &menuBar,
            &statusController);
        controller.apply();

        QVERIFY(!window.styleSheet().isEmpty());
        QVERIFY(!tabs.styleSheet().isEmpty());
        QVERIFY(!splitter.styleSheet().isEmpty());
        QVERIFY(!breadcrumbs.styleSheet().isEmpty());
        QCOMPARE(menuBar.font(), AppearanceController::instance().uiFont());
    }

    void testShellTranslationControllerProjectsShellText() {
        ActivityBar activityBar;
        QMenuBar menuBar;
        ShellCommandSurface commandSurface(&menuBar);
        ShellTranslationController controller(&activityBar, &commandSurface);

        controller.apply();

        QVERIFY(!menuBar.actions().isEmpty());
        const auto buttons = activityBar.findChildren<QToolButton *>();
        QCOMPARE(buttons.size(), 4);
        for (const QToolButton *button : buttons)
            QVERIFY(!button->toolTip().isEmpty());
    }

    void testShellDialogControllerOwnsDialogPresentationPolicy() {
        QWidget parent;
        auto *lspManager = new LspManagerDialog(&parent);
        int lspRefreshes = 0;
        ShellDialogController controller(
            &parent, lspManager, [&]() { ++lspRefreshes; });

        QVERIFY(controller.handleShellCommand(ShellCommand::Preferences));
        auto *preferences = parent.findChild<PreferencesDialog *>();
        QVERIFY(preferences);
        QVERIFY(preferences->isVisible());
        QVERIFY(controller.handleShellCommand(ShellCommand::Preferences));
        QVERIFY(preferences->isHidden());

        QVERIFY(controller.handleShellCommand(ShellCommand::Shortcuts));
        auto *shortcuts = parent.findChild<ShortcutsDialog *>();
        QVERIFY(shortcuts);
        QVERIFY(shortcuts->isVisible());

        QVERIFY(controller.handleShellCommand(ShellCommand::VimHelp));
        auto *vimHelp = parent.findChild<VimHelpDialog *>();
        QVERIFY(vimHelp);
        QVERIFY(vimHelp->isVisible());

        QVERIFY(controller.handleShellCommand(ShellCommand::LspManager));
        QVERIFY(lspManager->isVisible());
        QCOMPARE(lspRefreshes, 1);
        QVERIFY(controller.handleShellCommand(ShellCommand::LspManager));
        QVERIFY(lspManager->isHidden());
        QCOMPARE(lspRefreshes, 1);
        QVERIFY(!controller.handleShellCommand(ShellCommand::Find));
    }

    void testShellCommandSurfaceEmitsIntentAndAppliesActionState() {
        QMenuBar menuBar;
        ShellCommandSurface surface(&menuBar);
        surface.installShortcuts(&menuBar);
        QSignalSpy commandSpy(
            &surface, &ShellCommandSurface::commandRequested);
        QCOMPARE(menuBar.findChildren<QShortcut *>().size(), 10);

        QAction *buildAction = nullptr;
        QAction *newAction = nullptr;
        for (QAction *menuAction : menuBar.actions()) {
            if (!menuAction->menu())
                continue;
            for (QAction *action : menuAction->menu()->actions()) {
                if (action->text() == QStringLiteral("&Build"))
                    buildAction = action;
                if (action->text() == QStringLiteral("&New File"))
                    newAction = action;
            }
        }

        QVERIFY(buildAction);
        QVERIFY(newAction);
        newAction->trigger();
        QCOMPARE(commandSpy.count(), 1);

        ShellCommandSurface::WorkspaceActionState state;
        state.canBuild = true;
        state.buildTooltip = QStringLiteral("build");
        surface.setWorkspaceActionState(state);
        QVERIFY(buildAction->isEnabled());
        QCOMPARE(buildAction->toolTip(), QStringLiteral("build"));
    }

    void testWorkspaceCommandAvailabilityTracksEditorAndTaskState() {
        QMenuBar menuBar;
        ShellCommandSurface surface(&menuBar);
        CodeEditor editor;
        editor.setFilePath(QStringLiteral("/tmp/main.zith"));
        bool canExecute = true;
        bool taskRunning = false;
        QString workspaceRoot = QStringLiteral("/tmp/workspace");
        bool activeZith = true;

        auto currentState = [&]() {
            WorkspaceCommandState state;
            state.canExecuteWorkspaceCommand = canExecute;
            state.activeZithEditor = activeZith;
            state.hasCurrentFile = !editor.filePath().isEmpty();
            state.hasWorkspaceRoot = !workspaceRoot.isEmpty();
            state.taskRunning = taskRunning;
            return state;
        };
        WorkspaceCommandAvailabilityController controller(&surface);

        controller.refresh(currentState());

        QAction *buildAction = nullptr;
        QAction *runAction = nullptr;
        QAction *stopAction = nullptr;
        for (QAction *menuAction : menuBar.actions()) {
            if (!menuAction->menu())
                continue;
            for (QAction *action : menuAction->menu()->actions()) {
                if (action->text() == QStringLiteral("&Build"))
                    buildAction = action;
                if (action->text() == QStringLiteral("&Run"))
                    runAction = action;
                if (action->text() == QStringLiteral("&Stop"))
                    stopAction = action;
            }
        }

        QVERIFY(buildAction);
        QVERIFY(runAction);
        QVERIFY(stopAction);
        QVERIFY(buildAction->isEnabled());
        QVERIFY(runAction->isEnabled());
        QVERIFY(!stopAction->isEnabled());

        workspaceRoot.clear();
        controller.refresh(currentState());
        QVERIFY(!buildAction->isEnabled());
        QVERIFY(!runAction->isEnabled());
        QCOMPARE(buildAction->toolTip(),
                 QStringLiteral("No active project root"));

        workspaceRoot = QStringLiteral("/tmp/workspace");
        taskRunning = true;
        controller.refresh(currentState());
        QVERIFY(!runAction->isEnabled());
        QVERIFY(stopAction->isEnabled());
        QCOMPARE(runAction->toolTip(), QStringLiteral("A task is already running"));

        activeZith = false;
        controller.refresh(currentState());
        QVERIFY(!runAction->isEnabled());
        QVERIFY(stopAction->isEnabled());
        QCOMPARE(stopAction->toolTip(),
                 QStringLiteral("Stop running task (Ctrl+Shift+Q)"));

        canExecute = false;
        controller.refresh(currentState());
        QVERIFY(!buildAction->isEnabled());
        QCOMPARE(buildAction->toolTip(),
                 QStringLiteral(
                     "Requires a zith-lsp server with workspace/executeCommand support"));
    }

    void testWorkspacePanelPresentationKeepsPanelAndShellStateCoherent() {
        OutlinePanel outline;
        BottomPanel bottom;
        QMenuBar menuBar;
        ShellCommandSurface surface(&menuBar);
        CodeEditor editor;
        editor.setFilePath(QStringLiteral("/tmp/main.zith"));
        bool persistedOutlineVisibility = false;
        int chromeUpdates = 0;
        EditorChromeController chrome(
            nullptr, nullptr, nullptr,
            [](const QString &) { return QStringLiteral("Zith"); },
            [&editor]() { return &editor; },
            [](const QString &, int) {}, [](int, int) {},
            [](const QString &) {},
            [&chromeUpdates](const QString &) { ++chromeUpdates; });

        WorkspacePanelPresentationController::Dependencies dependencies;
        dependencies.outlinePanel = &outline;
        dependencies.bottomPanel = &bottom;
        dependencies.commandSurface = &surface;
        dependencies.editorChrome = &chrome;
        WorkspacePanelPresentationController controller(
            std::move(dependencies),
            [&editor]() -> CodeEditor * { return &editor; },
            [&](bool visible) { persistedOutlineVisibility = visible; });

        QAction *outlineAction = nullptr;
        QAction *bottomAction = nullptr;
        for (QAction *menuAction : menuBar.actions()) {
            if (!menuAction->menu())
                continue;
            for (QAction *action : menuAction->menu()->actions()) {
                if (action->text() == QStringLiteral("Structure"))
                    outlineAction = action;
                if (action->text() == QStringLiteral("Toggle Bottom Panel"))
                    bottomAction = action;
            }
        }

        QVERIFY(outlineAction);
        QVERIFY(bottomAction);

        controller.setOutlineVisible(true);
        QVERIFY(outline.isVisible());
        QVERIFY(persistedOutlineVisibility);
        QCOMPARE(chromeUpdates, 1);
        QVERIFY(outlineAction->isChecked());

        controller.toggleOutline();
        QVERIFY(!outline.isVisible());
        QVERIFY(!persistedOutlineVisibility);
        QVERIFY(!outlineAction->isChecked());

        controller.setBottomPanelVisible(true);
        QVERIFY(bottom.isVisible());
        QVERIFY(bottomAction->isChecked());

        emit bottom.closeRequested();
        QVERIFY(!bottom.isVisible());
        QVERIFY(!bottomAction->isChecked());

        QVERIFY(controller.handleShellCommand(ShellCommand::ToggleOutline));
        QVERIFY(outline.isVisible());
        QVERIFY(controller.handleShellCommand(ShellCommand::ToggleBottomPanel));
        QVERIFY(bottom.isVisible());
    }

    void testBottomPanelClearClearsActiveDiagnostics() {
        BottomPanel bottom;
        const LspDiagnostic diagnostic{
            LspRange{{4, 2}, {4, 8}}, 1, QStringLiteral("broken"),
            QStringLiteral("zith")};
        bottom.diagnostics()->setDiagnostics(
            QStringLiteral("file:///tmp/main.zith"), 1, {diagnostic});
        QCOMPARE(bottom.diagnostics()->errorCount(), 1);

        bottom.showDiagnostics();
        bottom.clearCurrent();

        QVERIFY(bottom.diagnostics()->allDiagnostics().isEmpty());
        QCOMPARE(bottom.diagnostics()->errorCount(), 0);
        QCOMPARE(bottom.diagnostics()->warningCount(), 0);
    }

    void testWorkspaceEditApplierUpdatesOpenAndClosedTargets() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString closedPath =
            QDir(tempDir.path()).filePath(QStringLiteral("closed.zith"));
        QFile closedFile(closedPath);
        QVERIFY(closedFile.open(QIODevice::WriteOnly | QIODevice::Text));
        closedFile.write("closed\n");
        closedFile.close();

        QTabWidget tabs;
        CodeEditor openEditor;
        const QString openPath =
            QDir(tempDir.path()).filePath(QStringLiteral("open.zith"));
        openEditor.setFilePath(openPath);
        openEditor.setInitialDocumentText(QStringLiteral("open\n"), 1);
        tabs.addTab(&openEditor, QStringLiteral("open.zith"));

        const auto editFor = [](int endCharacter) {
            return QJsonArray{
                QJsonObject{
                    {QStringLiteral("range"),
                     QJsonObject{
                         {QStringLiteral("start"),
                          QJsonObject{{QStringLiteral("line"), 0},
                                      {QStringLiteral("character"), 0}}},
                         {QStringLiteral("end"),
                          QJsonObject{{QStringLiteral("line"), 0},
                                      {QStringLiteral("character"),
                                       endCharacter}}}}},
                    {QStringLiteral("newText"), QStringLiteral("changed")}}};
        };
        const QJsonObject workspaceEdit{
            {QStringLiteral("changes"),
             QJsonObject{{QUrl::fromLocalFile(openPath).toString(),
                          editFor(4)},
                         {QUrl::fromLocalFile(closedPath).toString(),
                          editFor(6)}}}};

        WorkspaceEditApplier applier(&tabs);
        const auto result = applier.apply(workspaceEdit);
        QVERIFY(result.applied);
        QCOMPARE(openEditor.toPlainText(), QStringLiteral("changed\n"));

        QFile updatedClosed(closedPath);
        QVERIFY(updatedClosed.open(QIODevice::ReadOnly | QIODevice::Text));
        QCOMPARE(QString::fromUtf8(updatedClosed.readAll()),
                 QStringLiteral("changed\n"));
    }

    void testLspCompletionRouterCombinesOnlyForActiveEditor() {
        QTabWidget tabs;
        CodeEditor editor;
        LspClient client;
        SnippetManager snippets;
        LspCompletionModel model;
        LspCompleter completer(&model);

        editor.setFilePath(QStringLiteral("/tmp/main.zith"));
        tabs.addTab(&editor, QStringLiteral("main.zith"));

        LspCompletionRouter::Dependencies dependencies;
        dependencies.tabWidget = &tabs;
        dependencies.snippetManager = &snippets;
        dependencies.completer = &completer;
        dependencies.completionModel = &model;
        LspCompletionRouter router(std::move(dependencies),
                                   []() { return true; });
        router.attach(&client);

        const LspCompletionItem item{
            QStringLiteral("println"), 3, QStringLiteral("function"),
            QStringLiteral("println"), 1, {}};
        emit client.completionResults(editor.fileUri(), 1, {item});

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(completer.widget(), static_cast<QWidget *>(&editor));

        emit client.completionResults(
            QUrl::fromLocalFile(QStringLiteral("/tmp/other.zith")).toString(),
            1, {LspCompletionItem{QStringLiteral("ignored")}});
        QCOMPARE(model.rowCount(), 1);
    }

    void testContextManagerStoresSessionAsValueObject() {
        ContextManager manager;
        const EditorSessionState session{
            QStringList{QStringLiteral("/tmp/main.zith"),
                        QStringLiteral("/tmp/other.zith")},
            1};

        manager.setContextState(0, session);
        QCOMPARE(manager.currentContext().session.openFiles,
                 session.openFiles);
        QCOMPARE(manager.currentContext().session.currentTab,
                 session.currentTab);
    }

    void testMainWindowPersistsSessionOnClose() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString filePath =
            QDir(tempDir.path()).filePath(QStringLiteral("main.zith"));
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("fn main() {}\n");
        file.close();

        auto &settings = TomlSettingsStore::instance();
        settings.setLspEnabled(false);
        settings.setRecentProjects({});
        settings.setOnboardingDismissed(true);

        MainWindow window;
        window.openFilePath(filePath);
        auto *contextManager = window.findChild<ContextManager *>();
        QVERIFY(contextManager);
        QVERIFY(contextManager->currentContext().session.openFiles.isEmpty());

        window.close();

        QCOMPARE(contextManager->currentContext().session.openFiles,
                 QStringList({filePath}));
        QCOMPARE(contextManager->currentContext().session.currentTab, 0);
    }

    void testWindowLayoutControllerUsesNamedPersistenceSeam() {
        class FakeLayoutPersistence final : public WindowLayoutPersistence
        {
        public:
            QByteArray geometry;
            QByteArray state;
            int width = 320;
            bool sidebar = true;
            bool outline = false;

            QByteArray mainWindowGeometry() const override { return geometry; }
            QByteArray mainWindowState() const override { return state; }
            int sidebarWidth() const override { return width; }
            bool sidebarVisible() const override { return sidebar; }
            bool outlineVisible() const override { return outline; }

            void setMainWindowGeometry(const QByteArray &value) override
            {
                geometry = value;
            }
            void setMainWindowState(const QByteArray &value) override
            {
                state = value;
            }
            void setSidebarWidth(int value) override { width = value; }
            void setSidebarVisible(bool value) override { sidebar = value; }
            void setOutlineVisible(bool value) override { outline = value; }
        } persistence;

        QMainWindow window;
        QSplitter splitter(Qt::Horizontal);
        QWidget sidebar;
        QWidget editor;
        QWidget outline;
        splitter.addWidget(&sidebar);
        splitter.addWidget(&editor);
        splitter.addWidget(&outline);
        splitter.resize(900, 400);
        splitter.setSizes({320, 500, 80});
        splitter.show();
        QCoreApplication::processEvents();

        WindowLayoutController::Dependencies dependencies;
        dependencies.window = &window;
        dependencies.splitter = &splitter;
        dependencies.sidebar = &sidebar;
        dependencies.outline = &outline;
        WindowLayoutController controller(std::move(dependencies),
                                          persistence);

        persistence.width = 240;
        persistence.sidebar = false;
        persistence.outline = true;
        controller.restore();

        QVERIFY(!sidebar.isVisible());
        QVERIFY(outline.isVisible());
        QVERIFY(splitter.sizes().first() >= persistence.width);
        QVERIFY(splitter.sizes().first() <= persistence.width + 32);

        controller.setSidebarVisible(true);
        controller.setOutlineVisible(false);
        QVERIFY(sidebar.isVisible());
        QVERIFY(!outline.isVisible());
        QCOMPARE(persistence.sidebar, true);
        QCOMPARE(persistence.outline, false);

        splitter.setSizes({275, 465, 80});
        emit splitter.splitterMoved(275, 0);
        QCoreApplication::processEvents();
        QCOMPARE(persistence.width, 275);

        controller.save();
        QVERIFY(!persistence.geometry.isEmpty());
        QVERIFY(!persistence.state.isEmpty());
    }

    void testContextManagerExplainsWhyTheActiveContextChanged() {
        ContextManager manager;
        QList<ContextManager::ContextChangeReason> reasons;
        connect(&manager, &ContextManager::contextChanged, this,
                [&](int, const Context &, ContextManager::ContextChangeReason reason) {
                    reasons.append(reason);
                });

        manager.setCurrentRoot(QStringLiteral("/tmp/first"));
        manager.appendNew(QStringLiteral("/tmp/second"));
        manager.navigateLeft();

        QCOMPARE(reasons.size(), 3);
        QCOMPARE(reasons.at(0),
                 ContextManager::ContextChangeReason::RootChanged);
        QCOMPARE(reasons.at(1),
                 ContextManager::ContextChangeReason::NewContext);
        QCOMPARE(reasons.at(2),
                 ContextManager::ContextChangeReason::Navigation);
    }

    void testContextWorkspaceControllerPreservesTransitionSemantics() {
        ContextManager manager;
        QStringList roots;
        QList<QPair<int, int>> indicators;
        QStringList restoredRoots;
        QList<bool> runtimePreferences;

        ContextWorkspaceController::Callbacks callbacks;
        callbacks.applyWorkspaceRoot =
            [&](const QString &root) { roots.append(root); };
        callbacks.updateContextIndicator =
            [&](int index, int count) { indicators.append({index, count}); };
        callbacks.restoreSession = [&](const Context &context) {
            restoredRoots.append(context.rootPath);
        };
        callbacks.lspEnabled = []() { return true; };
        callbacks.zithLspRunning = []() { return true; };
        callbacks.ensureLspRuntime =
            [&](bool preferCached) { runtimePreferences.append(preferCached); };

        ContextWorkspaceController controller(&manager, std::move(callbacks));
        QSignalSpy appliedSpy(
            &controller, &ContextWorkspaceController::workspaceContextApplied);

        manager.setCurrentRoot(QStringLiteral("/root"));
        QCOMPARE(roots, QStringList{QStringLiteral("/root")});
        QCOMPARE(restoredRoots.size(), 0);
        QCOMPARE(runtimePreferences, QList<bool>{true});
        QCOMPARE(appliedSpy.count(), 1);
        QCOMPARE(indicators.last(), qMakePair(0, 1));

        manager.appendNew(QStringLiteral("/new"));
        QCOMPARE(restoredRoots, QStringList{QStringLiteral("/new")});
        const QList<bool> expectedAfterNewContext{true, true};
        QCOMPARE(runtimePreferences, expectedAfterNewContext);
        QCOMPARE(appliedSpy.count(), 2);
        QCOMPARE(indicators.last(), qMakePair(1, 2));

        manager.navigateLeft();
        const QStringList expectedRoots{
            QStringLiteral("/new"), QStringLiteral("/root")};
        const QList<bool> expectedPreferences{true, true, true};
        QCOMPARE(restoredRoots, expectedRoots);
        QCOMPARE(runtimePreferences, expectedPreferences);
        QCOMPARE(appliedSpy.count(), 3);
        QCOMPARE(indicators.last(), qMakePair(0, 2));
    }

    void testContextNavigationControllerPreservesHistoryPolicy() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString secondRoot =
            QDir(tempDir.path()).filePath(QStringLiteral("second"));
        QVERIFY(QDir().mkpath(secondRoot));

        ContextManager manager;
        manager.setCurrentRoot(tempDir.path());

        QWidget shortcutHost;
        bool saved = false;
        QString requestedTitle;
        QString requestedInitialPath;
        WorkspaceRootController::Callbacks rootCallbacks;
        rootCallbacks.saveCurrentContext = [&]() { saved = true; };
        WorkspaceRootController rootController(&manager,
                                                std::move(rootCallbacks));
        WorkspaceRootInteractionController interaction(
            nullptr, &rootController,
            [&](const QString &title, const QString &initialPath) {
                requestedTitle = title;
                requestedInitialPath = initialPath;
                return secondRoot;
            },
            [&manager]() { return manager.currentRoot(); });
        ContextNavigationController controller(
            &manager, &shortcutHost, [&]() { saved = true; },
            &interaction);

        QCOMPARE(shortcutHost.findChildren<QShortcut *>().size(), 2);
        controller.navigateRight();
        QVERIFY(saved);
        QCOMPARE(requestedTitle, QStringLiteral("New project folder"));
        QCOMPARE(requestedInitialPath, QString());
        QCOMPARE(manager.count(), 2);
        QCOMPARE(manager.currentRoot(), secondRoot);

        saved = false;
        controller.navigateLeft();
        QVERIFY(saved);
        QCOMPARE(manager.currentIndex(), 0);

        saved = false;
        WorkspaceRootInteractionController cancelledInteraction(
            nullptr, &rootController,
            [](const QString &, const QString &) { return QString(); },
            [&manager]() { return manager.currentRoot(); });
        ContextNavigationController cancelled(
            &manager, nullptr, [&]() { saved = true; },
            &cancelledInteraction);
        manager.navigateRight();
        cancelled.navigateRight();
        QVERIFY(!saved);
        QCOMPARE(manager.count(), 2);
        QCOMPARE(manager.currentIndex(), 1);
    }

    void testWorkspaceRootInteractionControllerCentralizesDirectorySelection() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString selectedRoot =
            QDir(tempDir.path()).filePath(QStringLiteral("selected"));
        QVERIFY(QDir().mkpath(selectedRoot));

        ContextManager manager;
        manager.setCurrentRoot(tempDir.path());
        int saves = 0;
        QStringList recentProjects;
        WorkspaceRootController::Callbacks rootCallbacks;
        rootCallbacks.saveCurrentContext = [&]() { ++saves; };
        rootCallbacks.persistRecentProject =
            [&](const QString &path) { recentProjects.append(path); };
        WorkspaceRootController rootController(&manager,
                                                std::move(rootCallbacks));

        QStringList titles;
        QStringList initialPaths;
        WorkspaceRootInteractionController interaction(
            nullptr, &rootController,
            [&](const QString &title, const QString &initialPath) {
                titles.append(title);
                initialPaths.append(initialPath);
                return selectedRoot;
            },
            [&manager]() { return manager.currentRoot(); });

        QCOMPARE(interaction.openFolder(),
                 WorkspaceRootController::ActivationResult::
                     ReplacedCurrentContext);
        QCOMPARE(manager.currentRoot(), selectedRoot);
        QCOMPARE(saves, 1);
        QCOMPARE(recentProjects, QStringList{selectedRoot});
        QCOMPARE(titles, QStringList{QStringLiteral("Open Folder")});
        QCOMPARE(initialPaths, QStringList{tempDir.path()});

        QCOMPARE(interaction.newProject(),
                 WorkspaceRootController::ActivationResult::CreatedContext);
        QCOMPARE(manager.count(), 2);
        QCOMPARE(saves, 2);
        const QStringList expectedTitles{
            QStringLiteral("Open Folder"),
            QStringLiteral("New project folder")};
        const QStringList expectedInitialPaths{tempDir.path(), QString()};
        QCOMPARE(titles, expectedTitles);
        QCOMPARE(initialPaths, expectedInitialPaths);
    }

    void testWorkspaceRootControllerRejectsInvalidRootsWithoutEffects() {
        ContextManager manager;
        int saves = 0;
        QStringList recentProjects;
        int refreshes = 0;

        WorkspaceRootController::Callbacks callbacks;
        callbacks.saveCurrentContext = [&]() { ++saves; };
        callbacks.persistRecentProject =
            [&](const QString &path) { recentProjects.append(path); };
        callbacks.refreshRecentProjects = [&]() { ++refreshes; };

        WorkspaceRootController controller(&manager, std::move(callbacks));
        const auto result = controller.replaceCurrentRoot(
            QStringLiteral("/path/that/does/not/exist"));

        QCOMPARE(result, WorkspaceRootController::ActivationResult::Rejected);
        QCOMPARE(saves, 0);
        QCOMPARE(recentProjects.size(), 0);
        QCOMPARE(refreshes, 0);
        QVERIFY(manager.currentRoot().isEmpty());
    }

    void testWorkspaceRootControllerCentralizesRootActivationPolicy() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const QString root =
            QDir(tempDir.path()).filePath(QStringLiteral("project"));
        QVERIFY(QDir().mkpath(root));

        ContextManager manager;
        int saves = 0;
        QStringList recentProjects;
        int refreshes = 0;

        WorkspaceRootController::Callbacks callbacks;
        callbacks.saveCurrentContext = [&]() { ++saves; };
        callbacks.persistRecentProject =
            [&](const QString &path) { recentProjects.append(path); };
        callbacks.refreshRecentProjects = [&]() { ++refreshes; };

        WorkspaceRootController controller(&manager, std::move(callbacks));
        const QString normalized = QFileInfo(root).absoluteFilePath();

        QCOMPARE(controller.replaceCurrentRoot(root),
                 WorkspaceRootController::ActivationResult::
                     ReplacedCurrentContext);
        QCOMPARE(manager.currentRoot(), normalized);
        QCOMPARE(saves, 1);
        QCOMPARE(recentProjects, QStringList{normalized});
        QCOMPARE(refreshes, 1);

        QCOMPARE(controller.createContext(root),
                 WorkspaceRootController::ActivationResult::CreatedContext);
        QCOMPARE(manager.count(), 2);
        QCOMPARE(manager.currentRoot(), normalized);
        QCOMPARE(saves, 2);
        const QStringList expectedRecentProjects{normalized, normalized};
        QCOMPARE(recentProjects, expectedRecentProjects);
        QCOMPARE(refreshes, 2);
    }

    void testWorkspaceRootControllerRestoresPersistedRootWithoutActivationEffects() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        ContextManager manager;
        int saves = 0;
        QStringList recentProjects;
        int refreshes = 0;

        WorkspaceRootController::Callbacks callbacks;
        callbacks.saveCurrentContext = [&]() { ++saves; };
        callbacks.persistRecentProject =
            [&](const QString &path) { recentProjects.append(path); };
        callbacks.refreshRecentProjects = [&]() { ++refreshes; };

        WorkspaceRootController controller(&manager, std::move(callbacks));
        const QString normalized =
            QFileInfo(tempDir.path()).absoluteFilePath();

        QCOMPARE(controller.restoreCurrentRoot(tempDir.path()),
                 WorkspaceRootController::ActivationResult::
                     RestoredCurrentContext);
        QCOMPARE(manager.currentRoot(), normalized);
        QCOMPARE(saves, 0);
        QVERIFY(recentProjects.isEmpty());
        QCOMPARE(refreshes, 0);

        const QString filePath =
            QDir(tempDir.path()).filePath(QStringLiteral("not-a-directory"));
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();

        QCOMPARE(controller.restoreCurrentRoot(filePath),
                 WorkspaceRootController::ActivationResult::Rejected);
        QCOMPARE(manager.currentRoot(), normalized);
        QCOMPARE(saves, 0);
        QVERIFY(recentProjects.isEmpty());
        QCOMPARE(refreshes, 0);
    }

    void testSidebarControllerKeepsModeAndVisibilityPolicy() {
        ActivityBar activityBar;
        QStackedWidget sidePanel;
        sidePanel.addWidget(new QWidget(&sidePanel));
        sidePanel.addWidget(new QWidget(&sidePanel));
        sidePanel.addWidget(new QWidget(&sidePanel));
        sidePanel.addWidget(new QWidget(&sidePanel));

        QList<bool> persistedVisibility;
        int settingsPreparations = 0;
        SidebarController controller(
            &activityBar, &sidePanel, nullptr,
            [&](bool visible) { persistedVisibility.append(visible); },
            [&]() { ++settingsPreparations; },
            []() {});

        controller.selectMode(ActivityBar::Search);
        QVERIFY(sidePanel.isVisible());
        QCOMPARE(sidePanel.currentIndex(), 1);
        QCOMPARE(persistedVisibility.last(), true);

        controller.selectMode(ActivityBar::Search);
        QVERIFY(!sidePanel.isVisible());
        QCOMPARE(persistedVisibility.last(), false);

        controller.showSettings();
        QCOMPARE(settingsPreparations, 1);
        QVERIFY(sidePanel.isVisible());
        QCOMPARE(sidePanel.currentIndex(), 3);
        QCOMPARE(activityBar.activeMode(), ActivityBar::Settings);

        emit activityBar.modeChanged(ActivityBar::Explorer);
        QCOMPARE(sidePanel.currentIndex(), 0);
        QVERIFY(sidePanel.isVisible());

        QVERIFY(controller.handleShellCommand(ShellCommand::HideSidebar));
        QVERIFY(!sidePanel.isVisible());
    }

    void testEditorWorkspacePresentationKeepsCentralModeAndFindPolicy() {
        QTabWidget tabs;
        QStackedWidget centralStack;
        WelcomeWidget welcome;
        QWidget editorPanel;
        BreadcrumbsBar breadcrumbs;
        FindReplaceBar findBar;
        centralStack.addWidget(&welcome);
        centralStack.addWidget(&editorPanel);

        EditorWorkspacePresentationController::Dependencies dependencies;
        dependencies.tabs = &tabs;
        dependencies.centralStack = &centralStack;
        dependencies.welcomeWidget = &welcome;
        dependencies.editorPanel = &editorPanel;
        dependencies.breadcrumbs = &breadcrumbs;
        dependencies.findReplaceBar = &findBar;
        EditorWorkspacePresentationController controller(
            std::move(dependencies));

        controller.synchronize();
        QCOMPARE(centralStack.currentWidget(), static_cast<QWidget *>(&welcome));
        QVERIFY(findBar.isHidden());

        CodeEditor editor;
        tabs.addTab(&editor, QStringLiteral("main.zith"));
        controller.synchronize();
        QCOMPARE(centralStack.currentWidget(),
                 static_cast<QWidget *>(&editorPanel));

        controller.showFind();
        QCOMPARE(findBar.editor(), &editor);
        QVERIFY(!findBar.isHidden());

        QVERIFY(controller.handleShellCommand(ShellCommand::FindPrevious));

        tabs.removeTab(0);
        controller.synchronize();
        QCOMPARE(centralStack.currentWidget(), static_cast<QWidget *>(&welcome));
        QVERIFY(findBar.isHidden());
    }

    void testGitRepositorySessionSequencesStatusAndRemote() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString fakeGitPath =
            QDir(tempDir.path()).filePath(QStringLiteral("fake-git"));
        QFile fakeGit(fakeGitPath);
        QVERIFY(fakeGit.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream script(&fakeGit);
        script << "#!/bin/sh\n"
                  "if [ \"$1\" = \"status\" ]; then\n"
                  "  printf '## main\\n M src/main.cpp\\n'\n"
                  "elif [ \"$1\" = \"remote\" ]; then\n"
                  "  printf 'origin\\n'\n"
                  "fi\n";
        fakeGit.close();
        QVERIFY(fakeGit.setPermissions(QFileDevice::ReadOwner
                                       | QFileDevice::WriteOwner
                                       | QFileDevice::ExeOwner));

        GitRepositorySession session;
        session.setProgram(fakeGitPath);
        QSignalSpy stateSpy(&session, &GitRepositorySession::stateChanged);

        session.setRootPath(tempDir.path());

        QTRY_VERIFY_WITH_TIMEOUT(session.state().remoteAvailable, 5000);
        QVERIFY(!session.isBusy());
        QVERIFY(stateSpy.count() > 0);
        const GitRepositoryState state = session.state();
        QCOMPARE(state.rootPath, tempDir.path());
        QCOMPARE(state.branch, QStringLiteral("main"));
        QCOMPARE(state.entries.size(), 1);
        QCOMPARE(state.entries.first().relativePath,
                 QStringLiteral("src/main.cpp"));
        QVERIFY(state.repositoryAvailable);
        QVERIFY(state.remoteAvailable);
        QVERIFY(!state.busy);
    }

    void testGitRepositorySessionTreatsEmptyRootAsInformational() {
        GitRepositorySession session;
        QSignalSpy messageSpy(&session, &GitRepositorySession::messageChanged);
        QSignalSpy stateSpy(&session, &GitRepositorySession::stateChanged);

        session.refresh();

        QCOMPARE(messageSpy.count(), 1);
        QCOMPARE(messageSpy.at(0).at(0).toString(),
                 QStringLiteral("Open a project inside a Git repository."));
        QCOMPARE(messageSpy.at(0).at(1).toBool(), false);
        QCOMPARE(stateSpy.count(), 1);
        const GitRepositoryState state = session.state();
        QVERIFY(state.rootPath.isEmpty());
        QVERIFY(!state.repositoryAvailable);
        QVERIFY(!state.remoteAvailable);
        QVERIFY(!state.busy);
    }

    void testEditorDeletionPolicyPrefersPairsAndTabStops() {
        const DeletionDecision pair =
            EditorDeletionPolicy::decide(QStringLiteral("x("), ')', false);
        QCOMPARE(pair.action, DeletionDecision::Action::DeletePair);
        QCOMPARE(pair.characterCount, 2);

        const DeletionDecision indentation =
            EditorDeletionPolicy::decide(QStringLiteral("        "), 'x', false);
        QCOMPARE(indentation.action,
                 DeletionDecision::Action::DeleteIndentation);
        QCOMPARE(indentation.characterCount, 4);

        const DeletionDecision selected =
            EditorDeletionPolicy::decide(QStringLiteral("("), ')', true);
        QCOMPARE(selected.action, DeletionDecision::Action::Default);
    }
};

QTEST_MAIN(TestHelios)
#include "test_helios.moc"
