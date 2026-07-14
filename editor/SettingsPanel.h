#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QWidget>

class QCheckBox;
class QSpinBox;

class SettingsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget *parent = nullptr);

    void setFontSize(int pointSize);
    void setWordWrapEnabled(bool enabled);

signals:
    void fontSizeChanged(int pointSize);
    void wordWrapChanged(bool enabled);

private:
    QSpinBox *m_fontSizeSpin = nullptr;
    QCheckBox *m_wordWrapCheck = nullptr;
};

#endif
