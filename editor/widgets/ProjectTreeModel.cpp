#include "ProjectTreeModel.h"
#include <QIcon>
#include <QDir>
#include <QThreadPool>
#include <QRunnable>
#include <QMetaObject>
#include <QDebug>

class DirScanRunnable : public QRunnable {
public:
    DirScanRunnable(qint64 scanId, void *nodePtr, const QString &path, ProjectTreeModel *receiver)
        : m_scanId(scanId), m_nodePtr(nodePtr), m_path(path), m_receiver(receiver) {}

    void run() override {
        QDir dir(m_path);
        dir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        dir.setSorting(QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

        QStringList excludes = { ".git", "build", ".cache", "node_modules" };

        QFileInfoList list = dir.entryInfoList();
        QList<QFileInfo> filtered;
        for (const QFileInfo &info : list) {
            if (excludes.contains(info.fileName()))
                continue;
            filtered.append(info);
        }

        // Use type-safe lambda overload to dispatch back to the UI thread safely
        QMetaObject::invokeMethod(m_receiver, [scanId = m_scanId, nodePtr = m_nodePtr, filtered, receiver = m_receiver]() {
            if (receiver) {
                receiver->handleScanFinished(scanId, nodePtr, filtered);
            }
        }, Qt::QueuedConnection);
    }

private:
    qint64 m_scanId;
    void *m_nodePtr;
    QString m_path;
    ProjectTreeModel *m_receiver;
};

ProjectTreeModel::ProjectTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

ProjectTreeModel::~ProjectTreeModel()
{
    clear();
}

void ProjectTreeModel::clear()
{
    m_activeScanId++;
    if (m_rootNode) {
        freeNode(m_rootNode);
        m_rootNode = nullptr;
    }
}

void ProjectTreeModel::freeNode(ProjectTreeNode *node)
{
    if (!node) return;
    for (ProjectTreeNode *child : node->children) {
        freeNode(child);
    }
    delete node;
}

void ProjectTreeModel::setRootPath(const QString &path)
{
    beginResetModel();
    clear();

    m_rootPath = path;
    if (!path.isEmpty() && QFileInfo::exists(path)) {
        m_rootNode = new ProjectTreeNode;
        m_rootNode->name = QFileInfo(path).fileName();
        m_rootNode->path = path;
        m_rootNode->isDir = true;
        m_rootNode->depth = 0;
    }
    endResetModel();
}

QModelIndex ProjectTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column < 0 || column >= 1) return {};

    ProjectTreeNode *parentNode = m_rootNode;
    if (parent.isValid()) {
        parentNode = static_cast<ProjectTreeNode*>(parent.internalPointer());
    }

    if (!parentNode) return {};

    if (parentNode->isDir && !parentNode->isLoaded) {
        const_cast<ProjectTreeModel*>(this)->startScan(parentNode);
    }

    if (row < 0 || row >= parentNode->children.size()) return {};

    return createIndex(row, column, parentNode->children[row]);
}

QModelIndex ProjectTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) return {};

    auto *childNode = static_cast<ProjectTreeNode*>(child.internalPointer());
    if (!childNode) return {};

    ProjectTreeNode *parentNode = childNode->parent;
    if (!parentNode || parentNode == m_rootNode) return {};

    ProjectTreeNode *grandParentNode = parentNode->parent;
    int row = 0;
    if (grandParentNode) {
        row = grandParentNode->children.indexOf(parentNode);
    }

    return createIndex(row, 0, parentNode);
}

int ProjectTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) return 0;

    ProjectTreeNode *parentNode = m_rootNode;
    if (parent.isValid()) {
        parentNode = static_cast<ProjectTreeNode*>(parent.internalPointer());
    }

    if (!parentNode) return 0;

    if (parentNode->isDir && !parentNode->isLoaded) {
        const_cast<ProjectTreeModel*>(this)->startScan(parentNode);
        return 0;
    }

    return parentNode->children.size();
}

int ProjectTreeModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant ProjectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return {};

    auto *node = static_cast<ProjectTreeNode*>(index.internalPointer());
    if (!node) return {};

    if (role == Qt::DisplayRole) {
        return node->name;
    } else if (role == Qt::DecorationRole) {
        if (node->isLimitNode || node->isLoadMoreNode) {
            return {};
        }
        if (node->isDir) {
            return QIcon::fromTheme("folder");
        } else {
            return QIcon::fromTheme("text-x-generic");
        }
    }
    return {};
}

bool ProjectTreeModel::isDir(const QModelIndex &index) const
{
    if (!index.isValid()) return false;
    auto *node = static_cast<ProjectTreeNode*>(index.internalPointer());
    return node ? node->isDir : false;
}

QString ProjectTreeModel::filePath(const QModelIndex &index) const
{
    if (!index.isValid()) return "";
    auto *node = static_cast<ProjectTreeNode*>(index.internalPointer());
    return node ? node->path : "";
}

