#ifndef VIMHELPDIALOG_H
#define VIMHELPDIALOG_H

#include <QDialog>

class VimHelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VimHelpDialog(QWidget *parent = nullptr);

private slots:
    void applyTheme();
    void applyTranslations();
};

#endif
