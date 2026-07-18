#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

class QCheckBox;
class QComboBox;
class QFontComboBox;
class QLabel;
class QPushButton;
class QSpinBox;

class PreferencesDialog : public QDialog {
  Q_OBJECT

public:
  explicit PreferencesDialog(QWidget *parent = nullptr);

private slots:
  void applyTranslations();
  void applyDeferredChanges();
  void chooseCustomTheme();

private:
  void setDeferredChangesPending(bool pending);
  void refreshThemeSelection();

  QLabel *m_hint = nullptr;
  QLabel *m_uiFontLabel = nullptr;
  QFontComboBox *m_uiFont = nullptr;
  QLabel *m_uiSizeLabel = nullptr;
  QSpinBox *m_uiSize = nullptr;
  QLabel *m_editorFontLabel = nullptr;
  QFontComboBox *m_editorFont = nullptr;
  QLabel *m_editorSizeLabel = nullptr;
  QSpinBox *m_editorSize = nullptr;
  QLabel *m_renderingLabel = nullptr;
  QComboBox *m_rendering = nullptr;
  QLabel *m_scaleLabel = nullptr;
  QComboBox *m_scale = nullptr;
  QLabel *m_localeLabel = nullptr;
  QComboBox *m_locale = nullptr;
  QLabel *m_themeLabel = nullptr;
  QComboBox *m_theme = nullptr;
  QPushButton *m_openThemeButton = nullptr;
  QLabel *m_customThemeLabel = nullptr;
  QCheckBox *m_vim = nullptr;
  QPushButton *m_applyButton = nullptr;

  QString m_pendingUiFontFamily;
  int m_pendingUiFontSize = 13;
  int m_pendingUiScale = 100;
  QString m_pendingLocale;
  bool m_hasDeferredChanges = false;
};

#endif
