#ifndef SEARCHPANEL_H
#define SEARCHPANEL_H

#include <QWidget>

class QLineEdit;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QTimer;

class SearchPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SearchPanel(QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    QString rootPath() const { return m_rootPath; }

signals:
    void fileActivated(const QString &path, int line, int column);

private slots:
    void triggerSearch();
    void onItemActivated(QListWidgetItem *item);

private:
    static bool shouldScanFile(const QString &path);

    QLineEdit *m_queryInput = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QListWidget *m_results = nullptr;
    QTimer *m_searchTimer = nullptr;
    QString m_rootPath;
};

#endif
