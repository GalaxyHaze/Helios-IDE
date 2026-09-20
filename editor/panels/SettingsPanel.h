#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QWidget>

class QCheckBox;
class QLabel;
class QPushButton;
class QPlainTextEdit;
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
    void setCLspInfo(const QString &status,
                     const QString &path,
                     const QString &message);
    void appendLspLog(const QString &line);
    void clearLspLog();

signals:
    void fontFamilyChanged(const QString &family);
    void fontSizeChanged(int pointSize);
    void wordWrapChanged(bool enabled);
    void themeChanged(const QString &themeName);
    void localeChanged(const QString &locale);
    void openPreferencesRequested();
    void openShortcutsRequested();
    void openLspManagerRequested();
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
    QWidget *m_preferencesCard = nullptr;
    QLabel *m_preferencesTitleLabel = nullptr;
    QLabel *m_hintLabel = nullptr;
    QPushButton *m_openPreferencesButton = nullptr;
    QPushButton *m_openShortcutsButton = nullptr;
    QPushButton *m_openLspManagerButton = nullptr;
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
    QLabel *m_cLspStatusValue = nullptr;
    QLabel *m_cLspPathValue = nullptr;
    QLabel *m_cLspMessageValue = nullptr;
    QLabel *m_cLspTitleLabel = nullptr;
    QPlainTextEdit *m_lspLogView = nullptr;
    QPushButton *m_refreshRuntimeButton = nullptr;
    QPushButton *m_clearRuntimeCacheButton = nullptr;
    QCheckBox *m_lspEnabledCheck = nullptr;
    QTabWidget *m_tabWidget = nullptr;
    QTreeWidget *m_shortcutsTree = nullptr;
};

#endif
