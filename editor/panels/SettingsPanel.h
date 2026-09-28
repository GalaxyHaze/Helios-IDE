#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include "../core/LspRuntimePresentationState.h"
#include "../core/LspRuntimePresentationModel.h"

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

    void setLspEnabled(bool enabled);

    void setRuntimeInfo(const LspRuntimeInfo &info);
    void setLspDiagnostics(const LspDiagnosticsInfo &info);
    void setCLspInfo(const ClangdInfo &info);
    void appendLspLog(const QString &line);
    void clearLspLog();

signals:
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
    void applyLspPresentation();

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
    LspRuntimePresentationModel m_lspPresentation;
};

#endif
