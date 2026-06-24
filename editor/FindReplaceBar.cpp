#include "FindReplaceBar.h"
#include "Code.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTextDocument>
#include <QRegularExpression>

FindReplaceBar::FindReplaceBar(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(84);
    setStyleSheet(
        "FindReplaceBar { background: #1e1e2e; border-bottom: 1px solid #363a4f; }"
        "QLineEdit { background: #11111b; color: #c6d0f5; border: 1px solid #363a4f; "
        "  border-radius: 4px; padding: 6px 10px; font-size: 13px; }"
        "QLineEdit:focus { border-color: #7287fd; }"
        "QPushButton { background: #363a4f; color: #c6d0f5; border: none; "
        "  border-radius: 4px; padding: 4px 12px; font-size: 12px; }"
        "QPushButton:hover { background: #45475a; }"
        "QPushButton:pressed { background: #209fb5; }"
        "QPushButton:disabled { color: #585b70; }"
        "QCheckBox { color: #a5adce; font-size: 12px; }"
        "QLabel { color: #a5adce; font-size: 12px; }"
    );

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 4, 10, 4);
    mainLayout->setSpacing(4);

    // Find row
    auto *findRow = new QHBoxLayout;
    auto *findLabel = new QLabel(QString::fromUtf8("\xF0\x9F\x94\x8D"));
    findRow->addWidget(findLabel);

    m_findInput = new QLineEdit;
    m_findInput->setPlaceholderText("Find...");
    m_findInput->setClearButtonEnabled(true);
    findRow->addWidget(m_findInput, 1);

    m_prevBtn = new QPushButton(QChar(0x25B2));
    m_prevBtn->setToolTip("Previous (Shift+Enter)");
    m_prevBtn->setFixedWidth(24);
    findRow->addWidget(m_prevBtn);

    m_nextBtn = new QPushButton(QChar(0x25BC));
    m_nextBtn->setToolTip("Next (Enter)");
    m_nextBtn->setFixedWidth(24);
    findRow->addWidget(m_nextBtn);

    m_caseCheck = new QCheckBox("Aa");
    m_caseCheck->setToolTip("Match case");
    findRow->addWidget(m_caseCheck);

    m_matchLabel = new QLabel;
    m_matchLabel->setFixedWidth(50);
    m_matchLabel->setAlignment(Qt::AlignCenter);
    findRow->addWidget(m_matchLabel);

    m_closeBtn = new QPushButton(QChar(0x2716));
    m_closeBtn->setToolTip("Close (Esc)");
    m_closeBtn->setFixedWidth(24);
    findRow->addWidget(m_closeBtn);

    mainLayout->addLayout(findRow);

    // Replace row
    m_replaceRow = new QWidget;
    auto *replaceRow = new QHBoxLayout(m_replaceRow);
    replaceRow->setContentsMargins(0, 0, 0, 0);

    auto *replaceLabel = new QLabel(QChar(0x270F));
    replaceRow->addWidget(replaceLabel);

    m_replaceInput = new QLineEdit;
    m_replaceInput->setPlaceholderText("Replace...");
    m_replaceInput->setClearButtonEnabled(true);
    replaceRow->addWidget(m_replaceInput, 1);

    m_replaceBtn = new QPushButton("Replace");
    m_replaceBtn->setFixedWidth(80);
    replaceRow->addWidget(m_replaceBtn);

    m_replaceAllBtn = new QPushButton("All");
    m_replaceAllBtn->setToolTip("Replace all");
    m_replaceAllBtn->setFixedWidth(40);
    replaceRow->addWidget(m_replaceAllBtn);

    mainLayout->addWidget(m_replaceRow);
    m_replaceRow->hide();

    // Connections
    connect(m_findInput, &QLineEdit::textChanged, this, &FindReplaceBar::onTextChanged);
    connect(m_findInput, &QLineEdit::returnPressed, this, &FindReplaceBar::findNext);
    connect(m_prevBtn, &QPushButton::clicked, this, &FindReplaceBar::findPrevious);
    connect(m_nextBtn, &QPushButton::clicked, this, &FindReplaceBar::findNext);
    connect(m_caseCheck, &QCheckBox::toggled, this, [this]() { onTextChanged(m_findInput->text()); });
    connect(m_closeBtn, &QPushButton::clicked, this, &FindReplaceBar::hide);
    connect(m_replaceBtn, &QPushButton::clicked, this, &FindReplaceBar::replace);
    connect(m_replaceAllBtn, &QPushButton::clicked, this, &FindReplaceBar::replaceAll);

    hide();
}

void FindReplaceBar::setEditor(CodeEditor *editor)
{
    m_editor = editor;
    updateButtons();
}

