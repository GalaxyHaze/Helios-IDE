#ifndef FILETREEPANEL_H
#define FILETREEPANEL_H

#include <QWidget>
#include <QString>

class QTreeView;
class QFileSystemModel;

class FileTreePanel : public QWidget
{
    Q_OBJECT

public:
    explicit FileTreePanel(QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    QString rootPath() const;

signals:
    void fileActivated(const QString &path);

private:
    QTreeView *m_treeView;
    QFileSystemModel *m_model;
};

#endif
