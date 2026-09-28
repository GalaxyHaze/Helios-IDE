#ifndef EDITORDELETIONPOLICY_H
#define EDITORDELETIONPOLICY_H

#include <QString>

struct DeletionDecision
{
    enum class Action {
        Default,
        DeletePair,
        DeleteIndentation
    };

    Action action = Action::Default;
    int characterCount = 0;

    bool handled() const { return action != Action::Default; }
};

class EditorDeletionPolicy
{
public:
    static DeletionDecision decide(const QString &textBeforeCursor,
                                   QChar nextCharacter,
                                   bool hasSelection);

private:
    static bool isMatchingPair(QChar previousCharacter, QChar nextCharacter);
};

#endif
