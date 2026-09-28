#ifndef EDITORDIAGNOSTICHIGHLIGHTER_H
#define EDITORDIAGNOSTICHIGHLIGHTER_H

#include "LspTypes.h"

#include <QTextEdit>

#include <functional>

class QTextDocument;

class EditorDiagnosticHighlighter
{
public:
    using ColorForSeverity = std::function<QColor(int)>;

    static QList<QTextEdit::ExtraSelection> selections(
        QTextDocument &document, const QList<LspDiagnostic> &diagnostics,
        const ColorForSeverity &colorForSeverity);
};

#endif
