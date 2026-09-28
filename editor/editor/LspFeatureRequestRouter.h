#ifndef LSPFEATUREREQUESTROUTER_H
#define LSPFEATUREREQUESTROUTER_H

#include "LspTypes.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>

#include <functional>

using LspFeatureRequestSender = std::function<qint64(
    const QString &, const QJsonObject &, const QString &, int, bool,
    std::function<void(const QJsonObject &)>)>;

struct LspFeatureRequestRouterCallbacks
{
    LspFeatureRequestSender sendRequest;
};

class LspFeatureRequestRouter : public QObject
{
    Q_OBJECT

public:
    enum class PositionFeature
    {
        Completion,
        Hover,
        Definition,
        Declaration,
        Implementation,
        References,
        DocumentHighlight,
        SignatureHelp
    };

    enum class DocumentFeature
    {
        SemanticTokens,
        Formatting,
        DocumentSymbols,
        FoldingRanges
    };

    explicit LspFeatureRequestRouter(
        LspFeatureRequestRouterCallbacks callbacks,
        QObject *parent = nullptr);

    void requestPosition(PositionFeature feature, const QString &uri,
                         int version, const LspPosition &position);
    void requestDocument(DocumentFeature feature, const QString &uri,
                         int version);
    void requestRename(const QString &uri, int version,
                       const LspPosition &position, const QString &newName);
    void requestCodeActions(const QString &uri, int version,
                            const LspRange &range,
                            const QList<LspDiagnostic> &diagnostics);
    void resolveCompletion(const QString &uri, int version,
                           const QJsonObject &item);
    qint64 executeWorkspaceCommand(
        const QString &command, const QJsonValue &args,
        std::function<void(const QJsonObject &)> callback = {});

signals:
    void completionResults(const QString &uri, int version,
                           const QList<LspCompletionItem> &items);
    void completionResolved(const QString &uri, int version,
                            const LspCompletionItem &item);
    void hoverResult(const QString &uri, int version,
                     const LspHoverInfo &info);
    void definitionResult(const QString &uri, int version,
                          const LspLocation &location);
    void declarationResult(const QString &uri, int version,
                           const LspLocation &location);
    void implementationResult(const QString &uri, int version,
                              const LspLocation &location);
    void referencesResult(const QString &uri, int version,
                          const QList<LspLocation> &locations);
    void documentHighlightsResult(const QString &uri, int version,
                                  const QList<LspRange> &ranges);
    void signatureHelpResult(const QString &uri, int version,
                             const LspSignatureHelp &help);
    void semanticTokensResult(const QString &uri, int version,
                              const QJsonArray &tokens);
    void formattingResult(
        const QString &uri, int version,
        const QList<QPair<LspRange, QString>> &edits);
    void documentSymbolsResult(const QString &uri, int version,
                               const QJsonArray &symbols);
    void foldingRangesResult(const QString &uri, int version,
                             const QJsonArray &ranges);
    void renameResult(const QString &uri, int version,
                      const QJsonObject &edit);
    void codeActionsResult(const QString &uri, int version,
                           const QJsonArray &actions);
    void commandResult(const QString &command, bool success,
                       const QJsonValue &result);

private:
    qint64 send(const QString &method, const QJsonObject &params,
                const QString &uri, int version, bool cancellable,
                std::function<void(const QJsonObject &)> callback);

    LspFeatureRequestRouterCallbacks m_callbacks;
};

#endif
