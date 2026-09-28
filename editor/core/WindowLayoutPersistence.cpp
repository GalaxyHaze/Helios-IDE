#include "WindowLayoutPersistence.h"

#include "TomlSettingsStore.h"

TomlWindowLayoutPersistence::TomlWindowLayoutPersistence(
    TomlSettingsStore &store)
    : m_store(store)
{
}

QByteArray TomlWindowLayoutPersistence::mainWindowGeometry() const
{
    return m_store.mainWindowGeometry();
}

QByteArray TomlWindowLayoutPersistence::mainWindowState() const
{
    return m_store.mainWindowState();
}

int TomlWindowLayoutPersistence::sidebarWidth() const
{
    return m_store.sidebarWidth();
}

bool TomlWindowLayoutPersistence::sidebarVisible() const
{
    return m_store.sidebarVisible();
}

bool TomlWindowLayoutPersistence::outlineVisible() const
{
    return m_store.outlineVisible();
}

void TomlWindowLayoutPersistence::setMainWindowGeometry(
    const QByteArray &geometry)
{
    m_store.setMainWindowGeometry(geometry);
}

void TomlWindowLayoutPersistence::setMainWindowState(const QByteArray &state)
{
    m_store.setMainWindowState(state);
}

void TomlWindowLayoutPersistence::setSidebarWidth(int width)
{
    m_store.setSidebarWidth(width);
}

void TomlWindowLayoutPersistence::setSidebarVisible(bool visible)
{
    m_store.setSidebarVisible(visible);
}

void TomlWindowLayoutPersistence::setOutlineVisible(bool visible)
{
    m_store.setOutlineVisible(visible);
}
