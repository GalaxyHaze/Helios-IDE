#include "ClangdLifecycleCoordinator.h"

#include "../editor/LspClient.h"

void ClangdLifecycleCoordinator::setClient(LspClient *client)
{
    m_client = client;
}

void ClangdLifecycleCoordinator::reconcile(
    const Configuration &configuration)
{
    const bool shouldRun =
        configuration.lspEnabled && configuration.cFamilyEnabled &&
        configuration.hasCFamilyDocuments;

    if (!shouldRun) {
        if (m_client != nullptr && m_client->isRunning()) {
            m_client->stop();
        }
        m_activePath.clear();
        m_activeWorkspaceRoot.clear();
        m_error.clear();
        m_state = State::Disabled;
        return;
    }

    if (configuration.serverPath.isEmpty()) {
        if (m_client != nullptr && m_client->isRunning()) {
            m_client->stop();
        }
        m_activePath.clear();
        m_activeWorkspaceRoot.clear();
        m_error = QStringLiteral("clangd was not found in PATH.");
        m_state = State::MissingPath;
        return;
    }

    if (m_client == nullptr) {
        m_activePath.clear();
        m_activeWorkspaceRoot.clear();
        m_error = QStringLiteral("clangd client is unavailable.");
        m_state = State::Error;
        return;
    }

    if (m_client->isRunning() &&
        m_activePath == configuration.serverPath &&
        m_activeWorkspaceRoot == configuration.workspaceRoot) {
        m_state = m_client->isReady() ? State::Ready : State::Starting;
        return;
    }

    if (m_client->isRunning()) {
        m_client->stop();
    }

    m_error.clear();
    if (m_client->start(LspStartOptions{
            configuration.serverPath,
            QString(),
            configuration.workspaceRoot,
            QStringLiteral("clangd")})) {
        m_activePath = configuration.serverPath;
        m_activeWorkspaceRoot = configuration.workspaceRoot;
        m_state = m_client->isReady() ? State::Ready : State::Starting;
    } else {
        m_activePath.clear();
        m_activeWorkspaceRoot.clear();
        m_error = QStringLiteral("Failed to start clangd.");
        m_state = State::Error;
    }
}

void ClangdLifecycleCoordinator::markInitialized()
{
    m_error.clear();
    m_state = State::Ready;
}

void ClangdLifecycleCoordinator::markStopped()
{
    m_state = m_activePath.isEmpty() ? State::Disabled : State::Stopped;
}

void ClangdLifecycleCoordinator::recordError(const QString &message)
{
    m_activePath.clear();
    m_activeWorkspaceRoot.clear();
    m_error = message;
    m_state = State::Error;
}

ClangdLifecycleCoordinator::State ClangdLifecycleCoordinator::state() const
{
    return m_state;
}

const QString &ClangdLifecycleCoordinator::activePath() const
{
    return m_activePath;
}

const QString &ClangdLifecycleCoordinator::activeWorkspaceRoot() const
{
    return m_activeWorkspaceRoot;
}

const QString &ClangdLifecycleCoordinator::error() const
{
    return m_error;
}
