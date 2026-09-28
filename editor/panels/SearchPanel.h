#ifndef SEARCHPANEL_H
#define SEARCHPANEL_H

#include <QWidget>
#include <QVector>
#include "../core/WorkspaceSearchController.h"

class QLineEdit;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QTimer;

class SearchPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SearchPanel(
        WorkspaceSearchController::ScanPolicyProvider scanPolicyProvider,
        QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    QString rootPath() const;
    void applyTheme();

signals:
    void fileActivated(const QString &path, int line, int column);
    void replaceAllPreviewReady(const QString &needle,
                                const QString &replacement,
                                const QVector<WorkspaceSearch::SearchReplaceTarget> &targets);

public slots:
    void deliverSearchResults(const QVector<SearchResult> &results);
    void onSearchFinished(int totalResults, bool truncated);

private slots:
    void triggerSearch();
    void triggerReplaceAll();
    void onItemActivated(QListWidgetItem *item);

private:
    QLabel *m_titleLabel = nullptr;
    QLineEdit *m_queryInput = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QListWidget *m_results = nullptr;
    QTimer *m_searchTimer = nullptr;
    QLineEdit *m_replaceInput = nullptr;
    QPushButton *m_replaceButton = nullptr;
    WorkspaceSearchController *m_searchController = nullptr;
};

#endif
