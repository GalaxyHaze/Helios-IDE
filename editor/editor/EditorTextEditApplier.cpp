#include "EditorTextEditApplier.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

#include <algorithm>

int EditorTextEditApplier::offsetForLspPosition(
    const QTextDocument &document, const LspPosition &position)
{
    if (document.blockCount() == 0)
        return 0;

    const int line = qBound(0, position.line, document.blockCount() - 1);
    const QTextBlock block = document.findBlockByNumber(line);
    const int character =
        qBound(0, position.character, static_cast<int>(block.text().size()));
    return block.position() + character;
}

void EditorTextEditApplier::apply(QTextDocument &document,
                                  const QList<TextEdit> &edits)
{
    if (edits.isEmpty())
        return;

    auto sortedEdits = edits;
    std::sort(
        sortedEdits.begin(), sortedEdits.end(),
        [](const TextEdit &left, const TextEdit &right) {
            if (left.first.start.line != right.first.start.line)
                return left.first.start.line > right.first.start.line;
            return left.first.start.character > right.first.start.character;
        });

    QTextCursor cursor(&document);
    cursor.beginEditBlock();
    for (const TextEdit &edit : sortedEdits) {
        const int start =
            offsetForLspPosition(document, edit.first.start);
        const int end = offsetForLspPosition(document, edit.first.end);
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        cursor.insertText(edit.second);
    }
    cursor.endEditBlock();
}
