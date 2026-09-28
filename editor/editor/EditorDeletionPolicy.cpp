#include "EditorDeletionPolicy.h"

DeletionDecision EditorDeletionPolicy::decide(const QString &textBeforeCursor,
                                              QChar nextCharacter,
                                              bool hasSelection)
{
    if (hasSelection || textBeforeCursor.isEmpty())
        return {};

    if (isMatchingPair(textBeforeCursor.back(), nextCharacter))
        return {DeletionDecision::Action::DeletePair, 2};

    if (textBeforeCursor.trimmed().isEmpty()
        && textBeforeCursor.size() % 4 == 0) {
        return {DeletionDecision::Action::DeleteIndentation, 4};
    }

    return {};
}

bool EditorDeletionPolicy::isMatchingPair(QChar previousCharacter,
                                          QChar nextCharacter)
{
    return (previousCharacter == '(' && nextCharacter == ')')
           || (previousCharacter == '[' && nextCharacter == ']')
           || (previousCharacter == '{' && nextCharacter == '}')
           || (previousCharacter == '"' && nextCharacter == '"')
           || (previousCharacter == '\'' && nextCharacter == '\'')
           || (previousCharacter == '|' && nextCharacter == '|');
}
