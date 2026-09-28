#include "LspClientEventSource.h"

#include "../editor/LspClient.h"

LspClientEventSource::LspClientEventSource(LspClient *client, QObject *parent)
    : LspEventSource(parent), m_client(client)
{
    if (!m_client)
        return;

    connect(m_client, &LspClient::initialized, this,
            &LspEventSource::initialized);
    connect(m_client, &LspClient::serverError, this,
            &LspEventSource::serverError);
    connect(m_client, &LspClient::serverStopped, this,
            &LspEventSource::serverStopped);
    connect(m_client, &LspClient::processStopped, this,
            &LspEventSource::processStopped);
    connect(m_client, &LspClient::diagnosticsReceived, this,
            &LspEventSource::diagnosticsReceived);
    connect(m_client, &LspClient::saveAllRequested, this,
            &LspEventSource::saveAllRequested);
    connect(m_client, &LspClient::logMessage, this,
            &LspEventSource::logMessage);
    connect(m_client, &LspClient::showMessage, this,
            &LspEventSource::showMessage);
    connect(m_client, &LspClient::processOutputReceived, this,
            &LspEventSource::processOutputReceived);
    connect(m_client, &LspClient::processExitReceived, this,
            &LspEventSource::processExitReceived);
    connect(m_client, &LspClient::workDoneProgressReceived, this,
            &LspEventSource::workDoneProgressReceived);
    connect(m_client, &LspClient::commandResult, this,
            &LspEventSource::commandResult);
    connect(m_client, &LspClient::renameResult, this,
            &LspEventSource::renameResult);
}
