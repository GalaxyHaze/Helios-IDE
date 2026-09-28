#include "ThemeManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QDebug>
#include <QResource>
#include <QStandardPaths>

namespace {
bool isDarkPalette(const QPalette &palette)
{
    return palette.color(QPalette::Window).lightness() < 128;
}
}

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
    const QString resourceName = QString(":/appdata/themes/%1.json").arg(themeName);
    if (QFileInfo::exists(resourceName))
        return resourceName;

    QString path = QCoreApplication::applicationDirPath() + "/themes/" + themeName + ".json";
    if (QFileInfo::exists(path)) return path;

    const QString appDataDir = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    path = QDir(appDataDir).filePath("themes/" + themeName + ".json");
    if (QFileInfo::exists(path)) return path;

    return "";
}

void ThemeManager::setFallbackTheme(bool dark)
{
    const ThemeDefinition definition = ThemeDefinitionParser::fallback(dark);
    m_palette = definition.palette;
    m_customColors = definition.customColors;
    m_syntaxStyles = definition.syntaxStyles;
    m_isDark = dark;
}

bool ThemeManager::loadTheme(const QString &themeName)
{
    if (themeName == m_currentThemeName)
        return m_currentThemeWasLoaded;

    const QString path = findThemeFile(themeName);
    QFile file(path);
    if (path.isEmpty() || !file.open(QIODevice::ReadOnly)) {
        // Keep the last known-good theme.  Falling back is reserved for the
        // initial startup path, where no valid theme has been loaded yet.
        if (!m_hasValidTheme) {
            setFallbackTheme(themeName.contains("dark", Qt::CaseInsensitive));
            m_currentThemeName = themeName;
            m_currentThemeWasLoaded = false;
        }
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (doc.isNull() || !doc.isObject()) {
        if (!m_hasValidTheme) {
            setFallbackTheme(themeName.contains("dark", Qt::CaseInsensitive));
            m_currentThemeName = themeName;
            m_currentThemeWasLoaded = false;
        }
        return false;
    }

    return loadThemeDocument(doc, themeName,
                             themeName.contains("dark", Qt::CaseInsensitive));
}

bool ThemeManager::loadThemeFile(const QString &path, const QString &displayName)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (!m_hasValidTheme) {
            setFallbackTheme(true);
            m_currentThemeName = displayName.isEmpty() ? QFileInfo(path).baseName()
                                                       : displayName;
            m_currentThemeWasLoaded = false;
        }
        return false;
    }

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (doc.isNull() || !doc.isObject()) {
        if (!m_hasValidTheme) {
            setFallbackTheme(true);
            m_currentThemeName = displayName.isEmpty() ? QFileInfo(path).baseName()
                                                       : displayName;
            m_currentThemeWasLoaded = false;
        }
        return false;
    }

    const QString themeName =
        displayName.isEmpty() ? QFileInfo(path).baseName() : displayName;
    return loadThemeDocument(doc, themeName, true);
}

bool ThemeManager::loadThemeDocument(const QJsonDocument &doc,
                                     const QString &themeName,
                                     bool fallbackDark)
{
    const std::optional<ThemeDefinition> definition =
        ThemeDefinitionParser::parse(doc, fallbackDark);
    if (!definition.has_value())
        return false;

    m_palette = definition->palette;
    m_customColors = definition->customColors;
    m_syntaxStyles = definition->syntaxStyles;
    m_currentThemeName = themeName;
    m_isDark = isDarkPalette(m_palette);
    m_currentThemeWasLoaded = true;
    m_hasValidTheme = true;

    emit themeChanged();
    return true;
}

QColor ThemeManager::customColor(const QString &key, const QColor &fallback) const
{
    return m_customColors.value(key, fallback);
}

QColor ThemeManager::semanticColor(SemanticRole role, const QColor &fallback) const
{
    QColor color;
    switch (role) {
    case SemanticRole::Canvas:
        color = customColor(QStringLiteral("window"),
                            m_palette.color(QPalette::Window));
        break;
    case SemanticRole::Surface:
        color = customColor(QStringLiteral("sidebar"),
                            m_palette.color(QPalette::Window));
        break;
    case SemanticRole::SurfaceAlt:
        color = customColor(QStringLiteral("tabWidgetPane"),
                            m_palette.color(QPalette::Base));
        break;
    case SemanticRole::SurfaceMuted:
        color = customColor(QStringLiteral("tabBarBg"),
                            m_palette.color(QPalette::Window));
        break;
    case SemanticRole::Border:
        color = customColor(QStringLiteral("sidebarBorder"),
                            m_palette.color(QPalette::AlternateBase));
        break;
    case SemanticRole::BorderStrong:
        color = customColor(QStringLiteral("tabBorder"),
                            semanticColor(SemanticRole::Border));
        break;
    case SemanticRole::Text:
        color = m_palette.color(QPalette::Text);
        break;
    case SemanticRole::TextMuted:
        color = customColor(QStringLiteral("tabFg"),
                            m_palette.color(QPalette::Text));
        break;
    case SemanticRole::TextFaint:
        color = customColor(QStringLiteral("editorLineNumber"),
                            semanticColor(SemanticRole::TextMuted));
        break;
    case SemanticRole::Hover:
        color = customColor(QStringLiteral("treeHover"),
                            m_palette.color(QPalette::Highlight));
        break;
    case SemanticRole::Selected:
        color = customColor(QStringLiteral("treeSelected"),
                            m_palette.color(QPalette::Highlight));
        break;
    case SemanticRole::SelectedText:
        color = customColor(QStringLiteral("treeSelectedFg"),
                            m_palette.color(QPalette::HighlightedText));
        break;
    case SemanticRole::InputBg:
        color = m_palette.color(QPalette::Base);
        break;
    case SemanticRole::InputText:
        color = m_palette.color(QPalette::Text);
        break;
    case SemanticRole::ButtonBg:
        color = m_palette.color(QPalette::Button);
        break;
    case SemanticRole::ButtonText:
        color = m_palette.color(QPalette::ButtonText);
        break;
    case SemanticRole::ButtonHover:
        color = semanticColor(SemanticRole::Hover);
        break;
    case SemanticRole::Accent:
        color = m_palette.color(QPalette::Link);
        break;
    case SemanticRole::AccentHover:
        color = customColor(QStringLiteral("sidebarActiveBorder"),
                            m_palette.color(QPalette::Link));
        break;
    case SemanticRole::OnAccent:
        color = m_palette.color(QPalette::HighlightedText);
        break;
    case SemanticRole::Error:
        color = customColor(QStringLiteral("diagnosticError"),
                            isDark() ? QColor("#ff7a90") : QColor("#c4495f"));
        break;
    case SemanticRole::Warning:
        color = customColor(QStringLiteral("diagnosticWarning"),
                            isDark() ? QColor("#f1c77a") : QColor("#ab701d"));
        break;
    case SemanticRole::Info:
        color = customColor(QStringLiteral("diagnosticInfo"),
                            isDark() ? QColor("#70c9f0") : QColor("#2e739b"));
        break;
    case SemanticRole::Success:
        color = isDark() ? QColor("#a6d189") : QColor("#237a57");
        break;
    }

    if (!color.isValid())
        color = fallback;
    if (!color.isValid())
        color = m_palette.color(QPalette::WindowText);
    return color;
}

SyntaxStyle ThemeManager::syntaxStyle(const QString &key) const
{
    return m_syntaxStyles.value(key, {m_palette.color(QPalette::Text), false, false});
}
