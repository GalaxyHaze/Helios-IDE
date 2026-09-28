#include "LspEditorResultRouter.h"

#include "../editor/Code.h"
#include "../panels/OutlinePanel.h"

#include <QTabWidget>

LspEditorResultRouter::LspEditorResultRouter(QTabWidget *tabWidget,
                                             OutlinePanel *outlinePanel,
                                             QObject *parent)
    : QObject(parent), m_tabWidget(tabWidget), m_outlinePanel(outlinePanel)
{
}

void LspEditorResultRouter::attach(LspClient *client)
{
    if (!client)
        return;

    connect(client, &LspClient::formattingResult, this,
            [this, client](const QString &uri, int version,
                   const QList<QPair<LspRange, QString>> &edits) {
                handleFormatting(client, uri, version, edits);
            });
    connect(client, &LspClient::documentSymbolsResult, this,
            [this, client](const QString &uri, int version,
                   const QJsonArray &symbols) {
                handleDocumentSymbols(client, uri, version, symbols);
            });
    connect(client, &LspClient::semanticTokensResult, this,
            [this, client](const QString &uri, int version,
                   const QList<LspSemanticToken> &tokens) {
                handleSemanticTokens(client, uri, version, tokens);
            });
    connect(client, &LspClient::foldingRangesResult, this,
            [this, client](const QString &uri, int version,
                   const QList<LspFoldingRange> &ranges) {
                handleFoldingRanges(client, uri, version, ranges);
            });
}

CodeEditor *LspEditorResultRouter::currentEditorFor(
    LspClient *client, const QString &uri, int version) const
{
    if (!m_tabWidget)
        return nullptr;

    auto *editor = qobject_cast<CodeEditor *>(m_tabWidget->currentWidget());
    if (!editor || editor->lspClient() != client || editor->fileUri() != uri ||
        editor->documentVersion() != version) {
        return nullptr;
    }
    return editor;
}

void LspEditorResultRouter::handleFormatting(
    LspClient *client, const QString &uri, int version,
    const QList<QPair<LspRange, QString>> &edits)
{
    if (auto *editor = currentEditorFor(client, uri, version))
        editor->applyEdits(edits);
}

void LspEditorResultRouter::handleDocumentSymbols(
    LspClient *client, const QString &uri, int version,
    const QJsonArray &symbols)
{
    if (!m_outlinePanel)
        return;
    if (currentEditorFor(client, uri, version))
        m_outlinePanel->setSymbols(symbols);
}

void LspEditorResultRouter::handleSemanticTokens(
    LspClient *client, const QString &uri, int version,
    const QList<LspSemanticToken> &tokens)
{
    if (auto *editor = currentEditorFor(client, uri, version))
        editor->setSemanticTokens(tokens);
}

void LspEditorResultRouter::handleFoldingRanges(
    LspClient *client, const QString &uri, int version,
    const QList<LspFoldingRange> &ranges)
{
    if (auto *editor = currentEditorFor(client, uri, version))
        editor->setFoldingRanges(ranges);
}
