#include "WindowLayoutController.h"

#include "WindowLayoutPersistence.h"

#include <QMainWindow>
#include <QSplitter>

#include <numeric>

WindowLayoutController::WindowLayoutController(
    Dependencies dependencies, WindowLayoutPersistence &persistence,
    QObject *parent)
    : QObject(parent),
      m_window(dependencies.window),
      m_splitter(dependencies.splitter),
      m_sidebar(dependencies.sidebar),
      m_outline(dependencies.outline),
      m_persistence(persistence)
{
    if (m_splitter) {
        connect(m_splitter, &QSplitter::splitterMoved, this,
                [this](int position, int) {
                    persistSidebarWidth(position);
                });
    }
}

void WindowLayoutController::restore()
{
    if (m_window) {
        const QByteArray geometry = m_persistence.mainWindowGeometry();
        if (!geometry.isEmpty())
            m_window->restoreGeometry(geometry);

        const QByteArray state = m_persistence.mainWindowState();
        if (!state.isEmpty())
            m_window->restoreState(state);
    }

    applySplitterWidth(m_persistence.sidebarWidth());
    if (m_sidebar)
        m_sidebar->setVisible(m_persistence.sidebarVisible());
    if (m_outline)
        m_outline->setVisible(m_persistence.outlineVisible());
}

void WindowLayoutController::save() const
{
    if (!m_window)
        return;

    m_persistence.setMainWindowGeometry(m_window->saveGeometry());
    m_persistence.setMainWindowState(m_window->saveState());
}

void WindowLayoutController::setSidebarVisible(bool visible)
{
    if (m_sidebar)
        m_sidebar->setVisible(visible);
    m_persistence.setSidebarVisible(visible);
}

void WindowLayoutController::setOutlineVisible(bool visible)
{
    if (m_outline)
        m_outline->setVisible(visible);
    m_persistence.setOutlineVisible(visible);
}

void WindowLayoutController::persistSidebarWidth(int position)
{
    m_persistence.setSidebarWidth(position);
}

void WindowLayoutController::applySplitterWidth(int width)
{
    if (!m_splitter || m_splitter->count() < 2)
        return;

    QList<int> sizes = m_splitter->sizes();
    if (sizes.size() < 2)
        return;

    if (std::accumulate(sizes.cbegin(), sizes.cend(), 0) <= 0)
        sizes = {width, 700, 220};
    else
        sizes[0] = width;

    m_splitter->setSizes(sizes);
}
