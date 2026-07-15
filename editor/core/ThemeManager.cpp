#include "ThemeManager.h"
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    // Default fallback setup
    setFallbackTheme(true);
}

ThemeManager &ThemeManager::instance()
{
    static ThemeManager manager;
    return manager;
}

QString ThemeManager::findThemeFile(const QString &themeName)
{
    QString appDir = QCoreApplication::applicationDirPath() + "/themes/";
    QString path = appDir + themeName + ".json";
    if (QFileInfo::exists(path)) return path;

    path = "/home/diogo/Helios/themes/" + themeName + ".json";
    if (QFileInfo::exists(path)) return path;

    return "";
}

void ThemeManager::setFallbackTheme(bool dark)
{
    m_isDark = dark;
    m_customColors.clear();

    if (dark) {
        m_palette.setColor(QPalette::Window, QColor("#11131a"));
        m_palette.setColor(QPalette::WindowText, QColor("#e6e9f2"));
        m_palette.setColor(QPalette::Base, QColor("#141720"));
        m_palette.setColor(QPalette::AlternateBase, QColor("#1d2230"));
        m_palette.setColor(QPalette::ToolTipBase, QColor("#252b3a"));
        m_palette.setColor(QPalette::ToolTipText, QColor("#edf0fa"));
        m_palette.setColor(QPalette::Text, QColor("#d9deeb"));
        m_palette.setColor(QPalette::Button, QColor("#202638"));
        m_palette.setColor(QPalette::ButtonText, QColor("#e6e9f2"));
        m_palette.setColor(QPalette::BrightText, QColor("#ff7a90"));
        m_palette.setColor(QPalette::Link, QColor("#8fa2ff"));
        m_palette.setColor(QPalette::Highlight, QColor("#3b5ccc"));
        m_palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));

        m_customColors["sidebar"] = QColor("#0e1016");
        m_customColors["sidebarBorder"] = QColor("#272d3d");
        m_customColors["sidebarHover"] = QColor("#20283a");
        m_customColors["sidebarActive"] = QColor("#1d2537");
        m_customColors["sidebarActiveBorder"] = QColor("#8fa2ff");
        m_customColors["tabWidgetPane"] = QColor("#141720");
        m_customColors["tabBarBg"] = QColor("#11131a");
        m_customColors["tabBg"] = QColor("#11131a");
        m_customColors["tabFg"] = QColor("#8991a5");
        m_customColors["tabBorder"] = QColor("#272d3d");
        m_customColors["tabSelectedFg"] = QColor("#f1f3fb");
        m_customColors["tabHoverBg"] = QColor("#1d2532");
        m_customColors["treeBg"] = QColor("#141720");
        m_customColors["treeFg"] = QColor("#d1d7e5");
        m_customColors["treeHover"] = QColor("#27324a");
        m_customColors["treeSelected"] = QColor("#32436a");
        m_customColors["treeSelectedFg"] = QColor("#ffffff");
        m_customColors["treeBranch"] = QColor("#141720");
        m_customColors["editorBg"] = QColor("#1b1e2a");
        m_customColors["editorFg"] = QColor("#e6e9f2");
        m_customColors["editorLineNumber"] = QColor("#687188");
        m_customColors["editorCurrentLine"] = QColor("#23293a");
        m_customColors["editorSelection"] = QColor("#354b83");
        m_customColors["gutterBg"] = QColor("#151821");
        m_customColors["gutterActive"] = QColor("#aab8ff");
        m_customColors["bracketBg"] = QColor("#41537c");
        m_customColors["bracketFg"] = QColor("#ffffff");
        m_customColors["diagnosticError"] = QColor("#ff7a90");
        m_customColors["diagnosticWarning"] = QColor("#f1c77a");
        m_customColors["diagnosticInfo"] = QColor("#70c9f0");
        m_customColors["diagnosticUnknown"] = QColor("#aeb8ce");
    } else {
        m_palette.setColor(QPalette::Window, QColor("#e9edf2"));
        m_palette.setColor(QPalette::WindowText, QColor("#263043"));
        m_palette.setColor(QPalette::Base, QColor("#f3f5f7"));
        m_palette.setColor(QPalette::AlternateBase, QColor("#e2e7ed"));
        m_palette.setColor(QPalette::ToolTipBase, QColor("#f7f8fa"));
        m_palette.setColor(QPalette::ToolTipText, QColor("#263043"));
        m_palette.setColor(QPalette::Text, QColor("#2f3a4d"));
        m_palette.setColor(QPalette::Button, QColor("#dde4ec"));
        m_palette.setColor(QPalette::ButtonText, QColor("#283448"));
        m_palette.setColor(QPalette::BrightText, QColor("#c4495f"));
        m_palette.setColor(QPalette::Link, QColor("#3f64bd"));
        m_palette.setColor(QPalette::Highlight, QColor("#3f6fa3"));
        m_palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));

        m_customColors["sidebar"] = QColor("#e3e8ee");
        m_customColors["sidebarBorder"] = QColor("#c4ccd7");
        m_customColors["sidebarHover"] = QColor("#d5dde7");
        m_customColors["sidebarActive"] = QColor("#cbd7e5");
        m_customColors["sidebarActiveBorder"] = QColor("#4b6fae");
        m_customColors["tabWidgetPane"] = QColor("#f3f5f7");
        m_customColors["tabBarBg"] = QColor("#e9edf2");
        m_customColors["tabBg"] = QColor("#e9edf2");
        m_customColors["tabFg"] = QColor("#627087");
        m_customColors["tabBorder"] = QColor("#c4ccd7");
        m_customColors["tabSelectedFg"] = QColor("#263043");
        m_customColors["tabHoverBg"] = QColor("#dce3eb");
        m_customColors["treeBg"] = QColor("#e9edf2");
        m_customColors["treeFg"] = QColor("#2f3a4d");
        m_customColors["treeHover"] = QColor("#d4dee9");
        m_customColors["treeSelected"] = QColor("#bfcfe2");
        m_customColors["treeSelectedFg"] = QColor("#1e2a3d");
        m_customColors["treeBranch"] = QColor("#e9edf2");
        m_customColors["editorBg"] = QColor("#f8fafb");
        m_customColors["editorFg"] = QColor("#263043");
        m_customColors["editorLineNumber"] = QColor("#8793a5");
        m_customColors["editorCurrentLine"] = QColor("#eaf0f5");
        m_customColors["editorSelection"] = QColor("#b8cbe2");
        m_customColors["gutterBg"] = QColor("#edf1f5");
        m_customColors["gutterActive"] = QColor("#3f64a6");
        m_customColors["bracketBg"] = QColor("#c8d7e9");
        m_customColors["bracketFg"] = QColor("#1d2a40");
        m_customColors["diagnosticError"] = QColor("#c4495f");
        m_customColors["diagnosticWarning"] = QColor("#ab701d");
        m_customColors["diagnosticInfo"] = QColor("#2e739b");
        m_customColors["diagnosticUnknown"] = QColor("#637088");
    }
}

