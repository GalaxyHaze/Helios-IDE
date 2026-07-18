#include "PreferencesDialog.h"

#include "../core/AppearanceController.h"
#include "../core/ThemeManager.h"
#include "../core/TomlSettingsStore.h"
#include "../core/TranslationManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFontComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {
constexpr auto kCustomThemeId = "custom";
}

PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Preferences");
  setModal(false);
  resize(520, 460);

  const auto &settings = TomlSettingsStore::instance();
  m_pendingUiFontFamily = settings.uiFontFamily();
  m_pendingUiFontSize = settings.uiFontSize();
  m_pendingUiScale = settings.uiScale();
  m_pendingLocale = settings.locale();

  auto *layout = new QVBoxLayout(this);
  m_hint = new QLabel(this);
  m_hint->setWordWrap(true);
  layout->addWidget(m_hint);

  auto *form = new QFormLayout;

  m_uiFontLabel = new QLabel(this);
  m_uiFont = new QFontComboBox(this);
  m_uiFont->setCurrentFont(AppearanceController::instance().uiFont());
  form->addRow(m_uiFontLabel, m_uiFont);

  m_uiSizeLabel = new QLabel(this);
  m_uiSize = new QSpinBox(this);
  m_uiSize->setRange(8, 32);
  m_uiSize->setValue(settings.uiFontSize());
  form->addRow(m_uiSizeLabel, m_uiSize);

  m_editorFontLabel = new QLabel(this);
  m_editorFont = new QFontComboBox(this);
  m_editorFont->setFontFilters(QFontComboBox::MonospacedFonts);
  m_editorFont->setCurrentFont(AppearanceController::instance().editorFont());
  form->addRow(m_editorFontLabel, m_editorFont);

  m_editorSizeLabel = new QLabel(this);
  m_editorSize = new QSpinBox(this);
  m_editorSize->setRange(8, 48);
  m_editorSize->setValue(settings.editorFontSize());
  form->addRow(m_editorSizeLabel, m_editorSize);

  m_renderingLabel = new QLabel(this);
  m_rendering = new QComboBox(this);
  m_rendering->addItem("Antialias", "antialias");
  m_rendering->addItem("Default", "default");
  m_rendering->addItem("No antialias", "no-antialias");
  m_rendering->setCurrentIndex(
      qMax(0, m_rendering->findData(settings.renderingStrategy())));
  form->addRow(m_renderingLabel, m_rendering);

  m_scaleLabel = new QLabel(this);
  m_scale = new QComboBox(this);
  for (const int scale : {75, 90, 100, 110, 125, 150, 175, 200})
    m_scale->addItem(QString::number(scale) + "%", scale);
  m_scale->setCurrentIndex(qMax(0, m_scale->findData(settings.uiScale())));
  form->addRow(m_scaleLabel, m_scale);

  m_localeLabel = new QLabel(this);
  m_locale = new QComboBox(this);
  m_locale->addItem("English (US)", "en-US");
  m_locale->addItem("Portuguese (BR)", "pt-BR");
  m_locale->setCurrentIndex(qMax(0, m_locale->findData(settings.locale())));
  form->addRow(m_localeLabel, m_locale);

  m_themeLabel = new QLabel(this);
  m_theme = new QComboBox(this);
  const QList<QPair<QString, QString>> themes = {
      {"Helios Dark", "helios-dark"},
      {"Helios Light", "helios-light"},
      {"ColorMind Midnight", "colormind-midnight"},
      {"ColorMind Desert", "colormind-desert"},
      {"Helios Violet", "helios-violet-dark"},
      {"Helios Red", "helios-red-dark"},
      {"Helios Gold", "helios-gold-dark"},
      {"Helios Blue", "helios-blue-dark"},
      {"Helios Forest", "helios-forest-dark"},
      {"Helios Carbon", "helios-carbon-dark"},
      {"Custom JSON Theme", kCustomThemeId}};
  for (const auto &theme : themes)
    m_theme->addItem(theme.first, theme.second);
  form->addRow(m_themeLabel, m_theme);

  m_openThemeButton = new QPushButton(this);
  form->addRow(QString(), m_openThemeButton);

  m_customThemeLabel = new QLabel(this);
  m_customThemeLabel->setWordWrap(true);
  form->addRow(QString(), m_customThemeLabel);

  m_vim = new QCheckBox(this);
  m_vim->setChecked(settings.vimMotionsEnabled());
  form->addRow(QString(), m_vim);
  layout->addLayout(form);

  auto *buttons =
      new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Close,
                           this);
  m_applyButton = buttons->button(QDialogButtonBox::Apply);
  m_applyButton->setEnabled(false);
  connect(m_applyButton, &QPushButton::clicked, this,
          &PreferencesDialog::applyDeferredChanges);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::close);
  layout->addWidget(buttons);

  auto &appearance = AppearanceController::instance();
  connect(&TranslationManager::instance(), &TranslationManager::localeChanged,
          this, &PreferencesDialog::applyTranslations);
  connect(m_uiFont, &QFontComboBox::currentFontChanged, this,
          [this](const QFont &font) {
            m_pendingUiFontFamily = font.family();
            setDeferredChangesPending(true);
          });
  connect(m_uiSize, QOverload<int>::of(&QSpinBox::valueChanged), this,
          [this](int size) {
            m_pendingUiFontSize = size;
            setDeferredChangesPending(true);
          });
  connect(m_editorFont, &QFontComboBox::currentFontChanged, this,
          [&appearance](const QFont &font) {
            appearance.setEditorFontFamily(font.family());
          });
  connect(m_editorSize, QOverload<int>::of(&QSpinBox::valueChanged), this,
          [&appearance](int size) { appearance.setEditorFontSize(size); });
  connect(m_rendering, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this, &appearance](int index) {
            appearance.setRenderingStrategy(
                m_rendering->itemData(index).toString());
          });
  connect(m_scale, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [this](int index) {
            m_pendingUiScale = m_scale->itemData(index).toInt();
            setDeferredChangesPending(true);
          });
  connect(m_locale, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [this](int index) {
            m_pendingLocale = m_locale->itemData(index).toString();
            setDeferredChangesPending(true);
          });
  connect(m_theme, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          [this, &appearance](int index) {
            const QString themeId = m_theme->itemData(index).toString();
            if (themeId == kCustomThemeId) {
              refreshThemeSelection();
              return;
            }
            appearance.setTheme(themeId);
            refreshThemeSelection();
          });
  connect(m_openThemeButton, &QPushButton::clicked, this,
          &PreferencesDialog::chooseCustomTheme);
  connect(m_vim, &QCheckBox::toggled, this, [](bool enabled) {
    TomlSettingsStore::instance().setVimMotionsEnabled(enabled);
  });

  applyTranslations();
  refreshThemeSelection();
}

