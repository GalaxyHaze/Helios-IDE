#ifndef LSPRESTARTPOLICY_H
#define LSPRESTARTPOLICY_H

#include <QList>

#include <QtGlobal>

class LspRestartPolicy
{
public:
    enum class Action
    {
        Ignore,
        Restart,
        GiveUp
    };

    struct Decision
    {
        Action action = Action::Ignore;
        int attempt = 0;
        int delaySeconds = 0;
    };

    Decision decide(qint64 now, bool expectedStop, bool enabled,
                    bool hasRuntimePath);

private:
    static constexpr qint64 RestartWindowMs = 60LL * 1000LL;
    static constexpr int MaxRestartsInWindow = 3;

    QList<qint64> m_restartTimes;
};

#endif
