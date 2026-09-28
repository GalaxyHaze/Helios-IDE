#include "EditorSyntaxController.h"

#include "../editor/CHighlighter.h"
#include "../editor/Code.h"
#include "../editor/Syntax.h"

#include <QSyntaxHighlighter>

EditorSyntaxController::~EditorSyntaxController()
{
    const auto editors = m_highlighters.keys();
    for (CodeEditor *editor : editors)
        remove(editor);
}

void EditorSyntaxController::apply(CodeEditor *editor,
                                   LanguageIdentity::Language language)
{
    if (!editor)
        return;

    QSyntaxHighlighter *current = m_highlighters.value(editor, nullptr);
    if (language == LanguageIdentity::Language::CFamily) {
        if (qobject_cast<CHighlighter *>(current))
            return;
    } else if (qobject_cast<SyntaxHighlighter *>(current) &&
               language != LanguageIdentity::Language::PlainText) {
        return;
    }

    if (current) {
        current->setDocument(nullptr);
        delete current;
    }
    m_highlighters.insert(editor, create(editor, language));
}

void EditorSyntaxController::remove(CodeEditor *editor)
{
    if (!editor)
        return;

    QSyntaxHighlighter *highlighter = m_highlighters.take(editor);
    if (!highlighter)
        return;

    highlighter->setDocument(nullptr);
    delete highlighter;
}

QSyntaxHighlighter *EditorSyntaxController::highlighterFor(
    CodeEditor *editor) const
{
    return m_highlighters.value(editor, nullptr);
}

QSyntaxHighlighter *EditorSyntaxController::create(
    CodeEditor *editor, LanguageIdentity::Language language) const
{
    if (language == LanguageIdentity::Language::CFamily)
        return new CHighlighter(editor->document());
    return new SyntaxHighlighter(editor->document());
}
