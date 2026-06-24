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

    void setActiveMode(Mode mode);
    Mode activeMode() const { return m_activeMode; }

signals:
    void modeChanged(ActivityBar::Mode mode);

private:
    QToolButton *createButton(const QIcon &icon, const QString &tooltip);

    QList<QToolButton *> m_buttons;
    Mode m_activeMode = Explorer;
};

#endif
