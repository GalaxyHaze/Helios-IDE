#include "LspDocumentCoordinator.h"

#include "../editor/Code.h"
#include "../editor/LspClient.h"

void LspDocumentCoordinator::setClient(LanguageIdentity::Language language,
                                        LspClient *client)
{
    bindingFor(language).client = client;
}

void LspDocumentCoordinator::setEnabled(LanguageIdentity::Language language,
                                         bool enabled)
{
    bindingFor(language).enabled = enabled;
}

LspClient *LspDocumentCoordinator::clientForLanguage(
    LanguageIdentity::Language language) const
{
    return bindingFor(language).client;
}

LspClient *LspDocumentCoordinator::clientForPath(const QString &path) const
{
    return clientForLanguage(LanguageIdentity::forPath(path));
}

bool LspDocumentCoordinator::shouldUseForPath(const QString &path) const
{
    const LanguageIdentity::Language language =
        LanguageIdentity::forPath(path);
    const Binding &binding = bindingFor(language);
    return binding.enabled && binding.client != nullptr;
}

bool LspDocumentCoordinator::isEditorLanguage(
    CodeEditor *editor,
    LanguageIdentity::Language language) const
{
    return editor && LanguageIdentity::forPath(editor->filePath()) == language;
}

void LspDocumentCoordinator::openDocument(CodeEditor *editor)
{
    synchronize(editor, true);
}

void LspDocumentCoordinator::saveDocument(CodeEditor *editor)
{
    synchronize(editor, false);
}

void LspDocumentCoordinator::synchronize(CodeEditor *editor, bool openDocument)
{
    if (!editor || editor->filePath().isEmpty())
        return;

    const LanguageIdentity::Language language =
        LanguageIdentity::forPath(editor->filePath());
    LspClient *target = clientForLanguage(language);
    LspClient *current = editor->lspClient();

    if (current && current != target && current->isReady() &&
        !editor->fileUri().isEmpty()) {
        current->closeDocument(editor->fileUri());
    }
    if (target != current)
        editor->setLspClient(target);

    if (!shouldUseForPath(editor->filePath()) || !target ||
        !target->isReady()) {
        return;
    }

    if (openDocument) {
        target->openDocument(
            editor->fileUri(),
            LanguageIdentity::lspId(language).toUtf8().constData(),
            editor->toPlainText(),
            editor->documentVersion());
        editor->markLspDocumentSynchronized();
    } else {
        editor->flushPendingLspChanges();
        target->saveDocument(editor->fileUri());
    }
}

void LspDocumentCoordinator::close(CodeEditor *editor)
{
    if (!editor || editor->fileUri().isEmpty())
        return;

    LspClient *current = editor->lspClient();
    if (current && current->isReady())
        current->closeDocument(editor->fileUri());
    editor->detachLspClient();
}

LspDocumentCoordinator::Binding &LspDocumentCoordinator::bindingFor(
    LanguageIdentity::Language language)
{
    switch (language) {
    case LanguageIdentity::Language::Zith:
        return m_zith;
    case LanguageIdentity::Language::CFamily:
        return m_cFamily;
    case LanguageIdentity::Language::PlainText:
        return m_plainText;
    }

    return m_zith;
}

const LspDocumentCoordinator::Binding &
LspDocumentCoordinator::bindingFor(LanguageIdentity::Language language) const
{
    switch (language) {
    case LanguageIdentity::Language::Zith:
        return m_zith;
    case LanguageIdentity::Language::CFamily:
        return m_cFamily;
    case LanguageIdentity::Language::PlainText:
        return m_plainText;
    }

    return m_zith;
}
