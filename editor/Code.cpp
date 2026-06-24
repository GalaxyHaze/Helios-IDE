#include "Code.h"
#include "SnippetManager.h"
#include "LspCompletionModel.h"
#include <QPainter>
#include <QTextBlock>
#include <QTextCursor>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QApplication>
#include <QUrl>
#include <QFileInfo>
#include <QAbstractItemView>
#include <QHelpEvent>
#include <QWheelEvent>
#include <qnamespace.h>

static const QColor BG        = QColor("#11111b");
static const QColor FG        = QColor("#c6d0f5");
static const QColor CURSOR    = QColor("#e5c890");
static const QColor SEL_BG    = QColor("#209fb5");
static const QColor LINE_BG   = QColor("#363a4f");
static const QColor GUTTER_BG = QColor("#0a0a14");
static const QColor GUTTER_FG = QColor("#40a02b");
static const QColor GUTTER_ACTIVE = QColor("#a6d189");
static const int MIN_FONT_SIZE = 6;
static const int MAX_FONT_SIZE = 48;

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    // Zith Dark theme palette
    QPalette p;
    p.setColor(QPalette::Base, BG);
    p.setColor(QPalette::Text, FG);
    p.setColor(QPalette::Highlight, SEL_BG);
    p.setColor(QPalette::HighlightedText, FG);
    setPalette(p);

    setStyleSheet(QString(
        "QPlainTextEdit { background-color: %1; color: %2; selection-background-color: %3; }"
        "QPlainTextEdit:focus { border: none; }"
    ).arg(BG.name(), FG.name(), SEL_BG.name()));

    setCursorWidth(2);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, [this]() {
        highlightCurrentLine();
        matchBrackets();
    });
    connect(this, &CodeEditor::textChanged, this, &CodeEditor::onTextChanged);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

void CodeEditor::setFilePath(const QString &path)
{
    m_filePath = path;
    m_fileUri = QUrl::fromLocalFile(path).toString();
}

void CodeEditor::setLspClient(LspClient *client)
{
    m_lspClient = client;

    if (m_lspClient) {
        connect(m_lspClient, &LspClient::diagnosticsReceived, this, [this](const QString &uri, const QList<LspDiagnostic> &diags) {
            if (uri == m_fileUri)
                setDiagnostics(diags);
        });

        connect(m_lspClient, &LspClient::hoverResult, this, [this](const LspHoverInfo &info) {
            if (!info.contents.isEmpty())
                QToolTip::showText(QCursor::pos(), info.contents, this);
        });

        connect(m_lspClient, &LspClient::definitionResult, this, [this](const LspLocation &loc) {
            if (!loc.uri.isEmpty())
                emit navigateToLocation(loc.uri, loc.range.start.line, loc.range.start.character);
        });

        connect(m_lspClient, &LspClient::signatureHelpResult, this, [this](const LspSignatureHelp &help) {
            if (!help.parameters.isEmpty()) {
                QString text = help.activeSignature;
                text += "\n\n";
                for (int i = 0; i < help.parameters.size(); i++) {
                    if (i == help.activeParameter)
                        text += "• " + help.parameters[i] + "  ←\n";
                    else
                        text += "• " + help.parameters[i] + "\n";
                }
                QToolTip::showText(QCursor::pos(), text, this);
            }
        });
    }
}

void CodeEditor::setCompleter(LspCompleter *completer)
{
    m_completer = completer;

    if (m_completer) {
        m_completer->setWidget(this);
        connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
                this, &CodeEditor::onCompletionSelected);
    }
}

// ── Diagnostics ──────────────────────────────────────────────────────

void CodeEditor::setDiagnostics(const QList<LspDiagnostic> &diagnostics)
{
    m_diagnostics = diagnostics;
    updateDiagnosticHighlights();
}

void CodeEditor::clearDiagnostics()
{
    m_diagnostics.clear();
    updateDiagnosticHighlights();
}

