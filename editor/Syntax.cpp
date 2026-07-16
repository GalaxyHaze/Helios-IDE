#include "Syntax.h"


// ── Zith Dark theme token colors ─────────────────────────────────────
// Source: zith-extension/vs-code/themes/zith-color-theme.json

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // Comments — muted purple, italic (#6A5A8A)
    QTextCharFormat commentFormat;
    commentFormat.setForeground(QColor("#6A5A8A"));
    commentFormat.setFontItalic(true);

    // Strings — green (#a6d189)
    QTextCharFormat stringFormat;
    stringFormat.setForeground(QColor("#a6d189"));

    // Numbers — blue bold (#04a5e5)
    QTextCharFormat numberFormat;
    numberFormat.setForeground(QColor("#04a5e5"));
    numberFormat.setFontWeight(QFont::Bold);

    // Primitive types — purple bold (#ca9ee6)
    QTextCharFormat typeFormat;
    typeFormat.setForeground(QColor("#ca9ee6"));
    typeFormat.setFontWeight(QFont::Bold);

    // Control flow — red bold (#e64553)
    QTextCharFormat controlFlowFormat;
    controlFlowFormat.setForeground(QColor("#e64553"));
    controlFlowFormat.setFontWeight(QFont::Bold);

    // Declaration keywords (struct, fn, import, etc.) — purple bold (#8839ef)
    QTextCharFormat declFormat;
    declFormat.setForeground(QColor("#8839ef"));
    declFormat.setFontWeight(QFont::Bold);

    // Storage modifiers (let, var, const, mut) — teal (#81c8be)
    QTextCharFormat storageFormat;
    storageFormat.setForeground(QColor("#81c8be"));

    // Async keywords (spawn, await, join) — green bold (#a6d189)
    QTextCharFormat asyncFormat;
    asyncFormat.setForeground(QColor("#a6d189"));
    asyncFormat.setFontWeight(QFont::Bold);

    // Exception keywords (try, catch, throw) — orange bold (#df8e1d)
    QTextCharFormat exceptionFormat;
    exceptionFormat.setForeground(QColor("#df8e1d"));
    exceptionFormat.setFontWeight(QFont::Bold);

    // Other keywords (require, is, prefix, etc.) — white (#eff1f5)
    QTextCharFormat otherKeywordFormat;
    otherKeywordFormat.setForeground(QColor("#eff1f5"));

    // Boolean/null — red-orange bold (#dd7878)
    QTextCharFormat literalFormat;
    literalFormat.setForeground(QColor("#dd7878"));
    literalFormat.setFontWeight(QFont::Bold);

    // Logical operators (and, or, not) — red bold (#d20f39)
    QTextCharFormat logicalOpFormat;
    logicalOpFormat.setForeground(QColor("#d20f39"));
    logicalOpFormat.setFontWeight(QFont::Bold);

    // Comparison/assignment/arithmetic operators — blue (#7287fd)
    QTextCharFormat operatorFormat;
    operatorFormat.setForeground(QColor("#7287fd"));

    // Other operators (->, ?) — teal (#179299)
    QTextCharFormat otherOpFormat;
    otherOpFormat.setForeground(QColor("#179299"));

    // Punctuation (brackets) — pink (#ea999c)
    QTextCharFormat bracketFormat;
    bracketFormat.setForeground(QColor("#ea999c"));

    // Punctuation (., ; ,) — gray (#a5adce)
    QTextCharFormat punctFormat;
    punctFormat.setForeground(QColor("#a5adce"));

    // Multi-line comments — same as single-line
    multiLineCommentFormat.setForeground(QColor("#6A5A8A"));
    multiLineCommentFormat.setFontItalic(true);

    // ── Regex patterns ─────────────────────────────────────────
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");

    // ── WORD KEYWORDS (use \b word boundaries) ─────────────────
    struct KeywordGroup {
        QStringList words;
        QTextCharFormat fmt;
    };

    QVector<KeywordGroup> groups = {
        // Primitive types — purple bold
        {{"i8","i16","i32","i64","i128","i256",
          "u8","u16","u32","u64","u128","u256",
          "f32","f64","f128","bool","char","void","null",
          "type","struct","component","enum","raw","union",
          "family","entity","trait"}, typeFormat},

        // Control flow — red bold
        {{"if","else","for","in","switch",
          "return","break","continue","goto","end",
          "marker","scene","match","case","when"}, controlFlowFormat},

        // Declaration keywords — purple bold
        {{"fn","import","use","context","macro","export",
          "from","as","implement","alias","interface","mod"}, declFormat},

        // Storage modifiers — teal
        {{"let","var","auto","const","mut","global",
          "comptime","lend","shared","view","unique",
          "extension","yield","flowing","entry","noreturn",
          "recurse","pub","private","protected"}, storageFormat},

        // Async keywords — green bold
        {{"spawn","await","join"}, asyncFormat},

        // Exception keywords — orange bold
        {{"try","catch","must","throw","do","drop"}, exceptionFormat},

        // Other keywords — white
        {{"require","is","prefix","sufix","infix"}, otherKeywordFormat},

        // Boolean/null literals — red-orange bold
        {{"true","false","null"}, literalFormat},

        // Logical operators — red bold
        {{"and","or","not"}, logicalOpFormat},
    };

    auto groupedPattern = [](const QStringList &tokens, bool wordBoundary) {
        QStringList escapedTokens;
        escapedTokens.reserve(tokens.size());
        for (const QString &token : tokens)
            escapedTokens.append(QRegularExpression::escape(token));

        const QString pattern = "(?:" + escapedTokens.join('|') + ')';
        return QRegularExpression(wordBoundary ? "\\b" + pattern + "\\b" : pattern);
    };

    for (const auto &g : groups)
        keywordRules.append({groupedPattern(g.words, true), g.fmt});

    // ── SYMBOL OPERATORS (need QRegularExpression::escape) ─────
    struct OpGroup {
        QStringList symbols;
        QTextCharFormat fmt;
    };

    QVector<OpGroup> opGroups = {
        // Comparison — blue
        {{"==","!=",">=","<=","<",">"}, operatorFormat},
        // Assignment — blue
        {{"+=","-=","*=","/=","%=",":=","="}, operatorFormat},
        // Arithmetic — blue
        {{"+","-","*","/","%"}, operatorFormat},
        // Other operators — teal
        {{"->","...","..","?","!","::",":","|","^","&","~","<<",">>"}, otherOpFormat},
        // Brackets — pink
        {{"(",")","{","}","[","]"}, bracketFormat},
        // Punctuation — gray
        {{",",";","."}, punctFormat},
    };

    for (const auto &g : opGroups)
        operatorRules.append({groupedPattern(g.symbols, false), g.fmt});

    // ── Numbers — blue bold ────────────────────────────────────
    HighlightingRule hexRule;
    hexRule.pattern = QRegularExpression("\\b0[xX][0-9a-fA-F]+\\b");
    hexRule.format = numberFormat;
    operatorRules.append(hexRule);

    HighlightingRule binRule;
    binRule.pattern = QRegularExpression("\\b0[bB][01]+\\b");
    binRule.format = numberFormat;
    operatorRules.append(binRule);

    HighlightingRule floatRule;
    floatRule.pattern = QRegularExpression("\\b[0-9]+\\.[0-9]+([eE][+-]?[0-9]+)?\\b");
    floatRule.format = numberFormat;
    operatorRules.append(floatRule);

    HighlightingRule intRule;
    intRule.pattern = QRegularExpression("\\b[0-9]+\\b");
    intRule.format = numberFormat;
    operatorRules.append(intRule);

    // ── Strings — green ────────────────────────────────────────
    // Double-quoted strings with escape sequences
    {
        HighlightingRule rule;
        rule.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"");
        rule.format = stringFormat;
        stringRules.append(rule);
    }

    // Single-quoted strings with escape sequences
    {
        HighlightingRule rule;
        rule.pattern = QRegularExpression("'(?:[^'\\\\]|\\\\.)*'");
        rule.format = stringFormat;
        stringRules.append(rule);
    }

    // Line comments (//...) — muted purple italic
    {
        HighlightingRule rule;
        rule.pattern = QRegularExpression("//[^\n]*");
        rule.format = commentFormat;
        keywordRules.append(rule);
    }

    for (const HighlightingRule &rule : operatorRules) {
        rule.pattern.optimize();
    }
    for (const HighlightingRule &rule : keywordRules) {
        rule.pattern.optimize();
    }
    for (const HighlightingRule &rule : stringRules) {
        rule.pattern.optimize();
    }
}

