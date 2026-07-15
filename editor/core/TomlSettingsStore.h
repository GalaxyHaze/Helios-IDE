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

    QString fontFamily() const { return m_fontFamily; }
    void setFontFamily(const QString &family) { m_fontFamily = family; save(); }

    int fontSize() const { return m_fontSize; }
    void setFontSize(int size) { m_fontSize = size; save(); }

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
    QString m_locale = "en-US";
    QString m_fontFamily = "System Default";
    int m_fontSize = 13;
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
