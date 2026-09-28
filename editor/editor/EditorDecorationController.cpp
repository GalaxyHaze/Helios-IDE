#include "EditorDecorationController.h"

#include "BracketMatcher.h"
#include "EditorAppearanceController.h"
#include "EditorDiagnosticHighlighter.h"
#include "EditorTextEditApplier.h"

#include <QPlainTextEdit>
#include <QTextBlock>

EditorDecorationController::EditorDecorationController(
    QPlainTextEdit *editor, EditorAppearanceController *appearance,
    QObject *parent)
    : QObject(parent), m_editor(editor), m_appearance(appearance)
{
}

void EditorDecorationController::setDiagnostics(
    const QList<LspDiagnostic> &diagnostics)
{
    m_diagnostics = diagnostics;
    updateDiagnosticSelections();
    applySelections();
}

void EditorDecorationController::setSemanticTokens(
    const QList<LspSemanticToken> &tokens)
{
    m_semanticTokens = tokens;
    updateSemanticTokenSelections();
    applySelections();
}

void EditorDecorationController::clearSemanticTokens()
{
    m_semanticTokens.clear();
    m_semanticTokenSelections.clear();
    applySelections();
}

void EditorDecorationController::updateSemanticTokenSelections()
{
    m_semanticTokenSelections.clear();
    if (!m_editor || !m_appearance)
        return;

    const QColor base = m_appearance->appearance().foreground;
    for (const LspSemanticToken &token : m_semanticTokens) {
        const int hue = (token.tokenType * 47) % 360;
        QColor underline = QColor::fromHsv(
            hue, 120, qBound(80, base.value(), 255));
        underline.setAlpha(190);

        QTextEdit::ExtraSelection selection;
        selection.format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        selection.format.setUnderlineColor(underline);
        selection.cursor = m_editor->textCursor();
        selection.cursor.setPosition(
            EditorTextEditApplier::offsetForLspPosition(
                *m_editor->document(), token.range.start));
        selection.cursor.setPosition(
            EditorTextEditApplier::offsetForLspPosition(
                *m_editor->document(), token.range.end),
            QTextCursor::KeepAnchor);
        m_semanticTokenSelections.append(selection);
    }
}

void EditorDecorationController::setLspHighlightRanges(
    const QList<LspRange> &ranges)
{
    m_lspHighlightSelections.clear();
    if (!m_editor || !m_appearance)
        return;

    QTextCharFormat format;
    QColor color = m_appearance->appearance().selection;
    color.setAlpha(85);
    format.setBackground(color);
    for (const LspRange &range : ranges) {
        QTextEdit::ExtraSelection selection;
        selection.format = format;
        selection.cursor = m_editor->textCursor();
        selection.cursor.setPosition(
            EditorTextEditApplier::offsetForLspPosition(
                *m_editor->document(), range.start));
        selection.cursor.setPosition(
            EditorTextEditApplier::offsetForLspPosition(
                *m_editor->document(), range.end),
            QTextCursor::KeepAnchor);
        m_lspHighlightSelections.append(selection);
    }
    applySelections();
}

void EditorDecorationController::clearLspHighlights()
{
    m_lspHighlightSelections.clear();
    applySelections();
}

void EditorDecorationController::setFindSelections(
    const QList<QTextEdit::ExtraSelection> &selections)
{
    m_findSelections = selections;
    applySelections();
}

void EditorDecorationController::refreshCursorDecorations()
{
    updateBracketSelections();
    applySelections();
}

void EditorDecorationController::refreshAppearance()
{
    updateDiagnosticSelections();
    updateSemanticTokenSelections();
    updateBracketSelections();
    applySelections();
}

void EditorDecorationController::updateDiagnosticSelections()
{
    if (!m_editor || !m_appearance)
        return;

    m_diagnosticSelections = EditorDiagnosticHighlighter::selections(
        *m_editor->document(), m_diagnostics, [this](int severity) {
            const EditorAppearance &appearance = m_appearance->appearance();
            switch (severity) {
            case 1:
                return appearance.diagnosticError;
            case 2:
                return appearance.diagnosticWarning;
            case 3:
                return appearance.diagnosticInfo;
            default:
                return appearance.diagnosticUnknown;
            }
        });
}

void EditorDecorationController::applySelections()
{
    if (!m_editor || !m_appearance)
        return;

    QList<QTextEdit::ExtraSelection> selections;
    selections.append(m_diagnosticSelections);
    selections.append(m_semanticTokenSelections);
    selections.append(m_bracketSelections);
    selections.append(m_lspHighlightSelections);
    selections.append(m_findSelections);

    if (!m_editor->isReadOnly()) {
        QTextEdit::ExtraSelection currentLine;
        currentLine.format.setBackground(
            m_appearance->appearance().currentLine);
        currentLine.format.setProperty(QTextFormat::FullWidthSelection, true);
        currentLine.cursor = m_editor->textCursor();
        currentLine.cursor.clearSelection();
        selections.append(currentLine);
    }

    m_editor->setExtraSelections(selections);
}

void EditorDecorationController::updateBracketSelections()
{
    m_bracketSelections.clear();
    if (!m_editor || !m_appearance)
        return;

    const QTextCursor cursor = m_editor->textCursor();
    const BracketMatch match =
        BracketMatcher::find(cursor.block().text(), cursor.positionInBlock());
    if (!match.isValid())
        return;

    QTextCharFormat format;
    format.setBackground(m_appearance->appearance().bracketBackground);
    format.setForeground(m_appearance->appearance().bracketForeground);
    format.setFontWeight(QFont::Bold);

    auto addSelection = [&](int position) {
        QTextEdit::ExtraSelection selection;
        selection.format = format;
        selection.cursor = cursor;
        selection.cursor.movePosition(QTextCursor::StartOfBlock);
        selection.cursor.movePosition(QTextCursor::Right,
                                      QTextCursor::MoveAnchor, position);
        selection.cursor.movePosition(QTextCursor::Right,
                                      QTextCursor::KeepAnchor, 1);
        m_bracketSelections.append(selection);
    };

    addSelection(match.bracketPosition);
    addSelection(match.matchingPosition);
}
