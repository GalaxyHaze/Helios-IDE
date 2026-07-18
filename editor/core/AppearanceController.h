#ifndef APPEARANCECONTROLLER_H
#define APPEARANCECONTROLLER_H

#include <QFont>
#include <QObject>

class AppearanceController : public QObject {
  Q_OBJECT

public:
  explicit AppearanceController(QObject *parent = nullptr);
  static AppearanceController &instance();

  void apply();
  QFont uiFont() const;
  QFont editorFont() const;

  bool setTheme(const QString &themeName);
  bool setThemeFile(const QString &path);
  void setUiFontFamily(const QString &family);
  void setUiFontSize(int pointSize);
  void setEditorFontFamily(const QString &family);
  void setEditorFontSize(int pointSize);
  void setRenderingStrategy(const QString &strategy);
  void setUiScale(int percent);

signals:
  void appearanceChanged();

private:
  QFont fontFor(const QString &family, int pointSize, bool editor) const;
  QFont m_defaultUiFont;
};

#endif
