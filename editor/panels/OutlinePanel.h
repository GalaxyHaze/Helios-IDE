#ifndef OUTLINEPANEL_H
#define OUTLINEPANEL_H

#include <QWidget>
#include <QJsonArray>

class QTreeWidget;
class QTreeWidgetItem;
class QLabel;

class OutlinePanel : public QWidget
{
    Q_OBJECT
public:
    explicit OutlinePanel(QWidget *parent = nullptr);

    void setSymbols(const QJsonArray &symbols);
    void clear();

signals:
    void symbolSelected(int line, int column);

private slots:
    void handleItemActivated(QTreeWidgetItem *item, int column);
    void updateThemeAndLanguage();

private:
    QTreeWidget *m_treeWidget;
    QLabel *m_titleLabel;

    void populateTree(const QJsonArray &symbols, QTreeWidgetItem *parentItem);
};

#endif