void CodeEditor::updateDiagnosticHighlights()
{
    m_diagnosticSelections.clear();

    for (const LspDiagnostic &d : m_diagnostics) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = textCursor();
        sel.cursor.movePosition(QTextCursor::Start);

        int startPos = document()->findBlockByNumber(d.range.start.line).position() + d.range.start.character;
        int endPos = document()->findBlockByNumber(d.range.end.line).position() + d.range.end.character;

        sel.cursor.setPosition(startPos);
        sel.cursor.setPosition(endPos, QTextCursor::KeepAnchor);

        QColor color;
        switch (d.severity) {
        case 1: color = QColor("#e64553"); break;
        case 2: color = QColor("#df8e1d"); break;
        default: color = QColor("#04a5e5"); break;
        }

        sel.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        sel.format.setUnderlineColor(color);
        color.setAlpha(40);
        sel.format.setBackground(color);

        m_diagnosticSelections.append(sel);
    }

    updateDiagnosticDisplay();
}

// ── Line Number Area ─────────────────────────────────────────────────

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    return 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), GUTTER_BG);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    int cursorLine = textCursor().blockNumber();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            bool isCurrent = (blockNumber == cursorLine);
            painter.setPen(isCurrent ? GUTTER_ACTIVE : GUTTER_FG);

            QFont f = font();
            f.setBold(isCurrent);
            painter.setFont(f);

            painter.drawText(0, top, lineNumberArea->width() - 8, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }

    // Gutter separator line
    painter.setPen(QColor("#1e1e2e"));
    int w = lineNumberArea->width();
    painter.drawLine(w - 1, event->rect().top(), w - 1, event->rect().bottom());
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    extraSelections.append(m_diagnosticSelections);
    extraSelections.append(m_bracketSelections);
    extraSelections.append(m_findSelections);

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(LINE_BG);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

// ── Bracket Matching ────────────────────────────────────────────────

static QChar bracketMatch(QChar ch)
{
    switch (ch.unicode()) {
    case '(': return ')';
    case ')': return '(';
    case '[': return ']';
    case ']': return '[';
    case '{': return '}';
    case '}': return '{';
    default:  return {};
    }
}

void CodeEditor::matchBrackets()
{
    m_bracketSelections.clear();

    QTextCursor cursor = textCursor();
    int pos = cursor.positionInBlock();
    QString text = cursor.block().text();
    if (text.isEmpty()) return;

    QChar ch;
    int dir = 0;

    if (pos > 0) {
        QChar prev = text.at(pos - 1);
        if (prev == ')' || prev == ']' || prev == '}')
            { ch = prev; dir = -1; }
    }
    if (dir == 0 && pos < text.length()) {
        QChar next = text.at(pos);
        if (next == '(' || next == '[' || next == '{')
            { ch = next; dir = 1; }
    }

    if (ch.isNull()) return;

    QChar target = bracketMatch(ch);
    int depth = 0;
    int matchPos = -1;
    int i = (dir == 1) ? pos + 1 : pos - 2;

    while (i >= 0 && i < text.length()) {
        if (text.at(i) == ch) depth++;
        else if (text.at(i) == target) {
            if (depth == 0) { matchPos = i; break; }
            depth--;
        }
        i += dir;
    }

    if (matchPos < 0) return;

    QTextCharFormat fmt;
    fmt.setBackground(QColor("#ea76cb"));
    fmt.setForeground(QColor("#11111b"));
    fmt.setFontWeight(QFont::Bold);

    auto addSel = [&](int p) {
        QTextEdit::ExtraSelection sel;
        sel.format = fmt;
        sel.cursor = cursor;
        sel.cursor.movePosition(QTextCursor::StartOfBlock);
        sel.cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, p);
        sel.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
        m_bracketSelections.append(sel);
    };

    addSel(dir == 1 ? pos : pos - 1);
    addSel(matchPos);
}

