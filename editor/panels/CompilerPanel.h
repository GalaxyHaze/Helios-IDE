#ifndef COMPILERPANEL_H
#define COMPILERPANEL_H

#include <QPlainTextEdit>
#include <QString>
#include <QWidget>

#include "../editor/LspClient.h"

class CompilerPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CompilerPanel(QWidget *parent = nullptr);
    ~CompilerPanel();

    void startBuild(const QString &title, const QString &progressToken = {});
    void appendOutput(const QString &text);
    void appendRawOutput(const QByteArray &bytes);
    void appendWorkDoneProgress(const QString &token, const QString &kind,
                                const QString &message = {});
    void appendDiagnostics(const QList<LspDiagnostic> &diagnostics);
    void clearOutput();
    void setRunningTask(const QString &taskId);
    void stopRunningTask();
    void showBuildResult(bool success, const QString &programUri = {});
    bool isRunning() const;
    void applyTheme();
    QString runningTaskId() const;
    QString outputText() const;
    void setActiveProgressToken(const QString &token);
    void clearActiveProgress();
    QString activeProgressToken() const;

signals:
    void compileStarted();
    void compileFinished(int exitCode);
    void stopRequested(const QString &taskId);

private:
    QPlainTextEdit *m_output;
    QString m_runningTaskId;
    QString m_activeProgressToken;
};

#endif
