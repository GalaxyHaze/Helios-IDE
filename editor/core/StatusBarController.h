#ifndef STATUSBARCONTROLLER_H
#define STATUSBARCONTROLLER_H

#include <QObject>
#include <QString>

class QStatusBar;
class QLabel;

class StatusBarController : public QObject
{
    Q_OBJECT

public:
    explicit StatusBarController(QStatusBar *statusBar, bool vimEnabled,
                                 QObject *parent = nullptr);

    void setContext(int index, int count);
    void setLspStatus(const QString &text, const QString &color = QString());
    void setVimMode(const QString &text);
    void setDiagnostics(int errors, int warnings);
    void setEditorPosition(int line, int column);
    void setLanguage(const QString &language);
    void applyTheme();
    void showMessage(const QString &message, int timeout = 0);

private:
    void applyLabelStyle(QLabel *label, const QString &color,
                         const QString &extra = QString());

    QStatusBar *m_statusBar = nullptr;
    QLabel *m_contextLabel = nullptr;
    QLabel *m_lspLabel = nullptr;
    QLabel *m_vimLabel = nullptr;
    QLabel *m_errorLabel = nullptr;
    QLabel *m_positionLabel = nullptr;
    QLabel *m_indentLabel = nullptr;
    QLabel *m_encodingLabel = nullptr;
    QLabel *m_languageLabel = nullptr;
    QString m_lspLabelColor;
};

#endif
