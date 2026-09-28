#ifndef SHELLTRANSLATIONCONTROLLER_H
#define SHELLTRANSLATIONCONTROLLER_H

#include <QObject>

class ActivityBar;
class ShellCommandSurface;

class ShellTranslationController : public QObject
{
    Q_OBJECT

public:
    ShellTranslationController(ActivityBar *activityBar,
                               ShellCommandSurface *commandSurface,
                               QObject *parent = nullptr);

    void apply();

private:
    ActivityBar *m_activityBar = nullptr;
    ShellCommandSurface *m_commandSurface = nullptr;
};

#endif
