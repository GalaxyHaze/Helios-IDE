#ifndef LSPMANAGERDIALOG_H
#define LSPMANAGERDIALOG_H

#include "../core/LspRuntimePresentationState.h"
#include "../core/LspRuntimePresentationModel.h"

#include <QDialog>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

class LspManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LspManagerDialog(QWidget *parent = nullptr);

    void setLspEnabled(bool enabled);
    void setUseOnlineZithLsp(bool enabled);
    void setCLspEnabled(bool enabled);
    void setCLspPath(const QString &path);
    void setCLspInfo(const ClangdInfo &info);
    void setRuntimeInfo(const LspRuntimeInfo &info);
    void setLspDiagnostics(const LspDiagnosticsInfo &info);
    void appendLspLog(const QString &line);
    void clearLspLog();

signals:
    void lspEnabledChanged(bool enabled);
    void useOnlineZithLspChanged(bool enabled);
    void cLspEnabledChanged(bool enabled);
    void cLspPathChanged(const QString &path);
    void refreshRuntimeRequested();
    void clearRuntimeCacheRequested();

private slots:
    void applyTheme();
    void applyTranslations();

private:
    void applyLspPresentation();

    QLabel *m_titleLabel = nullptr;
    QLabel *m_hintLabel = nullptr;
    QLabel *m_runtimeTitleLabel = nullptr;
    QLabel *m_runtimeHintLabel = nullptr;
    QLabel *m_diagTitleLabel = nullptr;
    QLabel *m_logLabel = nullptr;

    QLabel *m_labelStatus = nullptr;
    QLabel *m_labelTag = nullptr;
    QLabel *m_labelLsp = nullptr;
    QLabel *m_labelStdlib = nullptr;
    QLabel *m_labelCache = nullptr;
    QLabel *m_labelConnection = nullptr;
    QLabel *m_labelSyncMode = nullptr;
    QLabel *m_labelLastError = nullptr;

    QLabel *m_runtimeStatusValue = nullptr;
    QLabel *m_runtimeTagValue = nullptr;
    QLabel *m_runtimeLspPathValue = nullptr;
    QLabel *m_runtimeStdlibPathValue = nullptr;
    QLabel *m_runtimeCachePathValue = nullptr;
    QLabel *m_lspConnectionValue = nullptr;
    QLabel *m_lspSyncModeValue = nullptr;
    QLabel *m_lspLastErrorValue = nullptr;
    QLineEdit *m_cLspPathEdit = nullptr;
    QLabel *m_labelCLspStatus = nullptr;
    QLabel *m_labelCLspPath = nullptr;
    QLabel *m_labelCLspMessage = nullptr;
    QLabel *m_cLspStatusValue = nullptr;
    QLabel *m_cLspPathValue = nullptr;
    QLabel *m_cLspMessageValue = nullptr;
    QLabel *m_cLspTitle = nullptr;
    QCheckBox *m_cLspEnabledCheck = nullptr;
    QPlainTextEdit *m_lspLogView = nullptr;
    QPushButton *m_refreshRuntimeButton = nullptr;
    QPushButton *m_clearRuntimeCacheButton = nullptr;
    QPushButton *m_cLspBrowseButton = nullptr;
    QCheckBox *m_lspEnabledCheck = nullptr;
    QCheckBox *m_useOnlineZithLspCheck = nullptr;

    LspRuntimePresentationModel m_lspPresentation;
};

#endif
