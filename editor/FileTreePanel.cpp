#include "FileTreePanel.h"
#include <QTreeView>
#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QHeaderView>

FileTreePanel::FileTreePanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_model = new QFileSystemModel(this);
    m_model->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    m_model->setNameFilters({"*.zith", "*.toml", "*.json", "*.md"});
    m_model->setNameFilterDisables(false);

    m_treeView = new QTreeView;
    m_treeView->setModel(m_model);
    m_treeView->setHeaderHidden(true);
    // Filesystem models can insert many rows at once; animation makes each insertion repaint.
    m_treeView->setAnimated(false);
    m_treeView->setUniformRowHeights(true);
    m_treeView->setIndentation(14);
    m_treeView->setRootIsDecorated(true);
    m_treeView->setExpandsOnDoubleClick(true);
    m_treeView->setStyleSheet(
        "QTreeView { background: #11111b; color: #a5adce; border: none; font-size: 12px; }"
        "QTreeView::item { padding: 2px 4px; }"
        "QTreeView::item:hover { background: #363a4f; }"
        "QTreeView::item:selected { background: #363a4f; color: #c6d0f5; }"
        "QTreeView::branch { background: #11111b; }"
    );

    layout->addWidget(m_treeView);

    connect(m_treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex &idx) {
        QString path = m_model->filePath(idx);
        if (QFileInfo(path).isFile())
            emit fileActivated(path);
    });
}

void FileTreePanel::setRootPath(const QString &path)
{
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        m_treeView->setRootIndex({});
        return;
    }
    QModelIndex root = m_model->setRootPath(path);
    m_treeView->setRootIndex(root);
    m_treeView->expand(root);
}

QString FileTreePanel::rootPath() const
{
    return m_model->rootPath();
}
