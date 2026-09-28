#include "TomlSettingsStore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

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
#ifdef HELIOS_UNIT_TESTING
    if (!m_overrideDir.isEmpty())
        return QDir(m_overrideDir).filePath("settings.toml");
#endif
    const QString directory =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(directory).filePath("settings.toml");
}

void TomlSettingsStore::load()
{
    QFile file(filePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    const TomlSettingsSnapshot loaded =
        TomlSettingsCodec::parse(QString::fromUtf8(file.readAll()),
                                 snapshot());
    applySnapshot(loaded);
}

void TomlSettingsStore::save()
{
    const QString path = filePath();
    const QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    file.write(TomlSettingsCodec::serialize(snapshot()).toUtf8());
}

TomlSettingsSnapshot TomlSettingsStore::snapshot() const
{
    TomlSettingsSnapshot result;
    result.theme = m_theme;
    result.customThemePath = m_customThemePath;
    result.locale = m_locale;
    result.uiFontFamily = m_uiFontFamily;
    result.uiFontSize = m_uiFontSize;
    result.editorFontFamily = m_editorFontFamily;
    result.editorFontSize = m_editorFontSize;
    result.renderingStrategy = m_renderingStrategy;
    result.uiScale = m_uiScale;
    result.vimMotionsEnabled = m_vimMotionsEnabled;
    result.wordWrap = m_wordWrap;
    result.searchTextExtensions = m_searchTextExtensions;
    result.searchExcludedDirs = m_searchExcludedDirs;
    result.sidebarWidth = m_sidebarWidth;
    result.sidebarVisible = m_sidebarVisible;
    result.outlineVisible = m_outlineVisible;
    result.treeMaxDepth = m_treeMaxDepth;
    result.onboardingDismissed = m_onboardingDismissed;
    result.lspEnabled = m_lspEnabled;
    result.useOnlineZithLsp = m_useOnlineZithLsp;
    result.cLspEnabled = m_cLspEnabled;
    result.cLspPath = m_cLspPath;
    result.mainWindowGeometryBase64 = m_mainWindowGeometryBase64;
    result.mainWindowStateBase64 = m_mainWindowStateBase64;
    result.recentProjects = m_recentProjects;
    return result;
}

void TomlSettingsStore::applySnapshot(
    const TomlSettingsSnapshot &snapshot)
{
    m_theme = snapshot.theme;
    m_customThemePath = snapshot.customThemePath;
    m_locale = snapshot.locale;
    m_uiFontFamily = snapshot.uiFontFamily;
    m_uiFontSize = snapshot.uiFontSize;
    m_editorFontFamily = snapshot.editorFontFamily;
    m_editorFontSize = snapshot.editorFontSize;
    m_renderingStrategy = snapshot.renderingStrategy;
    m_uiScale = snapshot.uiScale;
    m_vimMotionsEnabled = snapshot.vimMotionsEnabled;
    m_wordWrap = snapshot.wordWrap;
    m_searchTextExtensions = snapshot.searchTextExtensions;
    m_searchExcludedDirs = snapshot.searchExcludedDirs;
    m_sidebarWidth = snapshot.sidebarWidth;
    m_sidebarVisible = snapshot.sidebarVisible;
    m_outlineVisible = snapshot.outlineVisible;
    m_treeMaxDepth = snapshot.treeMaxDepth;
    m_onboardingDismissed = snapshot.onboardingDismissed;
    m_lspEnabled = snapshot.lspEnabled;
    m_useOnlineZithLsp = snapshot.useOnlineZithLsp;
    m_cLspEnabled = snapshot.cLspEnabled;
    m_cLspPath = snapshot.cLspPath;
    m_mainWindowGeometryBase64 = snapshot.mainWindowGeometryBase64;
    m_mainWindowStateBase64 = snapshot.mainWindowStateBase64;
    m_recentProjects = snapshot.recentProjects;
}

void TomlSettingsStore::setUiFontFamily(const QString &family)
{
    if (m_uiFontFamily != family) {
        m_uiFontFamily = family;
        save();
    }
}

void TomlSettingsStore::setUiFontSize(int size)
{
    if (size >= 6 && m_uiFontSize != size) {
        m_uiFontSize = size;
        save();
    }
}

void TomlSettingsStore::setEditorFontFamily(const QString &family)
{
    if (m_editorFontFamily != family) {
        m_editorFontFamily = family;
        save();
    }
}

void TomlSettingsStore::setEditorFontSize(int size)
{
    if (size >= 6 && m_editorFontSize != size) {
        m_editorFontSize = size;
        save();
    }
}

void TomlSettingsStore::setRenderingStrategy(const QString &strategy)
{
    const bool valid = strategy == "antialias" ||
                       strategy == "no-antialias" ||
                       strategy == "default";
    if (valid && m_renderingStrategy != strategy) {
        m_renderingStrategy = strategy;
        save();
    }
}

void TomlSettingsStore::setUiScale(int percent)
{
    if (percent >= 75 && percent <= 200 && m_uiScale != percent) {
        m_uiScale = percent;
        save();
    }
}

void TomlSettingsStore::setVimMotionsEnabled(bool enabled)
{
    if (m_vimMotionsEnabled == enabled)
        return;
    m_vimMotionsEnabled = enabled;
    save();
    emit editorPreferencesChanged();
}

void TomlSettingsStore::setWordWrap(bool wrap)
{
    if (m_wordWrap == wrap)
        return;
    m_wordWrap = wrap;
    save();
    emit editorPreferencesChanged();
}

void TomlSettingsStore::setSearchTextExtensions(
    const QStringList &extensions)
{
    if (m_searchTextExtensions == extensions)
        return;
    m_searchTextExtensions = extensions;
    save();
}

void TomlSettingsStore::setSearchExcludedDirs(const QStringList &dirs)
{
    if (m_searchExcludedDirs == dirs)
        return;
    m_searchExcludedDirs = dirs;
    save();
}

void TomlSettingsStore::addRecentProject(const QString &project)
{
    if (project.isEmpty())
        return;
    m_recentProjects.removeAll(project);
    m_recentProjects.prepend(project);
    while (m_recentProjects.size() > 10)
        m_recentProjects.removeLast();
    save();
}
