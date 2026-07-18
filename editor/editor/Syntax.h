#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QRegularExpression>
#ifdef SYNTAX_HIGHLIGHTER_TIMING
#include <QElapsedTimer>
#endif

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SyntaxHighlighter(QTextDocument *parent = nullptr);
#ifdef SYNTAX_HIGHLIGHTER_TIMING
    virtual ~SyntaxHighlighter() override;
#endif

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    // Maps your ZithTokenType enums (simulated here by strings/groups) to Colors
    void initializeHighlightingRules();
    QTextCharFormat formatFor(const QString &token) const;

    // Helper to add a list of keywords to a specific rule group
    void addKeywords(const QStringList &keywordList, const QColor &color, bool bold = false);

    QVector<HighlightingRule> typeRules;
    QVector<HighlightingRule> controlFlowRules;
    QVector<HighlightingRule> keywordRules;
    QVector<HighlightingRule> operatorRules;
    QVector<HighlightingRule> stringRules;

    // Multi-line comment state
    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
    QTextCharFormat multiLineCommentFormat;

#ifdef SYNTAX_HIGHLIGHTER_TIMING
    qint64 highlightDuration = 0;
#endif
};

#endif // SYNTAXHIGHLIGHTER_H
