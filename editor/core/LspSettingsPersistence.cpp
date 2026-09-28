#include "LspSettingsPersistence.h"

#include "TomlSettingsStore.h"

TomlLspSettingsPersistence::TomlLspSettingsPersistence(
    TomlSettingsStore &store)
    : m_store(store)
{
}

bool TomlLspSettingsPersistence::lspEnabled() const
{
    return m_store.lspEnabled();
}

bool TomlLspSettingsPersistence::useOnlineZithLsp() const
{
    return m_store.useOnlineZithLsp();
}

bool TomlLspSettingsPersistence::cLspEnabled() const
{
    return m_store.cLspEnabled();
}

QString TomlLspSettingsPersistence::cLspPath() const
{
    return m_store.cLspPath();
}

void TomlLspSettingsPersistence::setLspEnabled(bool enabled)
{
    m_store.setLspEnabled(enabled);
}

void TomlLspSettingsPersistence::setUseOnlineZithLsp(bool enabled)
{
    m_store.setUseOnlineZithLsp(enabled);
}

void TomlLspSettingsPersistence::setCLspEnabled(bool enabled)
{
    m_store.setCLspEnabled(enabled);
}

void TomlLspSettingsPersistence::setCLspPath(const QString &path)
{
    m_store.setCLspPath(path);
}
