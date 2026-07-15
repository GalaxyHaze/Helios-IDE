#ifndef FILETREEPANEL_H
#define FILETREEPANEL_H

#include <QWidget>
#include <QString>

class QTreeView;
class ProjectTreeModel;

class FileTreePanel : public QWidget
{
    Q_OBJECT

public:
    explicit FileTreePanel(QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    QString rootPath() const;

signals:
    void fileActivated(const QString &path);
    void fileActivatedInNewTab(const QString &path);
    void projectRootChanged(const QString &path);

private slots:
    void showContextMenu(const QPoint &pos);
    void updateThemeAndLanguage();

private:
    QTreeView *m_treeView;
    ProjectTreeModel *m_model;
};

#endif
