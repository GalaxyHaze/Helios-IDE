#include "AppearanceController.h"

#include "ThemeManager.h"
#include "TomlSettingsStore.h"

#include <QApplication>
#include <QFontDatabase>
#include <QRegularExpression>

AppearanceController::AppearanceController(QObject *parent)
    : QObject(parent), m_defaultUiFont(QApplication::font()) {
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
          [this]() { apply(); });
}

AppearanceController &AppearanceController::instance() {
  static AppearanceController controller;
  return controller;
}

QFont AppearanceController::fontFor(const QString &family, int pointSize,
                                    bool editor) const {
  QFont font;
  if (family != "System Default" && !family.isEmpty()) {
    font.setFamily(family);
  } else if (editor) {
    font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  } else {
    font = m_defaultUiFont;
  }

  const auto &settings = TomlSettingsStore::instance();
  const int effectivePointSize =
      editor ? pointSize
             : qRound(pointSize * settings.uiScale() / 100.0);
  font.setPointSize(qMax(6, effectivePointSize));
  if (editor) {
    font.setStyleHint(QFont::Monospace);
    const QString strategy = settings.renderingStrategy();
    font.setStyleStrategy(strategy == "no-antialias" ? QFont::NoAntialias
                                                     : QFont::PreferAntialias);
  }
  return font;
}

QFont AppearanceController::uiFont() const {
  const auto &settings = TomlSettingsStore::instance();
  return fontFor(settings.uiFontFamily(), settings.uiFontSize(), false);
}

QFont AppearanceController::uiLargeFont() const
{
  const auto &settings = TomlSettingsStore::instance();
  const int minFontsize = 18;
  int fontSize = qMax(minFontsize, settings.uiFontSize());
  return fontFor(settings.uiFontFamily(), fontSize, false);
}

bool AppearanceController::needsUiLargeFont(const QString &text) const
{
  static QRegularExpression cjkRegex("[\u4E00-\u9FFF\u3400-\u4DBF\u3000-\u303F\uFF00-\uFFEF]");
  return text.contains(cjkRegex);
}

QFont AppearanceController::editorFont() const {
  const auto &settings = TomlSettingsStore::instance();
  return fontFor(settings.editorFontFamily(), settings.editorFontSize(), true);
}

const int AppearanceController::minFontSize() const
{
  return 8;
}

void AppearanceController::apply() {
  QApplication::setPalette(ThemeManager::instance().palette());
  QApplication::setFont(uiFont());
  emit appearanceChanged();
}

bool AppearanceController::setTheme(const QString &themeName) {
  if (!ThemeManager::instance().loadTheme(themeName))
    return false;
  TomlSettingsStore::instance().setTheme(themeName);
  TomlSettingsStore::instance().setCustomThemePath(QString());
  apply();
  return true;
}

bool AppearanceController::setThemeFile(const QString &path) {
  if (!ThemeManager::instance().loadThemeFile(path))
    return false;
  auto &settings = TomlSettingsStore::instance();
  settings.setTheme("custom");
  settings.setCustomThemePath(path);
  apply();
  return true;
}

void AppearanceController::setUiFontFamily(const QString &family) {
  TomlSettingsStore::instance().setUiFontFamily(family);
  apply();
}

void AppearanceController::setUiFontSize(int pointSize) {
  TomlSettingsStore::instance().setUiFontSize(pointSize);
  apply();
}

void AppearanceController::setEditorFontFamily(const QString &family) {
  TomlSettingsStore::instance().setEditorFontFamily(family);
  emit appearanceChanged();
}

void AppearanceController::setEditorFontSize(int pointSize) {
  TomlSettingsStore::instance().setEditorFontSize(pointSize);
  emit appearanceChanged();
}

void AppearanceController::setRenderingStrategy(const QString &strategy) {
  TomlSettingsStore::instance().setRenderingStrategy(strategy);
  emit appearanceChanged();
}

void AppearanceController::setUiScale(int percent) {
  TomlSettingsStore::instance().setUiScale(percent);
  apply();
}
