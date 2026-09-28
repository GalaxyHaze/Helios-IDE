#include "ZithRuntimeState.h"

void ZithRuntimeState::activate(const QString &lspPath,
                                const QString &stdlibPath,
                                const QString &tag,
                                const QString &workspaceRoot)
{
    m_lspPath = lspPath;
    m_stdlibPath = stdlibPath;
    m_tag = tag;
    m_workspaceRoot = workspaceRoot;
}

void ZithRuntimeState::clearRuntime()
{
    m_tag.clear();
    m_lspPath.clear();
    m_stdlibPath.clear();
    m_workspaceRoot.clear();
}

bool ZithRuntimeState::matches(const QString &lspPath,
                               const QString &stdlibPath,
                               const QString &workspaceRoot) const
{
    return m_lspPath == lspPath && m_stdlibPath == stdlibPath &&
           m_workspaceRoot == workspaceRoot;
}
