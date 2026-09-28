#include "LanguageServiceWorkspaceController.h"

#include "ClangdLifecycleCoordinator.h"
#include "LspDocumentCoordinator.h"
#include "../editor/Code.h"

#include <QTabWidget>

#include <utility>

LanguageServiceWorkspaceController::LanguageServiceWorkspaceController(
    QTabWidget *tabs, LspDocumentCoordinator *documentCoordinator,
    ClangdLifecycleCoordinator *clangdLifecycle, Callbacks callbacks,
    QObject *parent)
    : QObject(parent),
      m_tabs(tabs),
      m_documentCoordinator(documentCoordinator),
      m_clangdLifecycle(clangdLifecycle),
      m_callbacks(std::move(callbacks))
{
}

void LanguageServiceWorkspaceController::refreshRouting()
{
    if (!m_documentCoordinator)
        return;

    applyRouting(resolveConfiguration());
}

void LanguageServiceWorkspaceController::applyRouting(
    const Configuration &configuration)
{
    if (!m_documentCoordinator)
        return;

    m_documentCoordinator->setEnabled(LanguageIdentity::Language::Zith,
                                      configuration.lspEnabled);
    m_documentCoordinator->setEnabled(
        LanguageIdentity::Language::CFamily,
        configuration.lspEnabled && configuration.cFamilyEnabled
            && !configuration.clangdPath.isEmpty());
}

void LanguageServiceWorkspaceController::reconcile()
{
    const Configuration workspaceConfiguration = resolveConfiguration();
    applyRouting(workspaceConfiguration);
    if (!m_clangdLifecycle)
        return;

    bool hasCFamilyDocuments = false;
    if (m_tabs) {
        for (int index = 0; index < m_tabs->count(); ++index) {
            auto *editor =
                qobject_cast<CodeEditor *>(m_tabs->widget(index));
            if (isEditorLanguage(editor, LanguageIdentity::Language::CFamily)) {
                hasCFamilyDocuments = true;
                break;
            }
        }
    }

    ClangdLifecycleCoordinator::Configuration lifecycleConfiguration;
    lifecycleConfiguration.lspEnabled = workspaceConfiguration.lspEnabled;
    lifecycleConfiguration.cFamilyEnabled =
        workspaceConfiguration.cFamilyEnabled;
    lifecycleConfiguration.hasCFamilyDocuments = hasCFamilyDocuments;
    lifecycleConfiguration.serverPath = workspaceConfiguration.clangdPath;
    lifecycleConfiguration.workspaceRoot =
        workspaceConfiguration.workspaceRoot;
    m_clangdLifecycle->reconcile(lifecycleConfiguration);

    if (m_callbacks.presentationChanged)
        m_callbacks.presentationChanged();
}

LanguageServiceWorkspaceController::Configuration
LanguageServiceWorkspaceController::resolveConfiguration() const
{
    return m_callbacks.configuration ? m_callbacks.configuration()
                                     : Configuration{};
}

bool LanguageServiceWorkspaceController::shouldUseForPath(
    const QString &path)
{
    refreshRouting();
    return m_documentCoordinator &&
           m_documentCoordinator->shouldUseForPath(path);
}

LspClient *LanguageServiceWorkspaceController::clientForLanguage(
    LanguageIdentity::Language language) const
{
    return m_documentCoordinator
               ? m_documentCoordinator->clientForLanguage(language)
               : nullptr;
}

LspClient *LanguageServiceWorkspaceController::clientForPath(
    const QString &path) const
{
    return m_documentCoordinator
               ? m_documentCoordinator->clientForPath(path)
               : nullptr;
}

bool LanguageServiceWorkspaceController::isEditorLanguage(
    CodeEditor *editor, LanguageIdentity::Language language) const
{
    return m_documentCoordinator &&
           m_documentCoordinator->isEditorLanguage(editor, language);
}
