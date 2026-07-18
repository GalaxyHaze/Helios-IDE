#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QMap>
#include <QPalette>

struct SyntaxStyle
{
    QColor color;
    bool bold = false;
    bool italic = false;
};

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    explicit ThemeManager(QObject *parent = nullptr);
    static ThemeManager &instance();

    bool loadTheme(const QString &themeName);
    bool loadThemeFile(const QString &path, const QString &displayName = QString());
    QString currentThemeName() const { return m_currentThemeName; }

    QPalette palette() const { return m_palette; }
    QColor customColor(const QString &key, const QColor &fallback = QColor()) const;
    SyntaxStyle syntaxStyle(const QString &key) const;

    bool isDark() const { return m_isDark; }

signals:
    void themeChanged();

private:
    bool loadThemeDocument(const QJsonDocument &doc,
                           const QString &themeName,
                           bool fallbackDark);
    QString m_currentThemeName;
    QPalette m_palette;
    QMap<QString, QColor> m_customColors;
    QMap<QString, SyntaxStyle> m_syntaxStyles;
    bool m_isDark = true;
    bool m_currentThemeWasLoaded = false;
    bool m_hasValidTheme = false;

    void setFallbackTheme(bool dark);
    QString findThemeFile(const QString &themeName);
    void setFallbackSyntaxStyles();
};

#endif
