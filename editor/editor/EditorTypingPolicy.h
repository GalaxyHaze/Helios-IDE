#ifndef EDITORTYPINGPOLICY_H
#define EDITORTYPINGPOLICY_H

#include <QString>

struct AutoCloseDecision
{
    enum class Action
    {
        Ignore,
        JumpOver,
        InsertPair,
        SurroundSelection
    };

    Action action = Action::Ignore;
    QChar closing;

    bool handled() const { return action != Action::Ignore; }
};

class EditorTypingPolicy
{
public:
    static QString indentationForLine(const QString &line);
    static bool isAutoCloseCharacter(QChar character);
    static AutoCloseDecision autoCloseDecision(QChar character,
                                               QChar previousCharacter,
                                               QChar nextCharacter,
                                               bool hasSelection);

private:
    static QChar closingCharacter(QChar character);
};

#endif
