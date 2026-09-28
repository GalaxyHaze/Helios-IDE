#include "EditorLanguageFeedbackPresenter.h"

#include "Code.h"
#include "LspClient.h"

#include <QCursor>
#include <QFileInfo>
#include <QMenu>
#include <QRegularExpression>
#include <QToolTip>
#include <QUrl>

EditorLanguageFeedbackPresenter::EditorLanguageFeedbackPresenter(
    CodeEditor *editor, QObject *parent)
    : QObject(parent), m_editor(editor)
{
}

void EditorLanguageFeedbackPresenter::attach(LspClient *client)
{
    if (m_client == client)
        return;

    detach();
    m_client = client;
    if (!m_client || !m_editor)
        return;

    m_diagnosticsConnection = connect(
        m_client, &LspClient::diagnosticsReceived, this,
        [this](const QString &uri, int version,
               const QList<LspDiagnostic> &diagnostics) {
            if (uri == m_editor->fileUri() &&
                (version < 0 || version == m_editor->documentVersion())) {
                m_editor->setDiagnostics(diagnostics);
            }
        });

    m_hoverConnection = connect(
        m_client, &LspClient::hoverResult, this,
        [this](const QString &uri, int,
               const LspHoverInfo &info) {
            if (uri != m_editor->fileUri() || info.contents.isEmpty())
                return;

            QString text = info.contents;
            text.replace(QRegularExpression("```\\w*\\n?"), "");
            text.replace(QRegularExpression("\\n?```"), "");
            QToolTip::showText(QCursor::pos(), text.trimmed(), m_editor);
        });

    const auto navigateToLocations =
        [this](const QString &uri, int version,
               const QList<LspLocation> &locations) {
            if (uri != m_editor->fileUri() ||
                (version >= 0 && version != m_editor->documentVersion()) ||
                locations.isEmpty()) {
                return;
            }

            const auto navigate = [this](const LspLocation &location) {
                if (location.uri.isEmpty())
                    return;

                emit m_editor->navigateToLocation(
                    location.uri, location.range.start.line,
                    location.range.start.character);
            };

            if (locations.size() == 1) {
                navigate(locations.first());
                return;
            }

            auto *menu = new QMenu(m_editor);
            menu->setAttribute(Qt::WA_DeleteOnClose);
            for (const LspLocation &location : locations) {
                if (location.uri.isEmpty())
                    continue;

                const QUrl locationUrl(location.uri);
                const QString path = locationUrl.isLocalFile()
                                         ? locationUrl.toLocalFile()
                                         : location.uri;
                const QString label =
                    QStringLiteral("%1:%2:%3")
                        .arg(QFileInfo(path).fileName().isEmpty()
                                 ? path
                                 : QFileInfo(path).fileName())
                        .arg(location.range.start.line + 1)
                        .arg(location.range.start.character + 1);
                QAction *action = menu->addAction(label);
                connect(action, &QAction::triggered, menu,
                        [navigate, location]() { navigate(location); });
            }

            if (menu->actions().isEmpty()) {
                menu->deleteLater();
                return;
            }
            menu->popup(QCursor::pos());
        };

    m_definitionConnection = connect(
        m_client, &LspClient::definitionResult, this,
        [navigateToLocations](const QString &uri, int version,
                              const QList<LspLocation> &locations) {
            navigateToLocations(uri, version, locations);
        });
    m_implementationConnection = connect(
        m_client, &LspClient::implementationResult, this,
        [navigateToLocations](const QString &uri, int version,
                              const QList<LspLocation> &locations) {
            navigateToLocations(uri, version, locations);
        });
    m_declarationConnection = connect(
        m_client, &LspClient::declarationResult, this,
        [navigateToLocations](const QString &uri, int version,
                              const QList<LspLocation> &locations) {
            navigateToLocations(uri, version, locations);
        });

    m_signatureConnection = connect(
        m_client, &LspClient::signatureHelpResult, this,
        [this](const QString &uri, int,
               const LspSignatureHelp &help) {
            if (uri != m_editor->fileUri() || help.parameters.isEmpty())
                return;

            QString text = help.activeSignature + QStringLiteral("\n\n");
            for (int index = 0; index < help.parameters.size(); ++index) {
                text += QStringLiteral("• ") + help.parameters.at(index);
                if (index == help.activeParameter)
                    text += QStringLiteral("  ←");
                text += QLatin1Char('\n');
            }
            QToolTip::showText(QCursor::pos(), text, m_editor);
        });

    m_highlightsConnection =
        connect(m_client, &LspClient::documentHighlightsResult, this,
                [this](const QString &uri, int version,
                       const QList<LspRange> &ranges) {
                    if (uri != m_editor->fileUri() ||
                        version != m_editor->documentVersion()) {
                        return;
                    }
                    m_editor->setLspHighlightRanges(ranges);
                });
}

void EditorLanguageFeedbackPresenter::detach()
{
    if (m_diagnosticsConnection)
        disconnect(m_diagnosticsConnection);
    if (m_hoverConnection)
        disconnect(m_hoverConnection);
    if (m_definitionConnection)
        disconnect(m_definitionConnection);
    if (m_implementationConnection)
        disconnect(m_implementationConnection);
    if (m_declarationConnection)
        disconnect(m_declarationConnection);
    if (m_signatureConnection)
        disconnect(m_signatureConnection);
    if (m_highlightsConnection)
        disconnect(m_highlightsConnection);

    m_diagnosticsConnection = {};
    m_hoverConnection = {};
    m_definitionConnection = {};
    m_implementationConnection = {};
    m_declarationConnection = {};
    m_signatureConnection = {};
    m_highlightsConnection = {};
    m_client = nullptr;
    if (m_editor)
        m_editor->setLspHighlightRanges({});
    QToolTip::hideText();
}
