#include "LspRestartPolicy.h"

LspRestartPolicy::Decision LspRestartPolicy::decide(
    qint64 now, bool expectedStop, bool enabled, bool hasRuntimePath)
{
    if (expectedStop || !enabled || !hasRuntimePath)
        return {};

    while (!m_restartTimes.isEmpty() &&
           m_restartTimes.first() <= now - RestartWindowMs) {
        m_restartTimes.removeFirst();
    }

    if (m_restartTimes.size() >= MaxRestartsInWindow) {
        return {Action::GiveUp, static_cast<int>(m_restartTimes.size()), 0};
    }

    m_restartTimes.append(now);
    const int attempt = static_cast<int>(m_restartTimes.size());
    const int delaySeconds =
        static_cast<int>(1u << static_cast<unsigned>(attempt - 1));
    return {Action::Restart, attempt, delaySeconds};
}
