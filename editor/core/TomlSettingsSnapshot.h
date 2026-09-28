#ifndef TOMLSETTINGSSNAPSHOT_H
#define TOMLSETTINGSSNAPSHOT_H

#include <QString>
#include <QStringList>

struct TomlSettingsSnapshot
{
    QString theme = QStringLiteral("helios-dark");
    QString customThemePath;
    QString locale = QStringLiteral("en-US");
    QString uiFontFamily = QStringLiteral("System Default");
    int uiFontSize = 13;
    QString editorFontFamily = QStringLiteral("System Default");
    int editorFontSize = 13;
    QString renderingStrategy = QStringLiteral("antialias");
    int uiScale = 100;
    bool vimMotionsEnabled = false;
    bool wordWrap = false;
    QStringList searchTextExtensions = {
        QStringLiteral("zith"), QStringLiteral("toml"),
        QStringLiteral("json"), QStringLiteral("md"),
        QStringLiteral("txt"), QStringLiteral("cpp"),
        QStringLiteral("cc"), QStringLiteral("cxx"),
        QStringLiteral("c"), QStringLiteral("h"),
        QStringLiteral("hpp"), QStringLiteral("qml"),
        QStringLiteral("cmake"), QStringLiteral("makefile"),
        QStringLiteral("dockerfile"), QStringLiteral("sh"),
        QStringLiteral("py"), QStringLiteral("rs"),
        QStringLiteral("go"), QStringLiteral("js"),
        QStringLiteral("ts"), QStringLiteral("yml"),
        QStringLiteral("yaml"), QStringLiteral("xml"),
        QStringLiteral("ini"), QStringLiteral("env"),
        QStringLiteral("rc")};
    QStringList searchExcludedDirs = {QStringLiteral(".git"),
                                      QStringLiteral("build"),
                                      QStringLiteral("node_modules"),
                                      QStringLiteral("venv")};
    int sidebarWidth = 320;
    bool sidebarVisible = true;
    bool outlineVisible = false;
    int treeMaxDepth = 12;
    bool onboardingDismissed = false;
    bool lspEnabled = false;
    bool useOnlineZithLsp = false;
    bool cLspEnabled = true;
    QString cLspPath;
    QString mainWindowGeometryBase64;
    QString mainWindowStateBase64;
    QStringList recentProjects;
};

#endif
