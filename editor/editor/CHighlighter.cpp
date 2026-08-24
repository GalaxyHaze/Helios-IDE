#include "CHighlighter.h"

#include "../core/ThemeManager.h"

CHighlighter::CHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    initializeRules();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
            [this]() {
                initializeRules();
                rehighlight();
            });
}

QTextCharFormat CHighlighter::formatFor(const QString &token) const
{
    const SyntaxStyle style = ThemeManager::instance().syntaxStyle(token);
    QTextCharFormat format;
    format.setForeground(style.color);
    format.setFontWeight(style.bold ? QFont::Bold : QFont::Normal);
    format.setFontItalic(style.italic);
    return format;
}

void CHighlighter::initializeRules()
{
    m_rules.clear();

    const QTextCharFormat commentFormat = formatFor("comment");
    const QTextCharFormat stringFormat = formatFor("string");
    const QTextCharFormat numberFormat = formatFor("number");
    const QTextCharFormat typeFormat = formatFor("type");
    const QTextCharFormat controlFormat = formatFor("control");
    const QTextCharFormat jumpFormat = formatFor("jump");
    const QTextCharFormat keywordFormat = formatFor("keyword");
    const QTextCharFormat preprocessorFormat = formatFor("preprocessor");
    const QTextCharFormat storageFormat = formatFor("storage");
    const QTextCharFormat literalFormat = formatFor("literal");
    const QTextCharFormat operatorFormat = formatFor("operator");
    const QTextCharFormat otherOperatorFormat = formatFor("otherOperator");
    const QTextCharFormat bracketFormat = formatFor("bracket");
    const QTextCharFormat punctuationFormat = formatFor("punctuation");
    m_blockCommentFormat = commentFormat;
    m_commentStart = QRegularExpression("/\\*");
    m_commentEnd = QRegularExpression("\\*/");

    struct KeywordGroup
    {
        QStringList words;
        QTextCharFormat format;
    };

    const QVector<KeywordGroup> keywordGroups = {
        {{"int", "char", "float", "double", "void", "long", "short",
          "signed", "unsigned", "_Bool", "_Complex", "_Imaginary",
          "size_t", "ptrdiff_t", "wchar_t", "int8_t", "uint8_t",
          "int16_t", "uint16_t", "int32_t", "uint32_t", "int64_t",
          "uint64_t", "intptr_t", "uintptr_t"}, typeFormat},
        {{"if", "else", "for", "while", "do", "switch", "case", "default"},
         controlFormat},
        {{"break", "continue", "return", "goto"}, jumpFormat},
        {{"typedef", "struct", "union", "enum", "sizeof", "alignof",
          "alignas", "_Alignof", "_Alignas", "_Atomic", "_Generic",
          "_Noreturn", "_Thread_local", "_Static_assert", "static_assert"}, keywordFormat},
        {{"auto", "const", "extern", "register", "restrict", "static",
          "volatile", "inline", "_Thread_local"}, storageFormat},
        {{"true", "false", "NULL", "nullptr"}, literalFormat},
    };

    auto addWordGroup = [this](const KeywordGroup &group) {
        QStringList escapedWords;
        escapedWords.reserve(group.words.size());
        for (const QString &word : group.words)
            escapedWords.append(QRegularExpression::escape(word));
        const QString pattern = "\\b(?:" + escapedWords.join('|') + ")\\b";
        m_rules.append({QRegularExpression(pattern), group.format});
    };

    for (const KeywordGroup &group : keywordGroups)
        addWordGroup(group);

    auto addSymbolGroup = [this](const QStringList &symbols, const QTextCharFormat &format) {
        QStringList escapedSymbols;
        escapedSymbols.reserve(symbols.size());
        for (const QString &symbol : symbols)
            escapedSymbols.append(QRegularExpression::escape(symbol));
        const QString pattern = "(?:" + escapedSymbols.join('|') + ")";
        m_rules.append({QRegularExpression(pattern), format});
    };

    addSymbolGroup({"==", "!=", ">=", "<=", "=", "<", ">"}, operatorFormat);
    addSymbolGroup({"+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
                    "<<=", ">>=", "++", "--", "+", "-", "*", "/", "%"}, operatorFormat);
    addSymbolGroup({"->", "?", "&", "|", "^", "~", "<<", ">>", "!", "::"},
                   otherOperatorFormat);
    addSymbolGroup({"(", ")", "{", "}", "[", "]"}, bracketFormat);
    addSymbolGroup({",", ";", "."}, punctuationFormat);

    auto addPattern = [this](const QRegularExpression &pattern, const QTextCharFormat &format) {
        m_rules.append({pattern, format});
    };

    addPattern(QRegularExpression("\\b0[xX][0-9a-fA-F_]+(?:[uUlL]+)?\\b"), numberFormat);
    addPattern(QRegularExpression("\\b0[bB][01_]+(?:[uUlL]+)?\\b"), numberFormat);
    addPattern(QRegularExpression("\\b0[0-7_]+(?:[uUlL]+)?\\b"), numberFormat);
    addPattern(QRegularExpression("\\b[0-9]+\\.[0-9_]+(?:[eE][+-]?[0-9]+)?(?:[fFlL])?\\b"),
               numberFormat);
    addPattern(QRegularExpression("\\b[0-9]+(?:[eE][+-]?[0-9]+)(?:[fFlL])?\\b"),
               numberFormat);
    addPattern(QRegularExpression("\\b[0-9]+(?:[uUlL]+)?\\b"), numberFormat);

    addPattern(QRegularExpression("\"(?:\\\\.|[^\"\\\\])*\""), stringFormat);
    addPattern(QRegularExpression("'(?:\\\\.|[^'\\\\])*'"), stringFormat);

    addPattern(QRegularExpression("(?<!\\S)#[\\t ]*"
                                  "(?:include|define|undef|if|ifdef|ifndef|elif|else|endif|error|warning|pragma|line)\\b.*"),
               preprocessorFormat);
    addPattern(QRegularExpression("//[^\\n]*"), commentFormat);

    for (const HighlightingRule &rule : m_rules)
        rule.pattern.optimize();
}

void CHighlighter::highlightBlock(const QString &text)
{
    auto applyRule = [this, &text](const HighlightingRule &rule) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            const QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    };

    for (const HighlightingRule &rule : m_rules)
        applyRule(rule);

    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(m_commentStart);

    while (startIndex >= 0) {
        const QRegularExpressionMatch endMatch = m_commentEnd.match(text, startIndex);
        const int endIndex = endMatch.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, m_blockCommentFormat);
        startIndex = text.indexOf(m_commentStart, startIndex + commentLength);
    }
}