void PreferencesDialog::applyTranslations() {
  auto &tr = TranslationManager::instance();
  setWindowTitle(tr.translate("prefs.title"));
  m_hint->setText(tr.translate("prefs.hint"));
  m_uiFontLabel->setText(tr.translate("prefs.ui_font"));
  m_uiSizeLabel->setText(tr.translate("prefs.ui_size"));
  m_editorFontLabel->setText(tr.translate("prefs.editor_font"));
  m_editorSizeLabel->setText(tr.translate("prefs.editor_size"));
  m_renderingLabel->setText(tr.translate("prefs.rendering"));
  m_scaleLabel->setText(tr.translate("prefs.ui_scale"));
  m_localeLabel->setText(tr.translate("prefs.language"));
  m_themeLabel->setText(tr.translate("prefs.theme"));
  m_openThemeButton->setText(tr.translate("prefs.open_theme"));
  m_vim->setText(tr.translate("prefs.vim"));
  refreshThemeSelection();
}

void PreferencesDialog::applyDeferredChanges() {
  auto &appearance = AppearanceController::instance();
  appearance.setUiFontFamily(m_pendingUiFontFamily);
  appearance.setUiFontSize(m_pendingUiFontSize);
  appearance.setUiScale(m_pendingUiScale);

  auto &store = TomlSettingsStore::instance();
  store.setLocale(m_pendingLocale);
  TranslationManager::instance().loadLocale(m_pendingLocale);
  setDeferredChangesPending(false);
}

void PreferencesDialog::chooseCustomTheme() {
  const QString filePath = QFileDialog::getOpenFileName(
      this, TranslationManager::instance().translate("prefs.open_theme_title"),
      QString(), "JSON Theme (*.json)");
  if (filePath.isEmpty())
    return;

  if (!AppearanceController::instance().setThemeFile(filePath)) {
    QMessageBox::warning(
        this, TranslationManager::instance().translate("prefs.theme_error_title"),
        TranslationManager::instance().translate("prefs.theme_error_message"));
    refreshThemeSelection();
    return;
  }

  refreshThemeSelection();
}

void PreferencesDialog::setDeferredChangesPending(bool pending) {
  m_hasDeferredChanges = pending;
  if (m_applyButton)
    m_applyButton->setEnabled(m_hasDeferredChanges);
}

void PreferencesDialog::refreshThemeSelection() {
  const auto &store = TomlSettingsStore::instance();
  const QString themeId = store.theme();
  const QString customThemePath = store.customThemePath();

  {
    QSignalBlocker blocker(m_theme);
    if (themeId == kCustomThemeId && !customThemePath.isEmpty()) {
      m_theme->setCurrentIndex(qMax(0, m_theme->findData(kCustomThemeId)));
    } else {
      m_theme->setCurrentIndex(qMax(0, m_theme->findData(themeId)));
    }
  }

  if (themeId == kCustomThemeId && !customThemePath.isEmpty()) {
    m_customThemeLabel->setText(
        TranslationManager::instance().translate("prefs.custom_theme") + ": " +
        customThemePath);
    m_customThemeLabel->show();
  } else {
    m_customThemeLabel->setText(
        TranslationManager::instance().translate("prefs.custom_theme_none"));
    m_customThemeLabel->show();
  }
}
