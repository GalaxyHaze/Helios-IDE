#include "EditorLanguageFeatureController.h"

#include "Code.h"
#include "EditorLanguageFeedbackPresenter.h"
#include "LspCompletionModel.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QTextCursor>
#include <QTimer>
#include <QToolTip>

EditorLanguageFeatureController::EditorLanguageFeatureController(
    CodeEditor *editor, QObject *parent)
    : QObject(parent), m_editor(editor)
{
    m_feedbackPresenter =
        new EditorLanguageFeedbackPresenter(m_editor, this);

    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setSingleShot(true);
    connect(m_hoverTimer, &QTimer::timeout, this,
            &EditorLanguageFeatureController::requestHover);

    m_documentHighlightTimer = new QTimer(this);
    m_documentHighlightTimer->setSingleShot(true);
    m_documentHighlightTimer->setInterval(150);
    connect(m_documentHighlightTimer, &QTimer::timeout, this,
            &EditorLanguageFeatureController::requestDocumentHighlight);
}

void EditorLanguageFeatureController::setClient(LspClient *client)
{
    if (m_client == client)
        return;

    detachClient();
    m_client = client;
    if (!m_client || !m_editor)
        return;
    m_feedbackPresenter->attach(m_client);
}

void EditorLanguageFeatureController::detachClient()
{
    if (m_feedbackPresenter)
        m_feedbackPresenter->detach();
    m_client = nullptr;
    if (m_hoverTimer)
        m_hoverTimer->stop();
    if (m_documentHighlightTimer)
        m_documentHighlightTimer->stop();
}

bool EditorLanguageFeatureController::isAvailable(Feature feature) const
{
    if (!m_client || !m_client->isReady())
        return false;

    switch (feature) {
    case Feature::Completion:
        return m_client->supports(LspClient::Capability::Completion);
    case Feature::Hover:
        return m_client->supports(LspClient::Capability::Hover);
    case Feature::Definition:
        return m_client->supports(LspClient::Capability::Definition);
    case Feature::Implementation:
        return m_client->supports(LspClient::Capability::Implementation);
    case Feature::Declaration:
        return m_client->supports(LspClient::Capability::Declaration);
    case Feature::References:
        return m_client->supports(LspClient::Capability::References);
    case Feature::DocumentHighlight:
        return m_client->supports(LspClient::Capability::DocumentHighlight);
    case Feature::SignatureHelp:
        return m_client->supports(LspClient::Capability::SignatureHelp);
    case Feature::Formatting:
        return m_client->supports(LspClient::Capability::Formatting);
    case Feature::Rename:
        return m_client->supports(LspClient::Capability::Rename);
    case Feature::CodeActions:
        return m_client->supports(LspClient::Capability::CodeAction);
    }

    return false;
}

bool EditorLanguageFeatureController::handleKeyPress(QKeyEvent *event)
{
    if (!m_editor || !event)
        return false;

    if (((event->modifiers() == (Qt::ControlModifier | Qt::AltModifier) &&
          event->key() == Qt::Key_L) ||
         (event->modifiers() == (Qt::AltModifier | Qt::ShiftModifier) &&
          event->key() == Qt::Key_F)) &&
        isAvailable(Feature::Formatting)) {
        requestFormatting();
        return true;
    }

    if (event->key() == Qt::Key_F12) {
        if (event->modifiers() == Qt::ControlModifier &&
            isAvailable(Feature::Implementation)) {
            requestImplementation();
            return true;
        }
        if (event->modifiers() == Qt::NoModifier &&
            isAvailable(Feature::Definition)) {
            requestDefinition();
            return true;
        }
    }

    if (event->modifiers() == Qt::ControlModifier &&
        event->key() == Qt::Key_Space &&
        isAvailable(Feature::Completion)) {
        triggerCompletion();
        return true;
    }

    if (m_editor->handleCompletionKey(event))
        return true;
    return false;
}

bool EditorLanguageFeatureController::handleMousePress(QMouseEvent *event)
{
    if (!m_editor || !event)
        return false;

    if (m_hoverTimer)
        m_hoverTimer->stop();
    QToolTip::hideText();

    if (event->button() != Qt::LeftButton ||
        event->modifiers() != Qt::ControlModifier ||
        !isAvailable(Feature::Definition)) {
        return false;
    }

    EditorLanguageRequestContext request =
        m_editor->currentLanguageRequest();
    if (!request.isValid())
        return false;

    const QTextCursor cursor = m_editor->cursorForPosition(event->pos());
    request.position = {cursor.blockNumber(), cursor.positionInBlock()};
    m_client->requestDefinition(request.uri, request.version,
                                request.position);
    return true;
}

