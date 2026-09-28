#include "LspReferencesRouter.h"

#include "../editor/Code.h"
#include "../panels/ReferencesPanel.h"

#include <QTabWidget>

LspReferencesRouter::LspReferencesRouter(
    Dependencies dependencies,
    std::function<void()> showReferences, QObject *parent)
    : QObject(parent),
      m_tabWidget(dependencies.tabWidget),
      m_referencesPanel(dependencies.referencesPanel),
      m_showReferences(std::move(showReferences))
{
}

void LspReferencesRouter::attach(LspClient *client)
{
    if (!client)
        return;

    connect(client, &LspClient::referencesResult, this,
            [this](const QString &uri, int version,
                   const QList<LspLocation> &locations) {
                handleReferences(uri, version, locations);
            });
}

CodeEditor *LspReferencesRouter::currentEditorFor(const QString &uri,
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

void LspReferencesRouter::handleReferences(
    const QString &uri, int version, const QList<LspLocation> &locations)
{
    if (!m_referencesPanel || !currentEditorFor(uri, version))
        return;

    m_referencesPanel->setReferences(locations);
    if (!locations.isEmpty() && m_showReferences)
        m_showReferences();
}
