#ifndef EDITORSYNTAXCONTROLLER_H
#define EDITORSYNTAXCONTROLLER_H

#include "LanguageIdentity.h"

#include <QMap>

class CodeEditor;
class QSyntaxHighlighter;

class EditorSyntaxController
{
public:
    ~EditorSyntaxController();

    void apply(CodeEditor *editor, LanguageIdentity::Language language);
    void remove(CodeEditor *editor);
    QSyntaxHighlighter *highlighterFor(CodeEditor *editor) const;

private:
    QSyntaxHighlighter *create(CodeEditor *editor,
                               LanguageIdentity::Language language) const;

    QMap<CodeEditor *, QSyntaxHighlighter *> m_highlighters;
};

#endif
