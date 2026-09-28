#include "TomlSettingsCodec.h"

#include <QTextStream>

namespace
{
QString unquote(const QString &value)
{
    if (value.startsWith('"') && value.endsWith('"') &&
        value.length() >= 2) {
        return value.mid(1, value.length() - 2);
    }
    return value;
}

QStringList parseList(const QString &value)
{
    if (!value.startsWith('[') || !value.endsWith(']'))
        return {};

    const QString content = value.mid(1, value.length() - 2).trimmed();
    if (content.isEmpty())
        return {};

    QStringList result;
    const QStringList items = content.split(',', Qt::SkipEmptyParts);
    for (const QString &item : items) {
        const QString clean = unquote(item.trimmed());
        if (!clean.isEmpty())
            result.append(clean);
    }
    return result;
}

void writeList(QTextStream &out, const QString &key,
               const QStringList &values)
{
    out << key << " = [";
    for (int i = 0; i < values.size(); ++i) {
        out << "\"" << values.at(i) << "\"";
        if (i < values.size() - 1)
            out << ", ";
    }
    out << "]\n";
}
}

TomlSettingsSnapshot TomlSettingsCodec::parse(
    const QString &contents, const TomlSettingsSnapshot &defaults)
{
    TomlSettingsSnapshot settings = defaults;
    bool legacyFontFamilySeen = false;
    bool legacyFontSizeSeen = false;
    bool editorFontFamilySeen = false;
    bool editorFontSizeSeen = false;

    const QStringList lines = contents.split('\n');
    for (const QString &rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith('['))
            continue;

        const int equals = line.indexOf('=');
        if (equals < 0)
            continue;

        const QString key = line.left(equals).trimmed();
        const QString value = line.mid(equals + 1).trimmed();

        if (key == "theme") {
            settings.theme = unquote(value);
        } else if (key == "customThemePath") {
            settings.customThemePath = unquote(value);
        } else if (key == "locale") {
            settings.locale = unquote(value);
        } else if (key == "fontFamily") {
            legacyFontFamilySeen = true;
            settings.uiFontFamily = unquote(value);
        } else if (key == "fontSize") {
            legacyFontSizeSeen = true;
            settings.uiFontSize = value.toInt();
            if (settings.uiFontSize < 6)
                settings.uiFontSize = 13;
        } else if (key == "uiFontFamily") {
            settings.uiFontFamily = unquote(value);
        } else if (key == "uiFontSize") {
            settings.uiFontSize = value.toInt();
            if (settings.uiFontSize < 6)
                settings.uiFontSize = 13;
        } else if (key == "editorFontFamily") {
            editorFontFamilySeen = true;
            settings.editorFontFamily = unquote(value);
        } else if (key == "editorFontSize") {
            editorFontSizeSeen = true;
            settings.editorFontSize = value.toInt();
            if (settings.editorFontSize < 6)
                settings.editorFontSize = 13;
        } else if (key == "renderingStrategy") {
            settings.renderingStrategy = unquote(value);
        } else if (key == "uiScale") {
            settings.uiScale = value.toInt();
            if (settings.uiScale < 75 || settings.uiScale > 200)
                settings.uiScale = 100;
        } else if (key == "vimMotionsEnabled") {
            settings.vimMotionsEnabled = value == "true";
        } else if (key == "wordWrap") {
            settings.wordWrap = value == "true";
        } else if (key == "searchTextExtensions") {
            settings.searchTextExtensions = parseList(value);
        } else if (key == "searchExcludedDirs") {
            settings.searchExcludedDirs = parseList(value);
        } else if (key == "sidebarWidth") {
            settings.sidebarWidth = value.toInt();
            if (settings.sidebarWidth < 50)
                settings.sidebarWidth = 280;
        } else if (key == "sidebarVisible") {
            settings.sidebarVisible = value == "true";
        } else if (key == "outlineVisible") {
            settings.outlineVisible = value == "true";
        } else if (key == "treeMaxDepth") {
            settings.treeMaxDepth = value.toInt();
            if (settings.treeMaxDepth < 1 || settings.treeMaxDepth > 64)
                settings.treeMaxDepth = 12;
        } else if (key == "onboardingDismissed") {
            settings.onboardingDismissed = value == "true";
        } else if (key == "lspEnabled") {
            settings.lspEnabled = value == "true";
        } else if (key == "useOnlineZithLsp") {
            settings.useOnlineZithLsp = value == "true";
        } else if (key == "cLspEnabled") {
            settings.cLspEnabled = value == "true";
        } else if (key == "cLspPath") {
            settings.cLspPath = unquote(value);
        } else if (key == "mainWindowGeometry") {
            settings.mainWindowGeometryBase64 = unquote(value);
        } else if (key == "mainWindowState") {
            settings.mainWindowStateBase64 = unquote(value);
        } else if (key == "recentProjects") {
            settings.recentProjects = parseList(value);
        }
    }

    if (legacyFontFamilySeen && !editorFontFamilySeen)
        settings.editorFontFamily = settings.uiFontFamily;
    if (legacyFontSizeSeen && !editorFontSizeSeen)
        settings.editorFontSize = settings.uiFontSize;
    return settings;
}

