#ifndef EDITORAPPEARANCECONTROLLER_H
#define EDITORAPPEARANCECONTROLLER_H

#include <QColor>
#include <QObject>

class QPlainTextEdit;

struct EditorAppearance
{
    QColor background;
    QColor foreground;
    QColor selection;
    QColor currentLine;
    QColor lineNumber;
    QColor gutterBackground;
    QColor gutterActive;
    QColor border;
    QColor bracketBackground;
    QColor bracketForeground;
    QColor diagnosticError;
    QColor diagnosticWarning;
    QColor diagnosticInfo;
    QColor diagnosticUnknown;
};

class EditorAppearanceController : public QObject
{
    Q_OBJECT

public:
    explicit EditorAppearanceController(QPlainTextEdit *editor,
                                        QObject *parent = nullptr);

    const EditorAppearance &appearance() const { return m_appearance; }
    void apply();

signals:
    void appearanceChanged();

private:
    QPlainTextEdit *m_editor = nullptr;
    EditorAppearance m_appearance;
};

#endif
