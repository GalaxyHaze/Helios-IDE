#include <QtTest>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPlainTextEdit>
#include <QKeyEvent>
#include <QPushButton>
#include <QSyntaxHighlighter>
#include <QModelIndex>

#include "../editor/core/TomlSettingsStore.h"
#include "../editor/core/AppearanceController.h"
#include "../editor/core/ZithToolchainManager.h"
#include "../editor/core/ThemeManager.h"
#include "../editor/core/TranslationManager.h"
#include "../editor/core/RunOutputCollector.h"
#include "../editor/editor/LspClient.h"
#include "../editor/panels/CompilerPanel.h"
#include "../editor/panels/SettingsPanel.h"
#include "../editor/editor/Syntax.h"
#include "../editor/editor/VimMotionController.h"
#include "../editor/editor/CHighlighter.h"
#include "../editor/panels/SearchPanel.h"
#include "../editor/core/FileIcons.h"
#include "../editor/widgets/ProjectTreeModel.h"
#include "../editor/editor/Code.h"
#include "../editor/widgets/FindReplaceBar.h"
#include "../editor/core/SnippetManager.h"
#include "../editor/editor/LspCompletionModel.h"

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

    void testLspClientParsesWorkDoneProgress() {
        LspClient client;
        QSignalSpy progressSpy(&client, &LspClient::workDoneProgressReceived);

        client.receiveFrame(QJsonDocument(QJsonObject{
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
        }).toJson(QJsonDocument::Compact));
        client.receiveFrame(QJsonDocument(QJsonObject{
            {"jsonrpc", "2.0"},
            {"method", "$/progress"},
            {"params", QJsonObject{
                {"token", "zith-build-1"},
                {"value", QJsonObject{
                    {"kind", "report"},
                    {"message", "Compiling"}
                }}
            }}
        }).toJson(QJsonDocument::Compact));
        client.receiveFrame(QJsonDocument(QJsonObject{
            {"jsonrpc", "2.0"},
            {"method", "$/progress"},
            {"params", QJsonObject{
                {"token", "zith-build-1"},
                {"value", QJsonObject{
                    {"kind", "end"},
                    {"message", "Finished"}
                }}
            }}
        }).toJson(QJsonDocument::Compact));

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

        client.receiveFrame(QJsonDocument(QJsonObject{
            {"jsonrpc", "2.0"},
            {"id", "wdp-1"},
            {"method", "window/workDoneProgress/create"},
            {"params", QJsonObject{{"token", "zith-build-1"}}}
        }).toJson(QJsonDocument::Compact));

        client.stop();
        client.waitForFinishedForTesting(5000);
    }

    void testCompilerPanelShowsProgressAndDiagnostics() {
        CompilerPanel panel;
        panel.startBuild(QStringLiteral("Build project /tmp/demo"));

        panel.appendWorkDoneProgress(QStringLiteral("token-1"),
                                     QStringLiteral("begin"));
        panel.appendWorkDoneProgress(QStringLiteral("token-1"),
                                     QStringLiteral("report"),
                                     QStringLiteral("Compiling"));
        panel.appendWorkDoneProgress(QStringLiteral("old-token"),
                                     QStringLiteral("end"),
                                     QStringLiteral("Finished"));
        QVERIFY(panel.outputText().contains(QStringLiteral("Compiling...")));

        panel.appendWorkDoneProgress(QStringLiteral("token-1"),
                                     QStringLiteral("end"),
                                     QStringLiteral("Finished"));
        panel.appendDiagnostics(QList<LspDiagnostic>{
            {{ {2, 3}, {2, 7} }, 1, QStringLiteral("type error"), QStringLiteral("zithc")}});

        const QString text = panel.outputText();
        QVERIFY(text.contains(QStringLiteral("Finished")));
        QVERIFY(text.contains(QStringLiteral("  line 3, col 4: type error")));
        QVERIFY(!text.contains(QStringLiteral("old-token")));
    }

    void testCompilerPanelIgnoresForeignProgress() {
        CompilerPanel panel;
        panel.startBuild(QStringLiteral("Build /tmp/other"));
        panel.setActiveProgressToken(QStringLiteral("current"));

        panel.appendWorkDoneProgress(QStringLiteral("stale"),
                                     QStringLiteral("begin"));
        panel.appendWorkDoneProgress(QStringLiteral("stale"),
                                     QStringLiteral("end"));

        QVERIFY(!panel.outputText().contains(QStringLiteral("Compiling")));
        QVERIFY(!panel.outputText().contains(QStringLiteral("Finished")));
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

    void testZithToolchainRejectsLocalCacheAsRelease() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        ZithToolchainManager manager;
        const QString cacheRoot = QDir(tempDir.path()).filePath("zith-runtime");
        manager.setCacheRootForTesting(cacheRoot);
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

        QString lspPath;
        QString stdlibPath;
        QString tag;
        QVERIFY(manager.resolveNewestInstalledRelease(&lspPath, &stdlibPath, &tag));
        QCOMPARE(tag, QStringLiteral("v1.10.0"));
        QVERIFY(!lspPath.contains("/local/"));
        QVERIFY(!stdlibPath.contains("/local/"));

        QVERIFY(!manager.resolveInstalledRelease("local", &lspPath, &stdlibPath));
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

        QVERIFY(SearchPanel::shouldScanFile(
            "/tmp/project/src/main.zith", extensions, excludedDirs));
        QVERIFY(SearchPanel::shouldScanFile(
            "/tmp/project/Dockerfile", extensions, excludedDirs));
        QVERIFY(SearchPanel::shouldScanFile(
            "/tmp/project/notes.txt", extensions, excludedDirs) == false);
        QVERIFY(SearchPanel::shouldScanFile(
            "/tmp/project/vendor/lib.cpp", extensions, excludedDirs) == false);
        QVERIFY(SearchPanel::shouldScanFile(
            "/tmp/project/.git/config", extensions, excludedDirs) == false);
        QVERIFY(SearchPanel::shouldScanFile(
            "/tmp/project/src/main.cpp", extensions, excludedDirs));
        QVERIFY(SearchPanel::shouldScanFile(
            "/tmp/project/dockerfile", extensions, excludedDirs));
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

        const auto edits = SearchPanel::replaceEdits(text, "beta", "X");
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

        QCOMPARE(SearchPanel::applyReplaceEdits(text, edits),
                 QString("alpha X\nX gamma\nlast X\n"));
    }

    void testWorkspaceReplaceEditsIgnoresEmptyNeedleAndSyncsOpenFile() {
        QVERIFY(SearchPanel::replaceEdits("no changes", QString(), "X").isEmpty());

        CodeEditor editor;
        editor.setInitialDocumentText("alpha beta\nbeta gamma\n");
        const auto edits = SearchPanel::replaceEdits(
            editor.toPlainText(), "beta", "X");
        const QString replaced =
            SearchPanel::applyReplaceEdits(editor.toPlainText(), edits);
        QCOMPARE(replaced, QString("alpha X\nX gamma\n"));
        editor.setInitialDocumentText(replaced);
        QCOMPARE(editor.toPlainText(), replaced);
    }
};

QTEST_MAIN(TestHelios)
#include "test_helios.moc"