// ── Key Handling ─────────────────────────────────────────────────────

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    // ── Completer popup active ──────────────────────────────────
    if (m_completer && m_completer->popup()->isVisible()) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Tab:
            if (!m_completer->currentCompletion().isEmpty()) {
                onCompletionSelected(m_completer->insertText());
                return;
            }
            break;
        case Qt::Key_Escape:
            m_completer->popup()->hide();
            return;
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            break;
        default:
            break;
        }
    }

    // ── Smart backspace ────────────────────────────────────────
    if (e->key() == Qt::Key_Backspace) {
        QTextCursor cursor = textCursor();

        // Smart delete: if cursor between auto-closed pair, delete both
        if (!cursor.hasSelection()) {
            int pos = cursor.position();
            if (pos > 0 && pos < document()->characterCount()) {
                QChar prev = document()->characterAt(pos - 1);
                QChar next = document()->characterAt(pos);
                if ((prev == '(' && next == ')') ||
                    (prev == '[' && next == ']') ||
                    (prev == '{' && next == '}') ||
                    (prev == '"' && next == '"') ||
                    (prev == '\'' && next == '\'') ||
                    (prev == '|' && next == '|'))
                {
                    cursor.setPosition(pos - 1);
                    cursor.setPosition(pos + 1, QTextCursor::KeepAnchor);
                    cursor.removeSelectedText();
                    return;
                }
            }
        }

        // Smart backspace (delete 4-space tab stop)
        if (!cursor.hasSelection()) {
            int pos = cursor.positionInBlock();
            QString text = cursor.block().text().left(pos);
            if (!text.isEmpty() && text.trimmed().isEmpty() && text.length() % 4 == 0) {
                for (int i = 0; i < 4 && i < text.length(); ++i)
                    cursor.deletePreviousChar();
                return;
            }
        }
        QPlainTextEdit::keyPressEvent(e);
        return;
    }

    // ── Snippet expansion on Tab ───────────────────────────────
    if (e->key() == Qt::Key_Tab && !e->modifiers()) {
        if (tryExpandSnippet())
            return;
    }

    // ── Tab → 4 spaces ─────────────────────────────────────────
    if (e->key() == Qt::Key_Tab) {
        insertPlainText("    ");
        return;
    }

    // ── Auto-indent on Enter ───────────────────────────────────
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        autoIndent();
        return;
    }

    // ── Zoom ────────────────────────────────────────────────────
    if (e->modifiers() == Qt::ControlModifier) {
        if (e->key() == Qt::Key_Equal || e->key() == Qt::Key_Plus) {
            zoomIn();
            return;
        }
        if (e->key() == Qt::Key_Minus) {
            zoomOut();
            return;
        }
        if (e->key() == Qt::Key_0) {
            zoomReset();
            return;
        }
    }

    // ── Auto-dedent on } ───────────────────────────────────────
    if (e->text() == "}") {
        QTextCursor cursor = textCursor();
        QString text = cursor.block().text();
        int pos = cursor.positionInBlock();
        QString before = text.left(pos);
        QString after = text.mid(pos);
        if (!before.isEmpty() && before.trimmed().isEmpty() && after.trimmed().isEmpty() && before.length() >= 4) {
            for (int i = 0; i < 4; ++i)
                cursor.deletePreviousChar();
            setTextCursor(cursor);
        }
        QPlainTextEdit::keyPressEvent(e);
        return;
    }

    // ── Auto-close pairs ────────────────────────────────────────
    if (!e->text().isEmpty() && !(e->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
        QChar ch = e->text().at(0);
        if (isAutoCloseChar(ch) && handleAutoClose(ch))
            return;
    }

    // ── Default handling ────────────────────────────────────────
    QPlainTextEdit::keyPressEvent(e);

    // ── Post-event: trigger completions / signature help ────────
    if (m_completer) {
        QString text = textCursor().block().text();
        int pos = textCursor().positionInBlock();
        if (pos > 0) {
            QChar ch = text.at(pos - 1);
            if (ch == '.' || ch == ':')
                triggerCompletion();
        }
    }

    if (e->key() == Qt::Key_ParenLeft && m_lspClient)
        triggerSignatureHelp();
}

