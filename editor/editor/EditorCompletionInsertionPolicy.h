#ifndef EDITORCOMPLETIONINSERTIONPOLICY_H
#define EDITORCOMPLETIONINSERTIONPOLICY_H

#include <QString>

class EditorCompletionInsertionPolicy
{
public:
    struct Decision
    {
        int start = 0;
        int end = 0;
        QString text;
        bool valid = false;
    };

    static Decision prepare(const QString &line, int cursorPosition,
                            const QString &insertText, int insertTextFormat);
    static Decision prepareRange(int start, int end,
                                 const QString &insertText,
                                 int insertTextFormat);

private:
    static QString expandSnippet(const QString &text);
};

#endif
