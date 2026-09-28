#include "LspShutdownEscalation.h"

void LspShutdownEscalation::reset()
{
    m_terminateSent = false;
    m_killSent = false;
}

LspShutdownEscalation::Action
LspShutdownEscalation::nextAction(bool processRunning)
{
    if (!processRunning)
        return Action::None;
    if (!m_terminateSent) {
        m_terminateSent = true;
        return Action::Terminate;
    }
    if (!m_killSent) {
        m_killSent = true;
        return Action::Kill;
    }
    return Action::None;
}
