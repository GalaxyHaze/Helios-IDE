#ifndef LANGUAGESERVICEFEEDBACKCONTROLLER_H
#define LANGUAGESERVICEFEEDBACKCONTROLLER_H

#include <QJsonObject>
#include <QObject>
#include <functional>

class DiagnosticsPanel;
class LspEventSource;

class LanguageServiceFeedbackController : public QObject
{
    Q_OBJECT

public:
    struct Callbacks {
        std::function<void(const QJsonObject &)> applyWorkspaceEdit;
        std::function<void(const QString &, int)> showStatus;
        std::function<void(int, int)> updateDiagnosticCounts;
    };

    explicit LanguageServiceFeedbackController(
        DiagnosticsPanel *diagnosticsPanel, Callbacks callbacks,
        QObject *parent = nullptr);

    void attach(LspEventSource *events);

private:
    DiagnosticsPanel *m_diagnosticsPanel = nullptr;
    Callbacks m_callbacks;
};

#endif
