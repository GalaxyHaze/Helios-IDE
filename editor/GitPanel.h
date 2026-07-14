#ifndef GITPANEL_H
#define GITPANEL_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;

class GitPanel : public QWidget
{
    Q_OBJECT

public:
    explicit GitPanel(QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    QString rootPath() const { return m_rootPath; }

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

private:
    bool runGit(const QStringList &args, QString *stdOut = nullptr, QString *stdErr = nullptr);
    QStringList selectedRelativePaths() const;
    void setSummaryMessage(const QString &message, bool isError = false);

    QString m_rootPath;
    QLabel *m_summaryLabel = nullptr;
    QLabel *m_branchLabel = nullptr;
    QListWidget *m_statusList = nullptr;
    QLineEdit *m_commitInput = nullptr;
    QPushButton *m_stageAllButton = nullptr;
    QPushButton *m_stageSelectionButton = nullptr;
    QPushButton *m_unstageSelectionButton = nullptr;
    QPushButton *m_commitButton = nullptr;
};

#endif
