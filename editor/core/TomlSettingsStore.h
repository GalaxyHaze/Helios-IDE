#ifndef TOMLSETTINGSSTORE_H
#define TOMLSETTINGSSTORE_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QObject>

#include "TomlSettingsCodec.h"

class TomlSettingsStore : public QObject
{
    Q_OBJECT
public:
    explicit TomlSettingsStore(QObject *parent = nullptr);

    static TomlSettingsStore &instance();
#ifdef HELIOS_UNIT_TESTING
    void setConfigDirForTesting(const QString &dir)
    {
        m_overrideDir = dir;
        load();
    }
#endif

    void load();
    void save();

    QString theme() const { return m_theme; }
    void setTheme(const QString &theme)
    {
        if (m_theme == theme)
            return;
        m_theme = theme;
        save();
    }
    QString customThemePath() const { return m_customThemePath; }
    void setCustomThemePath(const QString &path)
    {
        if (m_customThemePath == path)
            return;
        m_customThemePath = path;
        save();
    }

    QString locale() const { return m_locale; }
    void setLocale(const QString &locale)
    {
        if (m_locale == locale)
            return;
        m_locale = locale;
        save();
    }

    bool lspEnabled() const { return m_lspEnabled; }
    bool useOnlineZithLsp() const { return m_useOnlineZithLsp; }
    void setUseOnlineZithLsp(bool enabled)
    {
        if (m_useOnlineZithLsp == enabled)
            return;
        m_useOnlineZithLsp = enabled;
        save();
    }

    void setLspEnabled(bool enabled)
    {
        if (m_lspEnabled == enabled)
            return;
        m_lspEnabled = enabled;
        save();
    }

    bool cLspEnabled() const { return m_cLspEnabled; }
    void setCLspEnabled(bool enabled)
    {
        if (m_cLspEnabled == enabled)
            return;
        m_cLspEnabled = enabled;
        save();
    }

    QString cLspPath() const { return m_cLspPath; }
    void setCLspPath(const QString &path)
    {
        if (m_cLspPath == path)
            return;
        m_cLspPath = path;
        save();
    }

    QByteArray mainWindowGeometry() const
    {
        return QByteArray::fromBase64(m_mainWindowGeometryBase64.toLatin1());
    }
    void setMainWindowGeometry(const QByteArray &geometry)
    {
        const QString encoded = QString::fromLatin1(geometry.toBase64());
        if (m_mainWindowGeometryBase64 == encoded)
            return;
        m_mainWindowGeometryBase64 = encoded;
        save();
    }

    QByteArray mainWindowState() const
    {
        return QByteArray::fromBase64(m_mainWindowStateBase64.toLatin1());
    }
    void setMainWindowState(const QByteArray &state)
    {
        const QString encoded = QString::fromLatin1(state.toBase64());
        if (m_mainWindowStateBase64 == encoded)
            return;
        m_mainWindowStateBase64 = encoded;
        save();
    }

    // Compatibility aliases for settings written before appearance was split.
    QString fontFamily() const { return m_uiFontFamily; }
    void setFontFamily(const QString &family) { setUiFontFamily(family); }
    int fontSize() const { return m_uiFontSize; }
    void setFontSize(int size) { setUiFontSize(size); }

    QString uiFontFamily() const { return m_uiFontFamily; }
    void setUiFontFamily(const QString &family);
    int uiFontSize() const { return m_uiFontSize; }
    void setUiFontSize(int size);
    QString editorFontFamily() const { return m_editorFontFamily; }
    void setEditorFontFamily(const QString &family);
    int editorFontSize() const { return m_editorFontSize; }
    void setEditorFontSize(int size);
    QString renderingStrategy() const { return m_renderingStrategy; }
    void setRenderingStrategy(const QString &strategy);
    int uiScale() const { return m_uiScale; }
    void setUiScale(int percent);
    bool vimMotionsEnabled() const { return m_vimMotionsEnabled; }
    void setVimMotionsEnabled(bool enabled);

    QStringList searchTextExtensions() const { return m_searchTextExtensions; }
    void setSearchTextExtensions(const QStringList &extensions);
    QStringList searchExcludedDirs() const { return m_searchExcludedDirs; }
    void setSearchExcludedDirs(const QStringList &dirs);

    bool wordWrap() const { return m_wordWrap; }
    void setWordWrap(bool wrap);

    int sidebarWidth() const { return m_sidebarWidth; }
    void setSidebarWidth(int width) { m_sidebarWidth = width; save(); }

    bool sidebarVisible() const { return m_sidebarVisible; }
    void setSidebarVisible(bool visible) { m_sidebarVisible = visible; save(); }

    bool outlineVisible() const { return m_outlineVisible; }
    void setOutlineVisible(bool visible) { m_outlineVisible = visible; save(); }

    int treeMaxDepth() const { return m_treeMaxDepth; }
    void setTreeMaxDepth(int depth) { m_treeMaxDepth = depth; save(); }

    bool onboardingDismissed() const { return m_onboardingDismissed; }
    void setOnboardingDismissed(bool dismissed) { m_onboardingDismissed = dismissed; save(); }

    QStringList recentProjects() const { return m_recentProjects; }
    void setRecentProjects(const QStringList &projects) { m_recentProjects = projects; save(); }
    void addRecentProject(const QString &project);

signals:
    void editorPreferencesChanged();

private:
#ifdef HELIOS_UNIT_TESTING
    QString m_overrideDir;
#endif
    QString m_theme = "helios-dark";
    QString m_customThemePath;
    QString m_locale = "en-US";
    QString m_uiFontFamily = "System Default";
    int m_uiFontSize = 13;
    QString m_editorFontFamily = "System Default";
    int m_editorFontSize = 13;
    QString m_renderingStrategy = "antialias";
    int m_uiScale = 100;
    bool m_vimMotionsEnabled = false;
    bool m_wordWrap = false;
    QStringList m_searchTextExtensions = {
        "zith", "toml", "json", "md", "txt", "cpp", "cc", "cxx", "c", "h",
        "hpp", "qml", "cmake", "makefile", "dockerfile", "sh", "py", "rs",
        "go", "js", "ts", "yml", "yaml", "xml", "ini", "env", "rc"
    };
    QStringList m_searchExcludedDirs = {".git", "build", "node_modules", "venv"};
    int m_sidebarWidth = 320;
    bool m_sidebarVisible = true;
    bool m_outlineVisible = false;
    int m_treeMaxDepth = 12;
    bool m_onboardingDismissed = false;
    bool m_lspEnabled = false;
    bool m_useOnlineZithLsp = false;
    bool m_cLspEnabled = true;
    QString m_cLspPath;
    QString m_mainWindowGeometryBase64;
    QString m_mainWindowStateBase64;
    QStringList m_recentProjects;

    TomlSettingsSnapshot snapshot() const;
    void applySnapshot(const TomlSettingsSnapshot &snapshot);
    QString filePath() const;
};

#endif