void CodeEditor::autoIndent()
{
    QTextCursor cursor = textCursor();
    QString currentLine = cursor.block().text();
    QString indent;
    for (const QChar &ch : currentLine) {
        if (ch.isSpace())
            indent += ch;
        else
            break;
    }
    QString extra;
    if (currentLine.trimmed().endsWith('{'))
        extra = "    ";
    insertPlainText("\n" + indent + extra);
}

bool CodeEditor::isAutoCloseChar(QChar ch) const
{
    return ch == '(' || ch == '[' || ch == '{' || ch == '"' || ch == '\'' || ch == '|';
}

bool CodeEditor::handleAutoClose(QChar ch)
{
    QTextCursor cursor = textCursor();

    // Surround selection with pair
    if (cursor.hasSelection()) {
        static const QHash<QChar, QChar> pairs = {
            {'(', ')'}, {'[', ']'}, {'{', '}'}, {'"', '"'}, {'\'', '\''}, {'|', '|'}
        };
        QChar close = pairs.value(ch);
        QString selected = cursor.selectedText();
        cursor.insertText(QString(ch) + selected + close);
        cursor.setPosition(cursor.position() - 1);
        setTextCursor(cursor);
        if (ch == '(' && m_lspClient)
            triggerSignatureHelp();
        return true;
    }

    int pos = cursor.position();

    static const QHash<QChar, QChar> pairs = {
        {'(', ')'}, {'[', ']'}, {'{', '}'}, {'"', '"'}, {'\'', '\''}, {'|', '|'}
    };
    QChar close = pairs.value(ch);
    if (close.isNull()) return false;

    // Jump over existing closing pair
    if (document()->characterAt(pos) == close) {
        cursor.movePosition(QTextCursor::Right);
        setTextCursor(cursor);
        return true;
    }

    // For quotes: skip auto-close if inside a word or preceded by a backslash
    if (ch == '"' || ch == '\'') {
        if (pos > 0) {
            QChar prev = document()->characterAt(pos - 1);
            if (prev != ' ' && prev != '\t' && prev != '(' && prev != '[' && prev != '{' && prev != ',' && prev != ';')
                return false;
        }
        QChar after = document()->characterAt(pos);
        if (!after.isNull() && after != ' ' && after != '\t' && after != ')' && after != ']' && after != '}' && after != ',' && after != ';' && after != '\n')
            return false;
    }

    // Insert pair
    insertPlainText(QString(ch) + close);
    cursor = textCursor();
    cursor.movePosition(QTextCursor::Left);
    setTextCursor(cursor);

    if (ch == '(' && m_lspClient)
        triggerSignatureHelp();

    return true;
}

void CodeEditor::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton &&
        e->modifiers() == Qt::ControlModifier &&
        m_lspClient)
    {
        QTextCursor cursor = cursorForPosition(e->pos());
        int line = cursor.blockNumber();
        int character = cursor.positionInBlock();
        m_lspClient->requestDefinition(m_fileUri, {line, character});
        return;
    }

    QPlainTextEdit::mousePressEvent(e);
}

void CodeEditor::mouseMoveEvent(QMouseEvent *e)
{
    if (e->modifiers() == Qt::ControlModifier) {
        viewport()->setCursor(Qt::PointingHandCursor);
    } else {
        viewport()->setCursor(Qt::IBeamCursor);
    }

    QPlainTextEdit::mouseMoveEvent(e);
}

bool CodeEditor::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip && m_lspClient) {
        QHelpEvent *help = static_cast<QHelpEvent *>(e);
        QTextCursor cursor = cursorForPosition(help->pos());
        int line = cursor.blockNumber();
        int character = cursor.positionInBlock();
        m_lspClient->requestHover(m_fileUri, {line, character});
        return true;
    }
    return QPlainTextEdit::event(e);
}

// ── Completion ───────────────────────────────────────────────────────