void EditorLanguageFeatureController::handleMouseMove(QMouseEvent *event)
{
    if (!m_editor || !event || !isAvailable(Feature::Hover))
        return;

    const QTextCursor cursor = m_editor->cursorForPosition(event->pos());
    const int line = cursor.blockNumber();
    const int character = cursor.positionInBlock();
    if (line == m_hoverLine && character == m_hoverCharacter)
        return;

    m_hoverLine = line;
    m_hoverCharacter = character;
    QToolTip::hideText();
    if (m_hoverTimer)
        m_hoverTimer->start(500);
}

void EditorLanguageFeatureController::handleCursorPositionChanged()
{
    if (m_documentHighlightTimer &&
        isAvailable(Feature::DocumentHighlight))
        m_documentHighlightTimer->start();
}

void EditorLanguageFeatureController::triggerCompletion()
{
    if (!m_editor || !isAvailable(Feature::Completion) ||
        !m_editor->prepareCompletion())
        return;

    const EditorLanguageRequestContext request =
        m_editor->currentLanguageRequest();
    if (request.isValid())
        m_client->requestCompletion(request.uri, request.version,
                                    request.position);
}

void EditorLanguageFeatureController::triggerSignatureHelp()
{
    if (!m_editor || !isAvailable(Feature::SignatureHelp))
        return;

    const EditorLanguageRequestContext request =
        m_editor->currentLanguageRequest();
    if (request.isValid())
        m_client->requestSignatureHelp(request.uri, request.version,
                                       request.position);
}

void EditorLanguageFeatureController::requestDefinition()
{
    if (!m_editor || !isAvailable(Feature::Definition))
        return;

    const EditorLanguageRequestContext request =
        m_editor->currentLanguageRequest();
    if (request.isValid())
        m_client->requestDefinition(request.uri, request.version,
                                    request.position);
}

void EditorLanguageFeatureController::requestImplementation()
{
    if (!m_editor || !isAvailable(Feature::Implementation))
        return;

    const EditorLanguageRequestContext request =
        m_editor->currentLanguageRequest();
    if (request.isValid())
        m_client->requestImplementation(request.uri, request.version,
                                        request.position);
}

void EditorLanguageFeatureController::requestDeclaration()
{
    if (!m_editor || !isAvailable(Feature::Declaration))
        return;

    const EditorLanguageRequestContext request =
        m_editor->currentLanguageRequest();
    if (request.isValid())
        m_client->requestDeclaration(request.uri, request.version,
                                     request.position);
}

void EditorLanguageFeatureController::requestReferences()
{
    if (!m_editor || !isAvailable(Feature::References)) {
        return;
    }

    const EditorLanguageRequestContext request =
        m_editor->currentLanguageRequest();
    if (request.isValid())
        m_client->requestReferences(request.uri, request.version,
                                    request.position);
}

void EditorLanguageFeatureController::requestFormatting()
{
    if (!m_editor || !isAvailable(Feature::Formatting)) {
        return;
    }

    const EditorLanguageRequestContext request =
        m_editor->currentLanguageRequest();
    if (request.isValid())
        m_client->requestFormatting(request.uri, request.version);
}

void EditorLanguageFeatureController::requestRename(const QString &newName)
{
    if (!m_editor || !isAvailable(Feature::Rename) ||
        newName.trimmed().isEmpty()) {
        return;
    }

    const EditorLanguageRequestContext request =
        m_editor->currentLanguageRequest();
    if (request.isValid())
        emit m_editor->renameRequested(request.uri, request.version,
                                       request.position, newName);
}

void EditorLanguageFeatureController::requestCodeActions()
{
    if (!m_editor || !isAvailable(Feature::CodeActions)) {
        return;
    }

    const QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection())
        return;

    const EditorLanguageRequestContext request =
        m_editor->currentLanguageRequest();
    if (!request.isValid())
        return;

    emit m_editor->codeActionsRequested(
        request.uri, request.version,
        {LspDocumentSync::positionForOffset(m_editor->toPlainText(),
                                             cursor.selectionStart()),
         LspDocumentSync::positionForOffset(m_editor->toPlainText(),
                                             cursor.selectionEnd())});
}

void EditorLanguageFeatureController::requestHover()
{
    if (!m_editor || !isAvailable(Feature::Hover) || m_hoverLine < 0) {
        return;
    }

    const EditorLanguageRequestContext request =
        m_editor->currentLanguageRequest();
    if (request.isValid()) {
        m_client->requestHover(
            request.uri, request.version, {m_hoverLine, m_hoverCharacter});
    }
}

void EditorLanguageFeatureController::requestDocumentHighlight()
{
    if (!m_editor || !isAvailable(Feature::DocumentHighlight)) {
        return;
    }

    const EditorLanguageRequestContext request =
        m_editor->currentLanguageRequest();
    if (request.isValid())
        m_client->requestDocumentHighlight(request.uri, request.version,
                                           request.position);
}
