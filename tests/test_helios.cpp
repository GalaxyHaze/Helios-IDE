#include <QtTest>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

#include "../editor/core/TomlSettingsStore.h"
#include "../editor/core/ThemeManager.h"
#include "../editor/core/TranslationManager.h"
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

        QStringList projects = {"/path/to/a", "/path/to/b"};
        store.setRecentProjects(projects);
        store.save();

        store.load();
        QCOMPARE(store.fontSize(), 15);
        QCOMPARE(store.theme(), QString("helios-dark"));
        QCOMPARE(store.locale(), QString("pt-BR"));
        QCOMPARE(store.sidebarVisible(), false);
        QCOMPARE(store.treeMaxDepth(), 20);
        QCOMPARE(store.recentProjects().size(), 2);
        QCOMPARE(store.recentProjects().first(), QString("/path/to/a"));
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
            for (const QString &key : paletteKeys) {
                QVERIFY2(palette.value(key).isString(), qPrintable(themeId + ":" + key));
                QVERIFY(QColor(palette.value(key).toString()).isValid());
            }
            for (const QString &key : customKeys) {
                QVERIFY2(custom.value(key).isString(), qPrintable(themeId + ":" + key));
                QVERIFY(QColor(custom.value(key).toString()).isValid());
            }

            QVERIFY2(tm.loadTheme(themeId), qPrintable(themeId));
            QCOMPARE(tm.currentThemeName(), themeId);
            QVERIFY(tm.palette().color(QPalette::Window).isValid());
            QVERIFY(tm.customColor("editorBg").isValid());
            QVERIFY(tm.customColor("diagnosticError").isValid());
        }
    }

    void testTranslationManagerFallback() {
        auto &tr = TranslationManager::instance();
        tr.loadLocale("pt-BR");
        QString val = tr.translate("invalid.key");
        QCOMPARE(val, QString("invalid.key"));

        QString validVal = tr.translate("menu.new_file");
        QCOMPARE(validVal, QString("Novo Arquivo"));
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