void SyntaxHighlighter::addKeywords(const QStringList &keywordList, const QColor &color, bool bold)
{
    QTextCharFormat format;
    format.setForeground(color);
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);

    for (const QString &keyword : keywordList) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression("\\b" + keyword + "\\b");
        rule.format = format;
        keywordRules.append(rule);
    }
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    // Exclude leading whitespaces from block highlighting search start
    int pos = 0;
    int length = text.length();
    while (pos < length) {
        QChar c = text[pos];
        if (c == ' ' || c == '\t') {
            pos++;
        } else {
            break;
        }
    }

    // On extra long lines, combined time of applying all regexes to that single line can cause
    // a very noticeable complete application freeze. Prevent this by bypassing heavy highlights on such lines.
    bool extraLongLine = length > 5000;

    if (!extraLongLine) {
        // Apply keyword rules (types, control flow, keywords, booleans, comments)
        for (const HighlightingRule &rule : keywordRules) {
            QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text, pos);
            while (it.hasNext()) {
                QRegularExpressionMatch m = it.next();
                setFormat(m.capturedStart(), m.capturedLength(), rule.format);
            }
        }

        // Apply operator/symbol/number rules
        for (const HighlightingRule &rule : operatorRules) {
            QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text, pos);
            while (it.hasNext()) {
                QRegularExpressionMatch m = it.next();
                setFormat(m.capturedStart(), m.capturedLength(), rule.format);
            }
        }

        // Apply string rules
        for (const HighlightingRule &rule : stringRules) {
            QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text, pos);
            while (it.hasNext()) {
                QRegularExpressionMatch m = it.next();
                setFormat(m.capturedStart(), m.capturedLength(), rule.format);
            }
        }
    }

    // Multi-line comment logic (runs even on extra long lines to propagate state correctly)
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = commentEndExpression.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int commentLength = 0;

        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }

        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
