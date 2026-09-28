#ifndef BRACKETMATCHER_H
#define BRACKETMATCHER_H

#include <QString>

struct BracketMatch
{
    int bracketPosition = -1;
    int matchingPosition = -1;

    bool isValid() const
    {
        return bracketPosition >= 0 && matchingPosition >= 0;
    }
};

class BracketMatcher
{
public:
    static BracketMatch find(const QString &text, int cursorPosition);

private:
    static QChar matchingBracket(QChar bracket);
};

#endif