bool ThemeManager::loadTheme(const QString &themeName)
{
    if (themeName == m_currentThemeName)
        return m_currentThemeWasLoaded;

    QString path = findThemeFile(themeName);
    QFile file(path);
    if (path.isEmpty() || !file.open(QIODevice::ReadOnly)) {
        setFallbackTheme(themeName.contains("dark", Qt::CaseInsensitive));
        m_currentThemeName = themeName;
        m_currentThemeWasLoaded = false;
        emit themeChanged();
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (doc.isNull() || !doc.isObject()) {
        setFallbackTheme(themeName.contains("dark", Qt::CaseInsensitive));
        m_currentThemeName = themeName;
        m_currentThemeWasLoaded = false;
        emit themeChanged();
        return false;
    }

    QJsonObject obj = doc.object();
    m_currentThemeName = themeName;
    m_isDark = themeName.contains("dark", Qt::CaseInsensitive);
    m_currentThemeWasLoaded = true;

    QPalette pal;
    QJsonObject palObj = obj.value("palette").toObject();

    auto parseColor = [](const QJsonObject &o, const QString &k, const QString &def) -> QColor {
        QString hex = o.value(k).toString(def);
        return QColor(hex);
    };

    pal.setColor(QPalette::Window, parseColor(palObj, "window", m_isDark ? "#11131a" : "#e9edf2"));
    pal.setColor(QPalette::WindowText, parseColor(palObj, "windowText", m_isDark ? "#e6e9f2" : "#263043"));
    pal.setColor(QPalette::Base, parseColor(palObj, "base", m_isDark ? "#141720" : "#f3f5f7"));
    pal.setColor(QPalette::AlternateBase, parseColor(palObj, "alternateBase", m_isDark ? "#1d2230" : "#e2e7ed"));
    pal.setColor(QPalette::ToolTipBase, parseColor(palObj, "toolTipBase", m_isDark ? "#252b3a" : "#f7f8fa"));
    pal.setColor(QPalette::ToolTipText, parseColor(palObj, "toolTipText", m_isDark ? "#edf0fa" : "#263043"));
    pal.setColor(QPalette::Text, parseColor(palObj, "text", m_isDark ? "#d9deeb" : "#2f3a4d"));
    pal.setColor(QPalette::Button, parseColor(palObj, "button", m_isDark ? "#202638" : "#dde4ec"));
    pal.setColor(QPalette::ButtonText, parseColor(palObj, "buttonText", m_isDark ? "#e6e9f2" : "#283448"));
    pal.setColor(QPalette::BrightText, parseColor(palObj, "brightText", m_isDark ? "#ff7a90" : "#c4495f"));
    pal.setColor(QPalette::Link, parseColor(palObj, "link", m_isDark ? "#8fa2ff" : "#3f64bd"));
    pal.setColor(QPalette::Highlight, parseColor(palObj, "highlight", m_isDark ? "#3b5ccc" : "#3f6fa3"));
    pal.setColor(QPalette::HighlightedText, parseColor(palObj, "highlightedText", "#ffffff"));
    m_palette = pal;

    m_customColors.clear();
    QJsonObject custObj = obj.value("custom").toObject();
    for (auto it = custObj.begin(); it != custObj.end(); ++it) {
        m_customColors.insert(it.key(), QColor(it.value().toString()));
    }

    emit themeChanged();
    return true;
}

QColor ThemeManager::customColor(const QString &key, const QColor &fallback) const
{
    return m_customColors.value(key, fallback);
}
