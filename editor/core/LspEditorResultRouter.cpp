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
            [this](const QString &uri, int version,
                   const QList<QPair<LspRange, QString>> &edits) {
                handleFormatting(uri, version, edits);
            });
    connect(client, &LspClient::documentSymbolsResult, this,
            [this](const QString &uri, int version,
                   const QJsonArray &symbols) {
                handleDocumentSymbols(uri, version, symbols);
            });
}

CodeEditor *LspEditorResultRouter::currentEditorFor(const QString &uri,
                                                    int version) const
{
    if (!m_tabWidget)
        return nullptr;

    auto *editor = qobject_cast<CodeEditor *>(m_tabWidget->currentWidget());
    if (!editor || editor->fileUri() != uri ||
        editor->documentVersion() != version) {
        return nullptr;
    }
    return editor;
}

void LspEditorResultRouter::handleFormatting(
    const QString &uri, int version,
    const QList<QPair<LspRange, QString>> &edits)
{
    if (auto *editor = currentEditorFor(uri, version))
        editor->applyEdits(edits);
}

void LspEditorResultRouter::handleDocumentSymbols(
    const QString &uri, int version, const QJsonArray &symbols)
{
    if (!m_outlinePanel)
        return;
    if (currentEditorFor(uri, version))
        m_outlinePanel->setSymbols(symbols);
}
