#ifndef WELCOMEWIDGET_H
#define WELCOMEWIDGET_H

#include <QWidget>
#include <QStringList>

class QListWidget;
class QLabel;
class QPushButton;
class QVBoxLayout;

class WelcomeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WelcomeWidget(QWidget *parent = nullptr);

signals:
    void openFolderRequested();
    void newProjectRequested();
    void projectSelected(const QString &path);

public slots:
    void refreshRecentProjects();
    void updateThemeAndLanguage();

private:
    QLabel *m_titleLabel;
    QLabel *m_recentLabel;
    QListWidget *m_recentList;
    QPushButton *m_openBtn;
    QPushButton *m_newBtn;
    QLabel *m_shortcutsTitle;
    QLabel *m_tipsTitle;
    QLabel *m_shortcutTexts;
    QLabel *m_tipText;
};

#endif
