#include "LspProcessTransport.h"

#include <QProcess>
#include <QTimer>

LspProcessTransport::LspProcessTransport(QObject *parent)
    : QObject(parent)
{
}

LspProcessTransport::~LspProcessTransport()
{
    if (m_process) {
        m_process->disconnect(this);
        if (m_process->state() != QProcess::NotRunning)
            m_process->kill();
    }
}

#ifdef HELIOS_UNIT_TESTING
void LspProcessTransport::waitForFinishedForTesting(int timeoutMs)
{
    if (m_process)
        m_process->waitForFinished(timeoutMs);
}
#endif

bool LspProcessTransport::start(const QString &program)
{
    if (m_process)
        return false;

    m_protocolCodec.reset();
    m_finishEmitted = false;
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);
    connect(m_process, &QProcess::started, this,
            &LspProcessTransport::onProcessStarted);
    connect(m_process, &QProcess::readyReadStandardOutput, this,
            &LspProcessTransport::onReadyRead);
    connect(m_process, &QProcess::readyReadStandardError, this,
            &LspProcessTransport::onReadyReadError);
    connect(m_process, &QProcess::errorOccurred, this,
            [this](QProcess::ProcessError) { onProcessError(); });
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus status) {
                onProcessFinished(exitCode, status == QProcess::CrashExit);
            });
    m_process->start(program, {}, QIODevice::ReadWrite);
    return true;
}

bool LspProcessTransport::writeMessage(const QJsonObject &message)
{
    if (!m_process || m_process->state() != QProcess::Running)
        return false;

    QByteArray frame;
    QString error;
    if (!LspProtocolCodec::encode(message, frame, error)) {
        emit protocolError(error);
        return false;
    }
    return m_process->write(frame) == frame.size();
}

bool LspProcessTransport::isRunning() const
{
    return m_process && m_process->state() != QProcess::NotRunning;
}

void LspProcessTransport::terminate()
{
    if (m_process)
        m_process->terminate();
}

void LspProcessTransport::kill()
{
    if (m_process)
        m_process->kill();
}

void LspProcessTransport::onReadyRead()
{
    if (!m_process)
        return;

    const LspProtocolCodec::DecodeResult decoded =
        m_protocolCodec.consume(m_process->readAllStandardOutput());
    if (decoded.inputTooLarge)
        emit inputTooLarge();
    for (const QString &error : decoded.errors)
        emit decodeError(error);
    for (const QJsonObject &message : decoded.messages)
        emit messageReceived(message);
}

void LspProcessTransport::onReadyReadError()
{
    if (m_process)
        emit stderrChunk(m_process->readAllStandardError());
}

void LspProcessTransport::onProcessStarted()
{
    emit started();
}

void LspProcessTransport::onProcessError()
{
    if (!m_process || m_process->state() != QProcess::NotRunning)
        return;

    emit processError(m_process->errorString());
    QTimer::singleShot(0, this, [this]() {
        if (m_process && m_process->state() == QProcess::NotRunning)
            finish(-1, true);
    });
}

void LspProcessTransport::onProcessFinished(int exitCode, bool crashed)
{
    finish(exitCode, crashed);
}

void LspProcessTransport::finish(int exitCode, bool crashed)
{
    if (!m_process || m_finishEmitted)
        return;

    m_finishEmitted = true;
    QProcess *process = m_process;
    m_process = nullptr;
    process->disconnect(this);
    process->deleteLater();
    emit finished({exitCode, crashed});
}
