#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QWidget>

class QCheckBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QPlainTextEdit;

class SettingsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget *parent = nullptr);

    void setFontSize(int pointSize);
    void setWordWrapEnabled(bool enabled);
    void setRuntimeInfo(const QString &status,
                        const QString &tag,
                        const QString &lspPath,
                        const QString &stdlibPath,
                        const QString &cachePath);
    // LSP diagnostics area: connection state, sync mode and last error.
    void setLspDiagnostics(const QString &connection,
                           const QString &syncMode,
                           const QString &lastError);
    void appendLspLog(const QString &line);
    void clearLspLog();

signals:
    void fontSizeChanged(int pointSize);
    void wordWrapChanged(bool enabled);
    void refreshRuntimeRequested();
    void clearRuntimeCacheRequested();

private:
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
    QSpinBox *m_fontSizeSpin = nullptr;
    QCheckBox *m_wordWrapCheck = nullptr;
};

#endif