void ProjectTreeModel::startScan(ProjectTreeNode *node)
{
    if (!node || node->isLoaded) return;

    if (node->depth >= m_maxDepth) {
        node->isLoaded = true;
        auto *limitNode = new ProjectTreeNode;
        limitNode->name = QString("Max depth reached (%1)").arg(m_maxDepth);
        limitNode->path = "";
        limitNode->isDir = false;
        limitNode->isLimitNode = true;
        limitNode->depth = node->depth + 1;
        limitNode->parent = node;
        node->children.append(limitNode);
        return;
    }

    node->isLoaded = true;
    emit loadingStarted(node->path);

    auto *runnable = new DirScanRunnable(m_activeScanId, node, node->path, this);
    QThreadPool::globalInstance()->start(runnable);
}

void ProjectTreeModel::handleScanFinished(qint64 scanId, void *nodePtr, const QList<QFileInfo> &entries)
{
    if (scanId != m_activeScanId) return;

    auto *node = static_cast<ProjectTreeNode*>(nodePtr);
    node->allEntries = entries;
    node->loadedCount = 0;

    emit loadingFinished(node->path);
    loadNextBatch(node);
}

void ProjectTreeModel::loadNextBatch(ProjectTreeNode *node)
{
    if (!node) return;

    int remaining = node->allEntries.size() - node->loadedCount;
    if (remaining <= 0) return;

    int toLoad = qMin(remaining, m_chunkSize);
    QModelIndex parentIdx = indexForPath(node->path);

    int loadMoreIndex = -1;
    for (int i = 0; i < node->children.size(); ++i) {
        if (node->children[i]->isLoadMoreNode) {
            loadMoreIndex = i;
            break;
        }
    }

    if (loadMoreIndex != -1) {
        beginRemoveRows(parentIdx, loadMoreIndex, loadMoreIndex);
        freeNode(node->children[loadMoreIndex]);
        node->children.removeAt(loadMoreIndex);
        endRemoveRows();
    }

    int startRow = node->children.size();
    int endRow = startRow + toLoad - 1;

    beginInsertRows(parentIdx, startRow, endRow + (remaining > m_chunkSize ? 1 : 0));

    for (int i = 0; i < toLoad; ++i) {
        const QFileInfo &info = node->allEntries[node->loadedCount + i];
        auto *child = new ProjectTreeNode;
        child->name = info.fileName();
        child->path = info.absoluteFilePath();
        child->isDir = info.isDir();
        child->depth = node->depth + 1;
        child->parent = node;
        node->children.append(child);
    }

    node->loadedCount += toLoad;

    if (node->allEntries.size() > node->loadedCount) {
        auto *moreNode = new ProjectTreeNode;
        moreNode->name = "Load more...";
        moreNode->path = node->path;
        moreNode->isDir = false;
        moreNode->isLoadMoreNode = true;
        moreNode->depth = node->depth + 1;
        moreNode->parent = node;
        node->children.append(moreNode);
    }

    endInsertRows();
}

void ProjectTreeModel::loadMore(const QModelIndex &loadMoreIndex)
{
    if (!loadMoreIndex.isValid()) return;
    auto *node = static_cast<ProjectTreeNode*>(loadMoreIndex.internalPointer());
    if (!node || !node->isLoadMoreNode) return;

    ProjectTreeNode *parentNode = node->parent;
    if (parentNode) {
        loadNextBatch(parentNode);
    }
}

QModelIndex ProjectTreeModel::indexForPath(const QString &path) const
{
    if (path.isEmpty() || path == m_rootPath) return {};

    ProjectTreeNode *node = findNodeForPath(m_rootNode, path);
    if (!node || node == m_rootNode) return {};

    ProjectTreeNode *parent = node->parent;
    if (!parent) return {};

    int row = parent->children.indexOf(node);
    if (row == -1) return {};

    return createIndex(row, 0, node);
}

ProjectTreeNode *ProjectTreeModel::findNodeForPath(ProjectTreeNode *parent, const QString &path) const
{
    if (!parent) return nullptr;
    if (parent->path == path) return parent;

    for (ProjectTreeNode *child : parent->children) {
        if (path.startsWith(child->path)) {
            if (child->path == path) return child;
            return findNodeForPath(child, path);
        }
    }
    return nullptr;
}

void ProjectTreeModel::refresh(const QModelIndex &index)
{
    ProjectTreeNode *node = m_rootNode;
    if (index.isValid()) {
        node = static_cast<ProjectTreeNode*>(index.internalPointer());
    }

    if (!node || !node->isDir) return;

    QModelIndex idx = indexForPath(node->path);
    beginRemoveRows(idx, 0, node->children.size() - 1);
    for (ProjectTreeNode *child : node->children) {
        freeNode(child);
    }
    node->children.clear();
    node->isLoaded = false;
    endRemoveRows();

    startScan(node);
}
