#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QWidget>

class QCheckBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QFontComboBox;
class QPlainTextEdit;
class QComboBox;
class QTabWidget;
class QTreeWidget;

class SettingsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget *parent = nullptr);

    void setFontFamily(const QString &family);
    void setFontSize(int pointSize);
    void setWordWrapEnabled(bool enabled);
    void setTheme(const QString &themeName);
    void setLocale(const QString &locale);
    void setLspEnabled(bool enabled);

    void setRuntimeInfo(const QString &status,
                        const QString &tag,
                        const QString &lspPath,
                        const QString &stdlibPath,
                        const QString &cachePath);
    void setLspDiagnostics(const QString &connection,
                           const QString &syncMode,
                           const QString &lastError);
    void appendLspLog(const QString &line);
    void clearLspLog();

signals:
    void fontFamilyChanged(const QString &family);
    void fontSizeChanged(int pointSize);
    void wordWrapChanged(bool enabled);
    void themeChanged(const QString &themeName);
    void localeChanged(const QString &locale);
    void lspEnabledChanged(bool enabled);
    void refreshRuntimeRequested();
    void clearRuntimeCacheRequested();

private slots:
    void applyTheme();
    void applyTranslations();

private:
    void initializeShortcutTree();
    void updateShortcutTexts();

    QLabel *m_titleLabel = nullptr;
    QLabel *m_hintLabel = nullptr;
    QLabel *m_runtimeTitleLabel = nullptr;
    QLabel *m_runtimeHintLabel = nullptr;
    QLabel *m_diagTitleLabel = nullptr;
    QLabel *m_logLabel = nullptr;

    QLabel *m_runtimeStatusValue = nullptr;
    QLabel *m_runtimeTagValue = nullptr;
    QLabel *m_runtimeLspPathValue = nullptr;
    QLabel *m_runtimeStdlibPathValue = nullptr;
    QLabel *m_runtimeCachePathValue = nullptr;
    QLabel *m_lspConnectionValue = nullptr;
    QLabel *m_lspSyncModeValue = nullptr;
    QLabel *m_lspLastErrorValue = nullptr;
    QPlainTextEdit *m_lspLogView = nullptr;
    QPushButton *m_refreshRuntimeButton = nullptr;
    QPushButton *m_clearRuntimeCacheButton = nullptr;
    QFontComboBox *m_fontFamilyCombo = nullptr;
    QSpinBox *m_fontSizeSpin = nullptr;
    QCheckBox *m_wordWrapCheck = nullptr;
    QCheckBox *m_lspEnabledCheck = nullptr;
    QComboBox *m_themeCombo = nullptr;
    QComboBox *m_localeCombo = nullptr;
    QTabWidget *m_tabWidget = nullptr;
    QTreeWidget *m_shortcutsTree = nullptr;
};

#endif
