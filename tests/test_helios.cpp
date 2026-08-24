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
#include "../editor/core/ThemeManager.h"
#include "../editor/core/TranslationManager.h"
#include "../editor/panels/SettingsPanel.h"
#include "../editor/editor/Syntax.h"
#include "../editor/editor/VimMotionController.h"
#include "../editor/editor/CHighlighter.h"
#include "../editor/core/FileIcons.h"
#include "../editor/widgets/ProjectTreeModel.h"

class TestHelios : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        QCoreApplication::setApplicationName("HeliosTest");
    }

    void testTomlSettingsStore() {
        auto &store = TomlSettingsStore::instance();
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
};

QTEST_MAIN(TestHelios)
#include "test_helios.moc"
