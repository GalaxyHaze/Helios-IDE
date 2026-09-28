#ifndef LSPRESULTDECODER_H
#define LSPRESULTDECODER_H

#include "LspTypes.h"

class LspResultDecoder
{
public:
    static LspRange range(const QJsonObject &value);
    static LspLocation location(const QJsonObject &value);
    static QList<LspLocation> locations(const QJsonValue &value);
    static QList<LspSemanticToken> semanticTokens(const QJsonValue &value);
    static QList<LspFoldingRange> foldingRanges(const QJsonValue &value);

    static QList<LspCompletionItem> completionItems(const QJsonValue &value);
    static LspCompletionItem completionItem(const QJsonObject &value,
                                            bool includeDocumentation = false);
    static LspHoverInfo hover(const QJsonObject &value);
    static LspSignatureHelp signatureHelp(const QJsonObject &value);

    static QList<LspRange> documentHighlights(const QJsonValue &value);
    static QList<QPair<LspRange, QString>> textEdits(const QJsonValue &value);
    static QList<LspDiagnostic> diagnostics(const QJsonArray &values);
};

#endif
