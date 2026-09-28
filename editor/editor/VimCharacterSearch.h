#ifndef VIMCHARACTERSEARCH_H
#define VIMCHARACTERSEARCH_H

#include <QChar>

class QPlainTextEdit;

struct VimCharacterSearchRequest
{
    QChar character;
    bool forward = true;
    bool till = false;
    int count = 1;
};

class VimCharacterSearch
{
public:
    explicit VimCharacterSearch(QPlainTextEdit *editor);

    bool find(const VimCharacterSearchRequest &request);
    bool repeatLast(bool reverseDirection, int count);
    void reset();

private:
    QPlainTextEdit *m_editor = nullptr;
    QChar m_lastCharacter;
    bool m_lastForward = true;
    bool m_lastTill = false;
};

#endif
