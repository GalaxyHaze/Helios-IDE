#include "CompilerPanel.h"
#include <QFont>
#include <QVBoxLayout>

CompilerPanel::CompilerPanel(QWidget *parent)
    : QDockWidget("Compiler Output", parent)
{
    setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

    m_output = new QPlainTextEdit;
    m_output->setReadOnly(true);
    m_output->setFont(QFont("monospace", 10));
    m_output->setStyleSheet(
        "QPlainTextEdit { background: #11111b; color: #c6d0f5; border: none; }"
    );

    setWidget(m_output);

    m_process = new QProcess(this);

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &CompilerPanel::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &CompilerPanel::onProcessOutput);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CompilerPanel::onProcessFinished);
}

CompilerPanel::~CompilerPanel()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
}

bool CompilerPanel::isRunning() const
{
    return m_process->state() != QProcess::NotRunning;
}

void CompilerPanel::runCommand(const QString &workingDir, const QString &command, const QStringList &args)
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }

    m_output->clear();
    QString cmdLine = command;
    for (const QString &a : args)
        cmdLine += " " + a;
    m_output->appendPlainText("$ " + cmdLine);
    m_output->appendPlainText(QString());

    m_process->setWorkingDirectory(workingDir);
    m_process->start(command, args);

    emit compileStarted();
}

void CompilerPanel::onProcessOutput()
{
    QByteArray data = m_process->readAllStandardOutput();
    if (!data.isEmpty())
        m_output->appendPlainText(QString::fromUtf8(data));

    data = m_process->readAllStandardError();
    if (!data.isEmpty())
        m_output->appendPlainText(QString::fromUtf8(data));
}

void CompilerPanel::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    QString msg;
    if (status == QProcess::CrashExit) {
        msg = "Process crashed";
    } else if (exitCode == 0) {
        msg = "Completed successfully";
    } else {
        msg = QString("Completed with exit code %1").arg(exitCode);
    }
    m_output->appendPlainText(msg);
    emit compileFinished(exitCode);
}
