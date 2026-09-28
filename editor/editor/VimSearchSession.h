#ifndef VIMSEARCHSESSION_H
#define VIMSEARCHSESSION_H

#include <QString>

class QKeyEvent;
class QPlainTextEdit;

class VimSearchSession
{
public:
    explicit VimSearchSession(QPlainTextEdit *editor);

    void begin(bool forward);
    bool handleKeyPress(QKeyEvent *event);
    void repeat(bool forward, int count);
    void reset();

    bool isActive() const { return m_active; }

private:
    void finish();
    void find(bool forward, int count);

    QPlainTextEdit *m_editor = nullptr;
    bool m_active = false;
    QString m_input;
    QString m_lastSearch;
    bool m_lastSearchForward = true;
};

#endif
