#ifndef EDITORWORKSPACEPRESENTATIONCONTROLLER_H
#define EDITORWORKSPACEPRESENTATIONCONTROLLER_H

#include <QObject>

#include "ShellCommand.h"

class BreadcrumbsBar;
class FindReplaceBar;
class QStackedWidget;
class QTabWidget;
class QWidget;
class WelcomeWidget;

class EditorWorkspacePresentationController : public QObject
{
    Q_OBJECT

public:
    struct Dependencies
    {
        QTabWidget *tabs = nullptr;
        QStackedWidget *centralStack = nullptr;
        WelcomeWidget *welcomeWidget = nullptr;
        QWidget *editorPanel = nullptr;
        BreadcrumbsBar *breadcrumbs = nullptr;
        FindReplaceBar *findReplaceBar = nullptr;
    };

    explicit EditorWorkspacePresentationController(
        Dependencies dependencies, QObject *parent = nullptr);

    void synchronize();
    void showFind();
    void showReplace();
    void findNext();
    void findPrevious();
    bool handleShellCommand(ShellCommand command);

private:
    void prepareFindBar();

    QTabWidget *m_tabs = nullptr;
    QStackedWidget *m_centralStack = nullptr;
    WelcomeWidget *m_welcomeWidget = nullptr;
    QWidget *m_editorPanel = nullptr;
    BreadcrumbsBar *m_breadcrumbs = nullptr;
    FindReplaceBar *m_findReplaceBar = nullptr;
};

#endif
