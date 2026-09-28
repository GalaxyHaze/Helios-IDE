#ifndef LSPSHUTDOWNESCALATION_H
#define LSPSHUTDOWNESCALATION_H

class LspShutdownEscalation
{
public:
    enum class Action
    {
        None,
        Terminate,
        Kill
    };

    void reset();
    Action nextAction(bool processRunning);

private:
    bool m_terminateSent = false;
    bool m_killSent = false;
};

#endif
