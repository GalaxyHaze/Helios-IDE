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

    void startBuild(const QString &title);
    void appendOutput(const QString &text);
    void appendRawOutput(const QByteArray &bytes);
    void appendDiagnostics(const QList<LspDiagnostic> &diagnostics);
    void clearOutput();
    void stopRunningTask();
    void showBuildResult(bool success, const QString &programUri = {});
    void applyTheme();
    QString outputText() const;

signals:
    void compileStarted();
    void compileFinished(int exitCode);
    void stopRequested();

private:
    QPlainTextEdit *m_output;
};

#endif