void CodeEditor::triggerCompletion()
{
    if (!m_completer || !m_lspClient)
        return;

    QTextCursor cursor = textCursor();
    int line = cursor.blockNumber();
    int character = cursor.positionInBlock();

    m_lspClient->requestCompletion(m_fileUri, {line, character});
}

void CodeEditor::onCompletionSelected(const QString &insertText)
{
    replaceCurrentWord(insertText);
}

void CodeEditor::replaceCurrentWord(const QString &insertText)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
    cursor.insertText(insertText);
    setTextCursor(cursor);
}

void CodeEditor::triggerSignatureHelp()
{
    QTextCursor cursor = textCursor();
    int line = cursor.blockNumber();
    int character = cursor.positionInBlock();
    m_lspClient->requestSignatureHelp(m_fileUri, {line, character});
}

void CodeEditor::requestHoverAtCursor() {}

void CodeEditor::setFindSelections(const QList<QTextEdit::ExtraSelection> &selections)
{
    m_findSelections = selections;
    highlightCurrentLine();
}

void CodeEditor::updateDiagnosticDisplay()
{
    highlightCurrentLine();
}

// ── Document Sync ───────────────────────────────────────────────────

void CodeEditor::onTextChanged()
{
    if (!m_lspClient || m_fileUri.isEmpty())
        return;

    m_documentVersion++;
    m_lspClient->changeDocument(m_fileUri, toPlainText(), m_documentVersion);
}

void CodeEditor::goToLine(int line, int character)
{
    QTextBlock block = document()->findBlockByNumber(line);
    if (!block.isValid())
        return;

    QTextCursor cursor(block);
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, character);
    setTextCursor(cursor);
    centerCursor();
    setFocus();
}

// ── Snippet Expansion ──────────────────────────────────────────────

void CodeEditor::setSnippetManager(SnippetManager *manager)
{
    m_snippetManager = manager;
}

bool CodeEditor::tryExpandSnippet()
{
    if (!m_snippetManager)
        return false;

    QTextCursor cursor = textCursor();
    int pos = cursor.positionInBlock();
    QString text = cursor.block().text().left(pos);

    int wordStart = pos;
    while (wordStart > 0 && (text[wordStart-1].isLetterOrNumber() || text[wordStart-1] == '-' || text[wordStart-1] == '_'))
        wordStart--;

    QString prefix = text.mid(wordStart, pos - wordStart);
    if (prefix.isEmpty())
        return false;

    QString body = m_snippetManager->expand(prefix);
    if (body.isEmpty())
        return false;

    cursor.setPosition(cursor.block().position() + wordStart);
    cursor.setPosition(cursor.block().position() + pos, QTextCursor::KeepAnchor);
    cursor.insertText(body);
    setTextCursor(cursor);
    return true;
}

// ── Zoom ────────────────────────────────────────────────────────────

static const int DEFAULT_FONT_SIZE = 12;

void CodeEditor::zoomIn()
{
    QFont f = font();
    int sz = f.pointSize();
    if (sz < MAX_FONT_SIZE)
        f.setPointSize(sz + 1);
    setFont(f);
    updateLineNumberAreaWidth(0);
    emit zoomChanged(double(f.pointSize()) / DEFAULT_FONT_SIZE);
}

void CodeEditor::zoomOut()
{
    QFont f = font();
    int sz = f.pointSize();
    if (sz > MIN_FONT_SIZE)
        f.setPointSize(sz - 1);
    setFont(f);
    updateLineNumberAreaWidth(0);
    emit zoomChanged(double(f.pointSize()) / DEFAULT_FONT_SIZE);
}

void CodeEditor::zoomReset()
{
    QFont f = font();
    f.setPointSize(DEFAULT_FONT_SIZE);
    setFont(f);
    updateLineNumberAreaWidth(0);
    emit zoomChanged(1.0);
}

void CodeEditor::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() == Qt::ControlModifier) {
        int delta = e->angleDelta().y();
        if (delta > 0)
            zoomIn();
        else if (delta < 0)
            zoomOut();
        e->accept();
        return;
    }
    QPlainTextEdit::wheelEvent(e);
}
