#ifndef LSPDOCUMENTCOORDINATOR_H
#define LSPDOCUMENTCOORDINATOR_H

#include "LanguageIdentity.h"

class CodeEditor;
class LspClient;

class LspDocumentCoordinator
{
public:
    void setClient(LanguageIdentity::Language language, LspClient *client);
    void setEnabled(LanguageIdentity::Language language, bool enabled);

    LspClient *clientForLanguage(LanguageIdentity::Language language) const;
    LspClient *clientForPath(const QString &path) const;
    bool shouldUseForPath(const QString &path) const;
    bool isEditorLanguage(CodeEditor *editor,
                          LanguageIdentity::Language language) const;

    void openDocument(CodeEditor *editor);
    void saveDocument(CodeEditor *editor);
    void close(CodeEditor *editor);

private:
    struct Binding
    {
        LspClient *client = nullptr;
        bool enabled = false;
    };

    Binding &bindingFor(LanguageIdentity::Language language);
    const Binding &bindingFor(LanguageIdentity::Language language) const;
    void synchronize(CodeEditor *editor, bool openDocument);

    Binding m_zith;
    Binding m_cFamily;
    Binding m_plainText;
};

#endif
