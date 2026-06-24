#ifndef COMPILERPANEL_H
#define COMPILERPANEL_H

#include <QDockWidget>
#include <QPlainTextEdit>
#include <QProcess>

class CompilerPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit CompilerPanel(QWidget *parent = nullptr);
    ~CompilerPanel();

    void runCommand(const QString &workingDir, const QString &command, const QStringList &args);
    bool isRunning() const;

signals:
    void compileStarted();
    void compileFinished(int exitCode);

private slots:
    void onProcessOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    QPlainTextEdit *m_output;
    QProcess *m_process;
};

#endif
