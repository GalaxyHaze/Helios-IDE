#ifndef WORKSPACESEARCHCONTROLLER_H
#define WORKSPACESEARCHCONTROLLER_H

#include "WorkspaceSearch.h"

#include <QObject>
#include <QString>
#include <QVector>

#include <atomic>
#include <functional>
#include <memory>

struct SearchResult
{
    QString path;
    int line = 0;
    int column = 0;
    QString preview;
};

class WorkspaceSearchController : public QObject
{
    Q_OBJECT

public:
    using ScanPolicyProvider = std::function<WorkspaceSearch::ScanPolicy()>;

    explicit WorkspaceSearchController(
        ScanPolicyProvider scanPolicyProvider = {},
        QObject *parent = nullptr);

    void setRootPath(const QString &path);
    QString rootPath() const;
    void search(const QString &needle);
    void previewReplace(const QString &needle, const QString &replacement);
    void cancel();

signals:
    void resultsReady(const QVector<SearchResult> &results);
    void searchFinished(int totalResults, bool truncated);
    void replaceAllPreviewReady(
        const QString &needle, const QString &replacement,
        const QVector<WorkspaceSearch::SearchReplaceTarget> &targets);

private:
    QString m_rootPath;
    ScanPolicyProvider m_scanPolicyProvider;
    std::shared_ptr<std::atomic<qint64>> m_searchToken =
        std::make_shared<std::atomic<qint64>>(0);
};

#endif
