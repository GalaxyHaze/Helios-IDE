#ifndef APPLICATIONTHEMECONTROLLER_H
#define APPLICATIONTHEMECONTROLLER_H

#include <QObject>

class QSplitter;
class QTabWidget;
class QMenuBar;
class QWidget;
class BreadcrumbsBar;
class StatusBarController;

class ApplicationThemeController : public QObject
{
    Q_OBJECT

public:
    ApplicationThemeController(QWidget *window, QTabWidget *tabWidget,
                               QSplitter *splitter,
                               BreadcrumbsBar *breadcrumbs,
                               QMenuBar *menuBar,
                               StatusBarController *statusBar,
                               QObject *parent = nullptr);

    void apply();

private:
    QWidget *m_window = nullptr;
    QTabWidget *m_tabWidget = nullptr;
    QSplitter *m_splitter = nullptr;
    BreadcrumbsBar *m_breadcrumbs = nullptr;
    QMenuBar *m_menuBar = nullptr;
    StatusBarController *m_statusBar = nullptr;
};

#endif
