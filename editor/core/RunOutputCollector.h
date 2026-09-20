#ifndef RUNOUTPUTCOLLECTOR_H
#define RUNOUTPUTCOLLECTOR_H

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

#include <optional>

class RunOutputCollector
{
public:
    void buffer(const QString &taskId, const QString &chunk);
    void bufferExit(const QString &taskId, int exitCode);
    QStringList takeFor(const QString &taskId);
    std::optional<int> takeExitFor(const QString &taskId);
    void discardFor(const QString &taskId);
    void clear();

private:
    QHash<QString, QStringList> m_pending;
    QHash<QString, QList<int>> m_pendingExits;
};

#endif
