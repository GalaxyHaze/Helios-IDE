#ifndef EDITORDECORATIONCONTROLLER_H
#define EDITORDECORATIONCONTROLLER_H

#include "LspTypes.h"

#include <QList>
#include <QObject>
#include <QTextEdit>

class EditorAppearanceController;
class QPlainTextEdit;

class EditorDecorationController : public QObject
{
public:
    EditorDecorationController(QPlainTextEdit *editor,
                               EditorAppearanceController *appearance,
                               QObject *parent = nullptr);

    void setDiagnostics(const QList<LspDiagnostic> &diagnostics);
    void setSemanticTokens(const QList<LspSemanticToken> &tokens);
    void clearSemanticTokens();
    void setLspHighlightRanges(const QList<LspRange> &ranges);
    void clearLspHighlights();
    void setFindSelections(
        const QList<QTextEdit::ExtraSelection> &selections);
    void refreshCursorDecorations();
    void refreshAppearance();

private:
    void updateDiagnosticSelections();
    void updateSemanticTokenSelections();
    void applySelections();
    void updateBracketSelections();

    QPlainTextEdit *m_editor = nullptr;
    EditorAppearanceController *m_appearance = nullptr;
    QList<LspDiagnostic> m_diagnostics;
    QList<QTextEdit::ExtraSelection> m_diagnosticSelections;
    QList<LspSemanticToken> m_semanticTokens;
    QList<QTextEdit::ExtraSelection> m_semanticTokenSelections;
    QList<QTextEdit::ExtraSelection> m_bracketSelections;
    QList<QTextEdit::ExtraSelection> m_lspHighlightSelections;
    QList<QTextEdit::ExtraSelection> m_findSelections;
};

#endif
