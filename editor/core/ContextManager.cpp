#include "ContextManager.h"

ContextManager::ContextManager(QObject *parent)
    : QObject(parent)
{
    m_contexts.append(Context{});
}

const Context &ContextManager::currentContext() const
{
    return m_contexts.at(m_current);
}

QString ContextManager::currentRoot() const
{
    return m_contexts.at(m_current).rootPath;
}

void ContextManager::navigateLeft()
{
    if (m_current > 0) {
        m_current--;
        emit contextChanged(m_current, m_contexts[m_current]);
    }
}

void ContextManager::navigateRight()
{
    if (m_current < m_contexts.size() - 1) {
        m_current++;
        emit contextChanged(m_current, m_contexts[m_current]);
    }
}

void ContextManager::setCurrentRoot(const QString &path)
{
    if (m_current < m_contexts.size()) {
        m_contexts[m_current].rootPath = path;
        emit contextChanged(m_current, m_contexts[m_current]);
    }
}

void ContextManager::appendNew(const QString &rootPath)
{
    m_contexts.append(Context{rootPath});
    m_current = m_contexts.size() - 1;
    emit contextChanged(m_current, m_contexts[m_current]);
}

void ContextManager::setContextState(int index, const QStringList &files, int currentTab)
{
    if (index >= 0 && index < m_contexts.size()) {
        m_contexts[index].openFiles = files;
        m_contexts[index].currentTab = currentTab;
    }
}
