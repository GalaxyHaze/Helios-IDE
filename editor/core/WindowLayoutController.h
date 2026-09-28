#ifndef WINDOWLAYOUTCONTROLLER_H
#define WINDOWLAYOUTCONTROLLER_H

#include <QObject>

class QMainWindow;
class QSplitter;
class QWidget;
class WindowLayoutPersistence;

class WindowLayoutController : public QObject
{
public:
    struct Dependencies
    {
        QMainWindow *window = nullptr;
        QSplitter *splitter = nullptr;
        QWidget *sidebar = nullptr;
        QWidget *outline = nullptr;
    };

    WindowLayoutController(Dependencies dependencies,
                           WindowLayoutPersistence &persistence,
                           QObject *parent = nullptr);

    void restore();
    void save() const;
    void setSidebarVisible(bool visible);
    void setOutlineVisible(bool visible);

private:
    void persistSidebarWidth(int position);
    void applySplitterWidth(int width);

    QMainWindow *m_window = nullptr;
    QSplitter *m_splitter = nullptr;
    QWidget *m_sidebar = nullptr;
    QWidget *m_outline = nullptr;
    WindowLayoutPersistence &m_persistence;
};

#endif