void FindReplaceBar::showFind()
{
    m_replaceRow->hide();
    setFixedHeight(46);
    show();
    m_findInput->setFocus();
    m_findInput->selectAll();
}

void FindReplaceBar::showReplace()
{
    m_replaceRow->show();
    setFixedHeight(84);
    show();
    m_findInput->setFocus();
    m_findInput->selectAll();
}

void FindReplaceBar::hide()
{
    clearHighlights();
    QWidget::hide();
    if (m_editor)
        m_editor->setFocus();
    emit closed();
}

bool FindReplaceBar::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Escape) {
            hide();
            return true;
        }
        if (key->key() == Qt::Key_Return && key->modifiers() == Qt::ShiftModifier) {
            findPrevious();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void FindReplaceBar::findNext()
{
    doFind(true);
}

void FindReplaceBar::findPrevious()
{
    doFind(false);
}

void FindReplaceBar::onTextChanged(const QString &text)
{
    Q_UNUSED(text);
    doFind(true);
    highlightAllMatches();
}

void FindReplaceBar::doFind(bool forward)
{
    if (!m_editor || m_findInput->text().isEmpty()) {
        m_matchLabel->setText({});
        return;
    }

    m_finding = true;

    QTextDocument::FindFlags flags;
    if (!forward)
        flags |= QTextDocument::FindBackward;
    if (m_caseCheck->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    QTextCursor cursor = m_editor->textCursor();
    QTextCursor found = m_editor->document()->find(m_findInput->text(), cursor, flags);

    if (found.isNull()) {
        // Wrap around
        if (forward) {
            cursor.movePosition(QTextCursor::Start);
            found = m_editor->document()->find(m_findInput->text(), cursor, flags);
        } else {
            cursor.movePosition(QTextCursor::End);
            found = m_editor->document()->find(m_findInput->text(), cursor, flags);
        }
    }

    if (!found.isNull()) {
        m_editor->setTextCursor(found);
        m_editor->ensureCursorVisible();
    }

    m_finding = false;
}

void FindReplaceBar::highlightAllMatches()
{
    clearHighlights();
    if (!m_editor || m_findInput->text().isEmpty()) return;

    QTextDocument::FindFlags flags;
    if (m_caseCheck->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    QList<QTextEdit::ExtraSelection> selections;
    QTextCursor cursor(m_editor->document());
    cursor.movePosition(QTextCursor::Start);

    int count = 0;
    QTextCursor found = m_editor->document()->find(m_findInput->text(), cursor, flags);
    while (!found.isNull()) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = found;
        QColor bg("#209fb5");
        bg.setAlpha(60);
        sel.format.setBackground(bg);
        selections.append(sel);
        count++;
        found = m_editor->document()->find(m_findInput->text(), found, flags);
    }

    m_matchLabel->setText(count > 0 ? QString("1/%1").arg(count) : "0/0");

    if (count > 0)
        m_editor->setFindSelections(selections);
}

void FindReplaceBar::clearHighlights()
{
    if (m_editor)
        m_editor->setFindSelections({});
    m_matchLabel->setText({});
}

void FindReplaceBar::replace()
{
    if (!m_editor) return;
    QTextCursor c = m_editor->textCursor();
    QString sel = c.selectedText();
    QString find = m_findInput->text();

    bool match = m_caseCheck->isChecked()
        ? sel == find
        : sel.compare(find, Qt::CaseInsensitive) == 0;

    if (!sel.isEmpty() && match && c.hasSelection()) {
        c.insertText(m_replaceInput->text());
        m_editor->setTextCursor(c);
    }

    doFind(true);
    highlightAllMatches();
}

void FindReplaceBar::replaceAll()
{
    if (!m_editor || m_findInput->text().isEmpty()) return;

    QTextDocument::FindFlags flags;
    if (m_caseCheck->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    m_editor->setPlainText(m_editor->toPlainText());
    QTextCursor cursor(m_editor->document());
    cursor.movePosition(QTextCursor::Start);

    int replaced = 0;
    QTextCursor found = m_editor->document()->find(m_findInput->text(), cursor, flags);
    while (!found.isNull()) {
        found.insertText(m_replaceInput->text());
        replaced++;
        found = m_editor->document()->find(m_findInput->text(), found, flags);
    }

    if (replaced > 0) {
        m_editor->document()->setModified(true);
        highlightAllMatches();
    }
}

void FindReplaceBar::updateButtons()
{
    bool hasEditor = m_editor != nullptr;
    m_prevBtn->setEnabled(hasEditor);
    m_nextBtn->setEnabled(hasEditor);
    m_replaceBtn->setEnabled(hasEditor);
    m_replaceAllBtn->setEnabled(hasEditor);
}
