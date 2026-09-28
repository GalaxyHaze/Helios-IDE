#include "EditorCompletionInsertionPolicy.h"

#include <QRegularExpression>

EditorCompletionInsertionPolicy::Decision
EditorCompletionInsertionPolicy::prepare(const QString &line,
                                         int cursorPosition,
                                         const QString &insertText,
                                         int insertTextFormat)
{
    if (cursorPosition < 0 || cursorPosition > line.size())
        return {};

    int start = cursorPosition;
    while (start > 0) {
        const QChar character = line.at(start - 1);
        if (!character.isLetterOrNumber() && character != QLatin1Char('_'))
            break;
        --start;
    }

    return prepareRange(start, cursorPosition, insertText,
                        insertTextFormat);
}

EditorCompletionInsertionPolicy::Decision
EditorCompletionInsertionPolicy::prepareRange(int start, int end,
                                              const QString &insertText,
                                              int insertTextFormat)
{
    if (start < 0 || end < start)
        return {};

    Decision decision;
    decision.start = start;
    decision.end = end;
    decision.text = insertTextFormat == 2 ? expandSnippet(insertText)
                                          : insertText;
    decision.valid = true;
    return decision;
}

QString EditorCompletionInsertionPolicy::expandSnippet(const QString &text)
{
    QString result = text;

    static const QRegularExpression placeholder(
        QStringLiteral(R"(\$\{(\d+):([^}]*)\})"));
    result.replace(placeholder, QStringLiteral(R"(\2)"));

    static const QRegularExpression tabstop(QStringLiteral(R"(\$\d+)"));
    result.replace(tabstop, QString());
    return result;
}
