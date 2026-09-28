#include "LspDocumentRegistry.h"

void LspDocumentRegistry::clear()
{
    m_versions.clear();
}

void LspDocumentRegistry::open(const QString &uri, int version)
{
    m_versions.insert(uri, version);
}

void LspDocumentRegistry::update(const QString &uri, int version)
{
    m_versions.insert(uri, version);
}

void LspDocumentRegistry::close(const QString &uri)
{
    m_versions.remove(uri);
}

int LspDocumentRegistry::version(const QString &uri) const
{
    return m_versions.value(uri, 1);
}

bool LspDocumentRegistry::isCurrent(const QString &uri, int version) const
{
    return version < 0 || m_versions.value(uri, -1) == version;
}
