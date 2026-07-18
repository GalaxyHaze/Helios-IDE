#ifndef TOMLSETTINGSSTORE_H
#define TOMLSETTINGSSTORE_H

#include <QString>
#include <QStringList>
#include <QObject>

class TomlSettingsStore : public QObject
{
    Q_OBJECT
public:
    explicit TomlSettingsStore(QObject *parent = nullptr);

    static TomlSettingsStore &instance();

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
    void setLspEnabled(bool enabled)
    {
        if (m_lspEnabled == enabled)
            return;
        m_lspEnabled = enabled;
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

    bool wordWrap() const { return m_wordWrap; }
    void setWordWrap(bool wrap) { m_wordWrap = wrap; save(); }

    int sidebarWidth() const { return m_sidebarWidth; }
    void setSidebarWidth(int width) { m_sidebarWidth = width; save(); }

    bool sidebarVisible() const { return m_sidebarVisible; }
    void setSidebarVisible(bool visible) { m_sidebarVisible = visible; save(); }

    int treeMaxDepth() const { return m_treeMaxDepth; }
    void setTreeMaxDepth(int depth) { m_treeMaxDepth = depth; save(); }

    bool onboardingDismissed() const { return m_onboardingDismissed; }
    void setOnboardingDismissed(bool dismissed) { m_onboardingDismissed = dismissed; save(); }

    QStringList recentProjects() const { return m_recentProjects; }
    void setRecentProjects(const QStringList &projects) { m_recentProjects = projects; save(); }
    void addRecentProject(const QString &project);

private:
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
    int m_sidebarWidth = 280;
    bool m_sidebarVisible = true;
    int m_treeMaxDepth = 12;
    bool m_onboardingDismissed = false;
    bool m_lspEnabled = false;
    QStringList m_recentProjects;

    QString filePath() const;
};

#endif
