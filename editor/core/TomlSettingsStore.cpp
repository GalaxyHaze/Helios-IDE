#include "TomlSettingsStore.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

TomlSettingsStore::TomlSettingsStore(QObject *parent)
    : QObject(parent)
{
    load();
}

TomlSettingsStore &TomlSettingsStore::instance()
{
    static TomlSettingsStore store;
    return store;
}

QString TomlSettingsStore::filePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(dir).filePath("settings.toml");
}

void TomlSettingsStore::load()
{
    QString path = filePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    bool legacyFontFamilySeen = false;
    bool legacyFontSizeSeen = false;
    bool editorFontFamilySeen = false;
    bool editorFontSizeSeen = false;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith('['))
            continue;

        int eqIdx = line.indexOf('=');
        if (eqIdx == -1)
            continue;

        QString key = line.left(eqIdx).trimmed();
        QString val = line.mid(eqIdx + 1).trimmed();

        if (key == "theme") {
            if (val.startsWith('"') && val.endsWith('"') && val.length() >= 2)
                m_theme = val.mid(1, val.length() - 2);
        } else if (key == "customThemePath") {
            if (val.startsWith('"') && val.endsWith('"') && val.length() >= 2)
                m_customThemePath = val.mid(1, val.length() - 2);
        } else if (key == "locale") {
            if (val.startsWith('"') && val.endsWith('"') && val.length() >= 2)
                m_locale = val.mid(1, val.length() - 2);
        } else if (key == "fontFamily") {
            legacyFontFamilySeen = true;
            if (val.startsWith('"') && val.endsWith('"') && val.length() >= 2) m_uiFontFamily = val.mid(1, val.length() - 2); else m_uiFontFamily = val.trimmed();
        } else if (key == "fontSize") {
            legacyFontSizeSeen = true;
            m_uiFontSize = val.toInt();
            if (m_uiFontSize < 6) m_uiFontSize = 13;
        } else if (key == "uiFontFamily") {
            if (val.startsWith('"') && val.endsWith('"') && val.length() >= 2) m_uiFontFamily = val.mid(1, val.length() - 2);
        } else if (key == "uiFontSize") {
            m_uiFontSize = val.toInt();
            if (m_uiFontSize < 6) m_uiFontSize = 13;
        } else if (key == "editorFontFamily") {
            editorFontFamilySeen = true;
            if (val.startsWith('"') && val.endsWith('"') && val.length() >= 2) m_editorFontFamily = val.mid(1, val.length() - 2);
        } else if (key == "editorFontSize") {
            editorFontSizeSeen = true;
            m_editorFontSize = val.toInt();
            if (m_editorFontSize < 6) m_editorFontSize = 13;
        } else if (key == "renderingStrategy") {
            if (val.startsWith('"') && val.endsWith('"') && val.length() >= 2) m_renderingStrategy = val.mid(1, val.length() - 2);
        } else if (key == "uiScale") {
            m_uiScale = val.toInt();
            if (m_uiScale < 75 || m_uiScale > 200) m_uiScale = 100;
        } else if (key == "vimMotionsEnabled") {
            m_vimMotionsEnabled = (val == "true");
        } else if (key == "wordWrap") {
            m_wordWrap = (val == "true");
        } else if (key == "sidebarWidth") {
            m_sidebarWidth = val.toInt();
            if (m_sidebarWidth < 50) m_sidebarWidth = 280;
        } else if (key == "sidebarVisible") {
            m_sidebarVisible = (val == "true");
        } else if (key == "treeMaxDepth") {
            m_treeMaxDepth = val.toInt();
            if (m_treeMaxDepth < 1 || m_treeMaxDepth > 64) m_treeMaxDepth = 12;
        } else if (key == "onboardingDismissed") {
            m_onboardingDismissed = (val == "true");
        } else if (key == "lspEnabled") {
            m_lspEnabled = (val == "true");
        } else if (key == "recentProjects") {
            m_recentProjects.clear();
            if (val.startsWith('[') && val.endsWith(']')) {
                QString content = val.mid(1, val.length() - 2).trimmed();
                if (!content.isEmpty()) {
                    QStringList items = content.split(',');
                    for (const QString &item : items) {
                        QString clean = item.trimmed();
                        if (clean.startsWith('"') && clean.endsWith('"') && clean.length() >= 2) {
                            clean = clean.mid(1, clean.length() - 2);
                            if (!clean.isEmpty()) {
                                m_recentProjects.append(clean);
                            }
                        }
                    }
                }
            }
        }
    }
    if (legacyFontFamilySeen && !editorFontFamilySeen)
        m_editorFontFamily = m_uiFontFamily;
    if (legacyFontSizeSeen && !editorFontSizeSeen)
        m_editorFontSize = m_uiFontSize;
}

