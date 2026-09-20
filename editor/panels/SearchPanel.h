#ifndef SEARCHPANEL_H
#define SEARCHPANEL_H

#include <QPointer>
#include <QWidget>
#include <QVector>
#include <QPair>

#include <atomic>
#include <memory>
#include "../editor/LspClient.h"

class QLineEdit;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QTimer;

struct SearchResult
{
    QString path;
    int line = 0;
    int column = 0;
    QString preview;
};

struct SearchReplaceTarget
{
    QString path;
    int matches = 0;
};

Q_DECLARE_METATYPE(SearchReplaceTarget)

class SearchPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SearchPanel(QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    QString rootPath() const { return m_rootPath; }
    static bool shouldScanFile(const QString &path,
                              const QStringList &textExtensions,
                              const QStringList &excludedDirs);
    static bool shouldScanFile(const QString &path);
    static QList<QPair<LspRange, QString>> replaceEdits(
        const QString &text,
        const QString &needle,
        const QString &replacement);
    static QString applyReplaceEdits(
        const QString &text,
        const QList<QPair<LspRange, QString>> &edits);
    static int offsetForPosition(const QString &text, const LspPosition &pos);
    void applyTheme();

signals:
    void fileActivated(const QString &path, int line, int column);
    void replaceAllPreviewReady(const QString &needle,
                                const QString &replacement,
                                const QVector<SearchReplaceTarget> &targets);

public slots:
    void deliverSearchResults(const QVector<SearchResult> &results, qint64 token);
    void onSearchFinished(qint64 token, int totalResults, bool truncated);

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
    QString m_rootPath;
    std::shared_ptr<std::atomic<qint64>> m_searchToken =
        std::make_shared<std::atomic<qint64>>(0);
};

#endif
