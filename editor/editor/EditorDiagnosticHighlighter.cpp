#include "EditorDiagnosticHighlighter.h"

#include <QTextBlock>
#include <QTextDocument>
#include <QTextCursor>

namespace {
int offsetForPosition(QTextDocument &document, const LspPosition &position)
{
    if (document.blockCount() == 0)
        return 0;

    const int line = qBound(0, position.line, document.blockCount() - 1);
    const QTextBlock block = document.findBlockByNumber(line);
    const int maxCharacter = static_cast<int>(block.text().size());
    const int character = qBound(0, position.character, maxCharacter);
    return block.position() + character;
}
} // namespace

QList<QTextEdit::ExtraSelection> EditorDiagnosticHighlighter::selections(
    QTextDocument &document, const QList<LspDiagnostic> &diagnostics,
    const ColorForSeverity &colorForSeverity)
{
    QList<QTextEdit::ExtraSelection> result;

    for (const LspDiagnostic &diagnostic : diagnostics) {
        QTextEdit::ExtraSelection selection;
        selection.cursor = QTextCursor(&document);
        selection.cursor.setPosition(
            offsetForPosition(document, diagnostic.range.start));
        selection.cursor.setPosition(
            offsetForPosition(document, diagnostic.range.end),
            QTextCursor::KeepAnchor);

        const QColor color = colorForSeverity
                                 ? colorForSeverity(diagnostic.severity)
                                 : QColor();
        selection.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        selection.format.setUnderlineColor(color);
        QColor background = color;
        background.setAlpha(40);
        selection.format.setBackground(background);
        result.append(selection);
    }
    return result;
}