void TomlSettingsStore::save()
{
    QString path = filePath();
    QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    out << "# Helios configuration file\n\n";

    out << "[editor]\n";
    out << "editorFontFamily = \"" << m_editorFontFamily << "\"\n";
    out << "editorFontSize = " << m_editorFontSize << "\n";
    out << "renderingStrategy = \"" << m_renderingStrategy << "\"\n";
    out << "wordWrap = " << (m_wordWrap ? "true" : "false") << "\n\n";

    out << "[ui]\n";
    out << "theme = \"" << m_theme << "\"\n";
    out << "customThemePath = \"" << m_customThemePath << "\"\n";
    out << "locale = \"" << m_locale << "\"\n";
    out << "uiFontFamily = \"" << m_uiFontFamily << "\"\n";
    out << "uiFontSize = " << m_uiFontSize << "\n";
    out << "uiScale = " << m_uiScale << "\n";
    out << "sidebarWidth = " << m_sidebarWidth << "\n";
    out << "sidebarVisible = " << (m_sidebarVisible ? "true" : "false") << "\n";
    out << "treeMaxDepth = " << m_treeMaxDepth << "\n";
    out << "onboardingDismissed = " << (m_onboardingDismissed ? "true" : "false") << "\n\n";

    out << "[lsp]\n";
    out << "lspEnabled = " << (m_lspEnabled ? "true" : "false") << "\n\n";

    out << "[vim]\n";
    out << "vimMotionsEnabled = " << (m_vimMotionsEnabled ? "true" : "false") << "\n\n";

    out << "[projects]\n";
    out << "recentProjects = [";
    for (int i = 0; i < m_recentProjects.size(); ++i) {
        out << "\"" << m_recentProjects[i] << "\"";
        if (i < m_recentProjects.size() - 1)
            out << ", ";
    }
    out << "]\n";
}

void TomlSettingsStore::setUiFontFamily(const QString &family)
{
    if (m_uiFontFamily != family) { m_uiFontFamily = family; save(); }
}

void TomlSettingsStore::setUiFontSize(int size)
{
    if (size >= 6 && m_uiFontSize != size) { m_uiFontSize = size; save(); }
}

void TomlSettingsStore::setEditorFontFamily(const QString &family)
{
    if (m_editorFontFamily != family) { m_editorFontFamily = family; save(); }
}

void TomlSettingsStore::setEditorFontSize(int size)
{
    if (size >= 6 && m_editorFontSize != size) { m_editorFontSize = size; save(); }
}

void TomlSettingsStore::setRenderingStrategy(const QString &strategy)
{
    if ((strategy == "antialias" || strategy == "no-antialias" || strategy == "default") && m_renderingStrategy != strategy) { m_renderingStrategy = strategy; save(); }
}

void TomlSettingsStore::setUiScale(int percent)
{
    if (percent >= 75 && percent <= 200 && m_uiScale != percent) { m_uiScale = percent; save(); }
}

void TomlSettingsStore::setVimMotionsEnabled(bool enabled)
{
    if (m_vimMotionsEnabled != enabled) { m_vimMotionsEnabled = enabled; save(); }
}

void TomlSettingsStore::addRecentProject(const QString &project)
{
    if (project.isEmpty()) return;
    m_recentProjects.removeAll(project);
    m_recentProjects.prepend(project);
    while (m_recentProjects.size() > 10) {
        m_recentProjects.removeLast();
    }
    save();
}
