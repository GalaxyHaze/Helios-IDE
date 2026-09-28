#ifndef GITPANEL_H
#define GITPANEL_H

#include "../core/GitRepositorySession.h"

#include <QWidget>

class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class GitStatusListPresenter;

class GitPanel : public QWidget
{
    Q_OBJECT

public:
    explicit GitPanel(QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    QString rootPath() const;
    void applyTheme();

public slots:
    void refreshStatus();

signals:
    void fileActivated(const QString &path);

private slots:
    void stageAll();
    void stageSelected();
    void unstageSelected();
    void commitChanges();
    void onItemActivated(QListWidgetItem *item);
    void onStateChanged(const GitRepositoryState &state);
    void onSessionMessage(const QString &message, bool isError);
    void onCommitSucceeded();

private:
    void setBusy(bool busy);
    void renderStatusSnapshot();
    QStringList selectedRelativePaths() const;
    void setSummaryMessage(const QString &message, bool isError = false);

    GitRepositorySession *m_session = nullptr;
    GitStatusListPresenter *m_statusPresenter = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QLabel *m_branchLabel = nullptr;
    QListWidget *m_statusList = nullptr;
    QLineEdit *m_commitInput = nullptr;
    QPushButton *m_stageAllButton = nullptr;
    QPushButton *m_stageSelectionButton = nullptr;
    QPushButton *m_unstageSelectionButton = nullptr;
    QPushButton *m_commitButton = nullptr;
    QPushButton *m_initButton = nullptr;
    QPushButton *m_connectGithubButton = nullptr;
    QPushButton *m_refreshButton = nullptr;
    GitRepositoryState m_repositoryState;
    void initRepository();
    void connectToGithub();
};

#endif
