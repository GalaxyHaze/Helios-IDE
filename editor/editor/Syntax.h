#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QRegularExpression>
#ifdef SyntaxHighlighterPerfCheck
#include <chrono>
#endif

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SyntaxHighlighter(QTextDocument *parent = nullptr);
#ifdef SyntaxHighlighterPerfCheck
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

#ifdef SyntaxHighlighterPerfCheck
    std::chrono::microseconds highlightDuration;
#endif
};

#endif // SYNTAXHIGHLIGHTER_H