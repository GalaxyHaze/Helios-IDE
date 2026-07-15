#ifndef PROJECTTREEMODEL_H
#define PROJECTTREEMODEL_H

#include <QAbstractItemModel>
#include <QFileInfo>
#include <QDir>
#include <QList>
#include <QPointer>

struct ProjectTreeNode {
    QString name;
    QString path;
    bool isDir = false;
    bool isLoaded = false;
    bool isLimitNode = false;
    bool isLoadMoreNode = false;
    int depth = 0;
    ProjectTreeNode *parent = nullptr;
    QList<ProjectTreeNode*> children;

    // pagination and background scan
    QList<QFileInfo> allEntries;
    int loadedCount = 0;
};

class ProjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ProjectTreeModel(QObject *parent = nullptr);
    ~ProjectTreeModel() override;

    void setRootPath(const QString &path);
    QString rootPath() const { return m_rootPath; }

    int maxDepth() const { return m_maxDepth; }
    void setMaxDepth(int depth) { m_maxDepth = qBound(1, depth, 64); }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool isDir(const QModelIndex &index) const;
    QString filePath(const QModelIndex &index) const;
    QModelIndex indexForPath(const QString &path) const;

    void loadMore(const QModelIndex &loadMoreIndex);
    void refresh(const QModelIndex &index = QModelIndex());

signals:
    void loadingStarted(const QString &path);
    void loadingFinished(const QString &path);

public slots:
    void handleScanFinished(qint64 scanId, void *nodePtr, const QList<QFileInfo> &entries);

private:
    ProjectTreeNode *m_rootNode = nullptr;
    QString m_rootPath;
    int m_maxDepth = 12;
    int m_chunkSize = 100;
    qint64 m_activeScanId = 0;

    void clear();
    void startScan(ProjectTreeNode *node);
    void loadNextBatch(ProjectTreeNode *node);
    ProjectTreeNode *findNodeForPath(ProjectTreeNode *parent, const QString &path) const;
    void freeNode(ProjectTreeNode *node);
};

#endif
