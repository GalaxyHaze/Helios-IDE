#include "LanguageServiceFeedbackController.h"

#include "../panels/DiagnosticsPanel.h"
#include "LspEventSource.h"

#include <utility>

LanguageServiceFeedbackController::LanguageServiceFeedbackController(
    DiagnosticsPanel *diagnosticsPanel, Callbacks callbacks, QObject *parent)
    : QObject(parent), m_diagnosticsPanel(diagnosticsPanel),
      m_callbacks(std::move(callbacks))
{
    if (!m_diagnosticsPanel)
        return;

    connect(m_diagnosticsPanel, &DiagnosticsPanel::countsChanged, this,
            [this](int errors, int warnings) {
                if (m_callbacks.updateDiagnosticCounts)
                    m_callbacks.updateDiagnosticCounts(errors, warnings);
            });
}

void LanguageServiceFeedbackController::attach(LspEventSource *events)
{
    if (!events)
        return;

    if (m_diagnosticsPanel) {
        connect(events, &LspEventSource::diagnosticsReceived, m_diagnosticsPanel,
                &DiagnosticsPanel::setDiagnostics);
    }

    connect(events, &LspEventSource::renameResult, this,
            [this](const QString &, int, const QJsonObject &edit) {
                if (m_callbacks.applyWorkspaceEdit)
                    m_callbacks.applyWorkspaceEdit(edit);
            });
    connect(events, &LspEventSource::showMessage, this,
            [this](const QString &message) {
                if (m_callbacks.showStatus)
                    m_callbacks.showStatus(message, 5000);
            });
}
