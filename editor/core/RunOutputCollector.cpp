#include "RunOutputCollector.h"

void RunOutputCollector::buffer(const QString &taskId, const QString &chunk)
{
    m_pending[taskId].append(chunk);
}

void RunOutputCollector::bufferExit(const QString &taskId, int exitCode)
{
    m_pendingExits[taskId].append(exitCode);
}

QStringList RunOutputCollector::takeFor(const QString &taskId)
{
    return m_pending.take(taskId);
}

std::optional<int> RunOutputCollector::takeExitFor(const QString &taskId)
{
    auto it = m_pendingExits.find(taskId);
    if (it == m_pendingExits.end())
        return std::nullopt;
    const int exitCode = it->takeFirst();
    if (it->isEmpty())
        m_pendingExits.erase(it);
    return exitCode;
}

void RunOutputCollector::discardFor(const QString &taskId)
{
    m_pending.remove(taskId);
    m_pendingExits.remove(taskId);
}

void RunOutputCollector::clear()
{
    m_pending.clear();
    m_pendingExits.clear();
}
