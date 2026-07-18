#ifndef ACTIVITYBAR_H
#define ACTIVITYBAR_H

#include <QDockWidget>
#include <QToolButton>
#include <QList>

class ActivityBar : public QDockWidget
{
    Q_OBJECT

public:
    explicit ActivityBar(QWidget *parent = nullptr);

    enum Mode { Explorer = 0, Search, Git, Settings };
    Q_ENUM(Mode)

    void setActiveMode(Mode mode, bool sidebarVisible = true);
    Mode activeMode() const { return m_activeMode; }
    void setButtonToolTip(Mode mode, const QString &tooltip);

signals:
    void modeChanged(ActivityBar::Mode mode);
    void preferencesRequested();

private:
    QToolButton *createButton(const QIcon &icon, const QString &tooltip, bool checkable = true);

    QList<QToolButton *> m_buttons;
    Mode m_activeMode = Explorer;
};

#endif
