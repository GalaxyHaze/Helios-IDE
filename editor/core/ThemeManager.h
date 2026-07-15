#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QMap>
#include <QPalette>

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    explicit ThemeManager(QObject *parent = nullptr);
    static ThemeManager &instance();

    bool loadTheme(const QString &themeName);
    QString currentThemeName() const { return m_currentThemeName; }

    QPalette palette() const { return m_palette; }
    QColor customColor(const QString &key, const QColor &fallback = QColor()) const;

    bool isDark() const { return m_isDark; }

signals:
    void themeChanged();

private:
    QString m_currentThemeName;
    QPalette m_palette;
    QMap<QString, QColor> m_customColors;
    bool m_isDark = true;
    bool m_currentThemeWasLoaded = false;

    void setFallbackTheme(bool dark);
    QString findThemeFile(const QString &themeName);
};

#endif
