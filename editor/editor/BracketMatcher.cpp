#include "BracketMatcher.h"

QChar BracketMatcher::matchingBracket(QChar bracket)
{
    switch (bracket.unicode()) {
    case '(':
        return ')';
    case ')':
        return '(';
    case '[':
        return ']';
    case ']':
        return '[';
    case '{':
        return '}';
    case '}':
        return '{';
    default:
        return {};
    }
}

BracketMatch BracketMatcher::find(const QString &text, int cursorPosition)
{
    if (text.isEmpty())
        return {};

    QChar bracket;
    int direction = 0;
    int bracketPosition = -1;

    if (cursorPosition > 0) {
        const QChar previous = text.at(cursorPosition - 1);
        if (previous == ')' || previous == ']' || previous == '}') {
            bracket = previous;
            direction = -1;
            bracketPosition = cursorPosition - 1;
        }
    }
    if (direction == 0 && cursorPosition < text.length()) {
        const QChar next = text.at(cursorPosition);
        if (next == '(' || next == '[' || next == '{') {
            bracket = next;
            direction = 1;
            bracketPosition = cursorPosition;
        }
    }

    if (direction == 0)
        return {};

    const QChar target = matchingBracket(bracket);
    int depth = 0;
    int index = direction == 1 ? cursorPosition + 1 : cursorPosition - 2;
    while (index >= 0 && index < text.length()) {
        if (text.at(index) == bracket) {
            ++depth;
        } else if (text.at(index) == target) {
            if (depth == 0)
                return {bracketPosition, index};
            --depth;
        }
        index += direction;
    }
    return {};
}
