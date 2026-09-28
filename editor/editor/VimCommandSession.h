#ifndef VIMCOMMANDSESSION_H
#define VIMCOMMANDSESSION_H

#include <QString>

#include <optional>

class QKeyEvent;

class VimCommandSession
{
public:
    void begin();
    bool handleKeyPress(QKeyEvent *event);
    void reset();

    bool isActive() const { return m_active; }
    std::optional<QString> takeSubmittedCommand();

private:
    bool m_active = false;
    QString m_input;
    std::optional<QString> m_submittedCommand;
};

#endif
