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

#include "../editor/core/TomlSettingsStore.h"
#include "../editor/core/AppearanceController.h"
#include "../editor/core/ThemeManager.h"
#include "../editor/core/TranslationManager.h"
#include "../editor/panels/SettingsPanel.h"
#include "../editor/editor/Syntax.h"
#include "../editor/editor/VimMotionController.h"
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
            "logicalOperator", "operator", "otherOperator", "bracket", "punctuation"
        };

        auto &tm = ThemeManager::instance();
        for (const QString &themeId : themeIds) {
            const QString path = QDir(QCoreApplication::applicationDirPath())
                                     .filePath("themes/" + themeId + ".json");
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
        }
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
};

QTEST_MAIN(TestHelios)
#include "test_helios.moc"
