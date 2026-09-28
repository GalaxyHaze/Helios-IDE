#ifndef VIMDOCUMENTOPERATIONS_H
#define VIMDOCUMENTOPERATIONS_H

#include "VimOperation.h"

#include <QTextCursor>

class QPlainTextEdit;

class VimDocumentOperations
{
public:
    explicit VimDocumentOperations(QPlainTextEdit *editor);

    void move(QTextCursor::MoveOperation operation, int count);
    void openLine(bool below);
    bool applySelection(const QTextCursor &selection,
                        VimPendingOperation operation);
    void deleteLines(int count);
    void yankLines(int count);
    void changeLines(int count);
    void deleteSelection();
    void yankSelection();
    void deleteCharacters(int count, bool backward);
    void paste(bool after);
    void replaceCharacters(const QString &text, int count);

private:
    QPlainTextEdit *m_editor = nullptr;
};

#endif
