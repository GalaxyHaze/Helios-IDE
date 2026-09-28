#include "WorkspaceSearchController.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QPointer>
#include <QThreadPool>
#include <QTextStream>

#include <utility>

namespace
{
constexpr int kMaxResults = 200;
constexpr int kMaxResultsPerFile = 20;

class SearchRunnable : public QRunnable
{
public:
    SearchRunnable(QPointer<WorkspaceSearchController> controller,
                   QString rootPath, QString needle,
                   WorkspaceSearch::ScanPolicy policy, qint64 token,
                   std::shared_ptr<std::atomic<qint64>> searchToken)
        : m_controller(controller),
          m_rootPath(std::move(rootPath)),
          m_needle(std::move(needle)),
          m_policy(std::move(policy)),
          m_token(token),
          m_searchToken(std::move(searchToken))
    {
        setAutoDelete(true);
    }

    void run() override
    {
        if (!isCurrent())
            return;

        QVector<SearchResult> results;
        int totalResults = 0;
        QDir root(m_rootPath);
        QDirIterator fileIterator(
            m_rootPath, QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);

        while (fileIterator.hasNext() && isCurrent() &&
               totalResults < kMaxResults) {
            const QString path = fileIterator.next();
            if (!WorkspaceSearch::shouldScanFile(path, m_policy))
                continue;

            QFile file(path);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;

            QTextStream stream(&file);
            int lineNumber = 0;
            int perFileResults = 0;
            const QString relativePath = root.relativeFilePath(path);
            while (!stream.atEnd() && isCurrent() &&
                   totalResults < kMaxResults &&
                   perFileResults < kMaxResultsPerFile) {
                const QString line = stream.readLine();
                ++lineNumber;
                const qsizetype columnIndex =
                    line.indexOf(m_needle, 0, Qt::CaseInsensitive);
                if (columnIndex < 0)
                    continue;

                SearchResult result;
                result.path = path;
                result.line = lineNumber;
                result.column = static_cast<int>(columnIndex);
                result.preview = QStringLiteral("%1:%2  %3")
                                     .arg(relativePath)
                                     .arg(lineNumber)
                                     .arg(line.simplified());
                results.append(result);
                ++totalResults;
                ++perFileResults;

                if (results.size() >= 25)
                    flushResults(results);
            }
        }

        if (!isCurrent())
            return;

        const bool truncated = totalResults >= kMaxResults;
        flushResults(results);
        QMetaObject::invokeMethod(
            m_controller,
            [controller = m_controller, totalResults, truncated,
             searchToken = m_searchToken, token = m_token]() {
                if (controller && searchToken->load() == token)
                    emit controller->searchFinished(totalResults, truncated);
            },
            Qt::QueuedConnection);
    }

private:
    bool isCurrent() const
    {
        return m_controller && m_searchToken &&
               m_searchToken->load() == m_token;
    }

    void flushResults(QVector<SearchResult> &results)
    {
        if (results.isEmpty() || !isCurrent())
            return;

        const QVector<SearchResult> batch = results;
        results.clear();
        QMetaObject::invokeMethod(
            m_controller,
            [controller = m_controller, batch, searchToken = m_searchToken,
             token = m_token]() {
                if (controller && searchToken->load() == token)
                    emit controller->resultsReady(batch);
            },
            Qt::QueuedConnection);
    }

    QPointer<WorkspaceSearchController> m_controller;
    QString m_rootPath;
    QString m_needle;
    WorkspaceSearch::ScanPolicy m_policy;
    qint64 m_token;
    std::shared_ptr<std::atomic<qint64>> m_searchToken;
};

class ReplaceScanRunnable : public QRunnable
{
public:
    ReplaceScanRunnable(
        QPointer<WorkspaceSearchController> controller, QString rootPath,
        QString needle, QString replacement,
        WorkspaceSearch::ScanPolicy policy, qint64 token,
        std::shared_ptr<std::atomic<qint64>> searchToken)
        : m_controller(controller),
          m_rootPath(std::move(rootPath)),
          m_needle(std::move(needle)),
          m_replacement(std::move(replacement)),
          m_policy(std::move(policy)),
          m_token(token),
          m_searchToken(std::move(searchToken))
    {
        setAutoDelete(true);
    }

    void run() override
    {
        if (!isCurrent())
            return;

        QVector<WorkspaceSearch::SearchReplaceTarget> targets;
        QDirIterator fileIterator(
            m_rootPath, QDir::Files | QDir::NoDotAndDotDot,
            QDirIterator::Subdirectories);
        while (fileIterator.hasNext() && isCurrent()) {
            const QString path = fileIterator.next();
            if (!WorkspaceSearch::shouldScanFile(path, m_policy))
                continue;

            QFile file(path);
            if (!file.open(QIODevice::ReadOnly))
                continue;
            const QString text = QString::fromUtf8(file.readAll());
            file.close();

            const auto edits =
                WorkspaceSearch::replaceEdits(text, {m_needle, m_replacement});
            if (!edits.isEmpty())
                targets.append({path, static_cast<int>(edits.size())});
        }

        if (!isCurrent())
            return;

        QMetaObject::invokeMethod(
            m_controller,
            [controller = m_controller, needle = m_needle,
             replacement = m_replacement, targets,
             searchToken = m_searchToken, token = m_token]() {
                if (controller && searchToken->load() == token)
                    emit controller->replaceAllPreviewReady(
                        needle, replacement, targets);
            },
            Qt::QueuedConnection);
    }

private:
    bool isCurrent() const
    {
        return m_controller && m_searchToken &&
               m_searchToken->load() == m_token;
    }

    QPointer<WorkspaceSearchController> m_controller;
    QString m_rootPath;
    QString m_needle;
    QString m_replacement;
    WorkspaceSearch::ScanPolicy m_policy;
    qint64 m_token;
    std::shared_ptr<std::atomic<qint64>> m_searchToken;
};
}

WorkspaceSearchController::WorkspaceSearchController(
    ScanPolicyProvider scanPolicyProvider, QObject *parent)
    : QObject(parent), m_scanPolicyProvider(std::move(scanPolicyProvider))
{
}

void WorkspaceSearchController::setRootPath(const QString &path)
{
    if (m_rootPath == path)
        return;
    cancel();
    m_rootPath = path;
}

QString WorkspaceSearchController::rootPath() const
{
    return m_rootPath;
}

void WorkspaceSearchController::search(const QString &needle)
{
    const qint64 token = ++(*m_searchToken);
    if (m_rootPath.isEmpty() || needle.isEmpty())
        return;

    const WorkspaceSearch::ScanPolicy policy =
        m_scanPolicyProvider ? m_scanPolicyProvider()
                             : WorkspaceSearch::ScanPolicy{};
    QThreadPool::globalInstance()->start(new SearchRunnable(
        this, m_rootPath, needle, policy, token, m_searchToken));
}

void WorkspaceSearchController::previewReplace(const QString &needle,
                                               const QString &replacement)
{
    const qint64 token = ++(*m_searchToken);
    if (m_rootPath.isEmpty() || needle.isEmpty())
        return;

    const WorkspaceSearch::ScanPolicy policy =
        m_scanPolicyProvider ? m_scanPolicyProvider()
                             : WorkspaceSearch::ScanPolicy{};
    QThreadPool::globalInstance()->start(new ReplaceScanRunnable(
        this, m_rootPath, needle, replacement, policy, token, m_searchToken));
}

void WorkspaceSearchController::cancel()
{
    ++(*m_searchToken);
}
