#include "EditorTypingPolicy.h"

QString EditorTypingPolicy::indentationForLine(const QString &line)
{
    QString indentation;
    for (const QChar &character : line) {
        if (!character.isSpace())
            break;
        indentation += character;
    }
    if (line.trimmed().endsWith(QLatin1Char('{')))
        indentation += QStringLiteral("    ");
    return indentation;
}

bool EditorTypingPolicy::isAutoCloseCharacter(QChar character)
{
    return character == QLatin1Char('(') || character == QLatin1Char('[') ||
           character == QLatin1Char('{') || character == QLatin1Char('"') ||
           character == QLatin1Char('\'') || character == QLatin1Char('|');
}

QChar EditorTypingPolicy::closingCharacter(QChar character)
{
    switch (character.unicode()) {
    case '(':
        return ')';
    case '[':
        return ']';
    case '{':
        return '}';
    case '"':
        return '"';
    case '\'':
        return '\'';
    case '|':
        return '|';
    default:
        return {};
    }
}

AutoCloseDecision EditorTypingPolicy::autoCloseDecision(
    QChar character, QChar previousCharacter, QChar nextCharacter,
    bool hasSelection)
{
    const QChar closing = closingCharacter(character);
    if (closing.isNull())
        return {};

    if (hasSelection)
        return {AutoCloseDecision::Action::SurroundSelection, closing};

    if (nextCharacter == closing)
        return {AutoCloseDecision::Action::JumpOver, closing};

    if (character == QLatin1Char('"') || character == QLatin1Char('\'')) {
        const bool validPrevious =
            previousCharacter.isNull() ||
            previousCharacter == QLatin1Char(' ') ||
            previousCharacter == QLatin1Char('\t') ||
            previousCharacter == QLatin1Char('(') ||
            previousCharacter == QLatin1Char('[') ||
            previousCharacter == QLatin1Char('{') ||
            previousCharacter == QLatin1Char(',') ||
            previousCharacter == QLatin1Char(';');
        const bool validNext =
            nextCharacter.isNull() || nextCharacter == QLatin1Char(' ') ||
            nextCharacter == QLatin1Char('\t') ||
            nextCharacter == QLatin1Char(')') ||
            nextCharacter == QLatin1Char(']') ||
            nextCharacter == QLatin1Char('}') ||
            nextCharacter == QLatin1Char(',') ||
            nextCharacter == QLatin1Char(';') ||
            nextCharacter == QLatin1Char('\n');
        if (!validPrevious || !validNext)
            return {};
    }

    return {AutoCloseDecision::Action::InsertPair, closing};
}
