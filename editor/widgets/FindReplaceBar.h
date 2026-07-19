#ifndef FINDREPLACEBAR_H
#define FINDREPLACEBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QTextDocument>

class CodeEditor;

class FindReplaceBar : public QWidget
{
    Q_OBJECT

public:
    explicit FindReplaceBar(QWidget *parent = nullptr);

    void setEditor(CodeEditor *editor);
    CodeEditor *editor() const { return m_editor; }

    void showFind();
    void showReplace();
    void hide();
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void closed();

public slots:
    void findNext();
    void findPrevious();

private slots:
    void onTextChanged(const QString &text);
    void replace();
    void replaceAll();
    void updateButtons();

private:
    void doFind(bool forward);
    void highlightAllMatches();
    void clearHighlights();
    void updateHeight();
    void updateButtonSize();

    CodeEditor *m_editor = nullptr;
    QLineEdit *m_findInput;
    QLineEdit *m_replaceInput;
    QLabel *m_matchLabel;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_replaceBtn;
    QPushButton *m_replaceAllBtn;
    QPushButton *m_closeBtn;
    QCheckBox *m_caseCheck;
    QWidget *m_replaceRow;
    bool m_finding = false;
    const QChar prevArrowChar = QChar(0x25B2);
};

#endif