QString TomlSettingsCodec::serialize(const TomlSettingsSnapshot &settings)
{
    QString contents;
    QTextStream out(&contents);
    out << "# Helios configuration file\n\n";

    out << "[editor]\n";
    out << "editorFontFamily = \"" << settings.editorFontFamily << "\"\n";
    out << "editorFontSize = " << settings.editorFontSize << "\n";
    out << "renderingStrategy = \"" << settings.renderingStrategy << "\"\n";
    out << "wordWrap = " << (settings.wordWrap ? "true" : "false")
        << "\n\n";

    out << "[search]\n";
    writeList(out, "searchTextExtensions", settings.searchTextExtensions);
    writeList(out, "searchExcludedDirs", settings.searchExcludedDirs);
    out << "\n";

    out << "[ui]\n";
    out << "theme = \"" << settings.theme << "\"\n";
    out << "customThemePath = \"" << settings.customThemePath << "\"\n";
    out << "locale = \"" << settings.locale << "\"\n";
    out << "uiFontFamily = \"" << settings.uiFontFamily << "\"\n";
    out << "uiFontSize = " << settings.uiFontSize << "\n";
    out << "uiScale = " << settings.uiScale << "\n";
    out << "sidebarWidth = " << settings.sidebarWidth << "\n";
    out << "sidebarVisible = "
        << (settings.sidebarVisible ? "true" : "false") << "\n";
    out << "outlineVisible = "
        << (settings.outlineVisible ? "true" : "false") << "\n";
    out << "treeMaxDepth = " << settings.treeMaxDepth << "\n";
    out << "onboardingDismissed = "
        << (settings.onboardingDismissed ? "true" : "false") << "\n\n";

    out << "[lsp]\n";
    out << "lspEnabled = " << (settings.lspEnabled ? "true" : "false")
        << "\n";
    out << "useOnlineZithLsp = "
        << (settings.useOnlineZithLsp ? "true" : "false") << "\n\n";

    out << "[cLsp]\n";
    out << "cLspEnabled = "
        << (settings.cLspEnabled ? "true" : "false") << "\n";
    out << "cLspPath = \"" << settings.cLspPath << "\"\n\n";

    out << "[main window]\n";
    out << "mainWindowGeometry = \"" << settings.mainWindowGeometryBase64
        << "\"\n";
    out << "mainWindowState = \"" << settings.mainWindowStateBase64
        << "\"\n\n";

    out << "[vim]\n";
    out << "vimMotionsEnabled = "
        << (settings.vimMotionsEnabled ? "true" : "false") << "\n\n";

    out << "[projects]\n";
    writeList(out, "recentProjects", settings.recentProjects);
    return contents;
}
