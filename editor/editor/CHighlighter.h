#ifndef CHIGHLIGHTER_H
#define CHIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

class CHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit CHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    void initializeRules();
    QTextCharFormat formatFor(const QString &token) const;

    QVector<HighlightingRule> m_rules;
    QRegularExpression m_commentStart;
    QRegularExpression m_commentEnd;
    QTextCharFormat m_blockCommentFormat;
};

#endif // CHIGHLIGHTER_H
