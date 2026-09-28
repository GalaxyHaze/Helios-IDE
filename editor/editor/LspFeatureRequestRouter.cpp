#include "LspFeatureRequestRouter.h"

#include "LspResultDecoder.h"

#include <utility>

namespace {
QJsonObject positionObject(const LspPosition &position)
{
    return {{"line", position.line}, {"character", position.character}};
}

QJsonObject rangeObject(const LspRange &range)
{
    return {{"start", positionObject(range.start)},
            {"end", positionObject(range.end)}};
}

QJsonObject positionParams(const QString &uri, const LspPosition &position)
{
    return {{"textDocument", QJsonObject{{"uri", uri}}},
            {"position", positionObject(position)}};
}
} // namespace

LspFeatureRequestRouter::LspFeatureRequestRouter(
    LspFeatureRequestRouterCallbacks callbacks, QObject *parent)
    : QObject(parent), m_callbacks(std::move(callbacks))
{
}

qint64 LspFeatureRequestRouter::send(
    const QString &method, const QJsonObject &params, const QString &uri,
    int version, bool cancellable,
    std::function<void(const QJsonObject &)> callback)
{
    if (!m_callbacks.sendRequest)
        return -1;
    return m_callbacks.sendRequest(method, params, uri, version, cancellable,
                                   std::move(callback));
}

void LspFeatureRequestRouter::requestPosition(
    PositionFeature feature, const QString &uri, int version,
    const LspPosition &position)
{
    const QJsonObject params = positionParams(uri, position);
    switch (feature) {
    case PositionFeature::Completion:
        send("textDocument/completion", params, uri, version, true,
             [this, uri, version](const QJsonObject &response) {
                 emit completionResults(
                     uri, version,
                     LspResultDecoder::completionItems(
                         response.value("result")));
             });
        break;
    case PositionFeature::Hover:
        send("textDocument/hover", params, uri, version, true,
             [this, uri, version](const QJsonObject &response) {
                 emit hoverResult(
                     uri, version,
                     LspResultDecoder::hover(
                         response.value("result").toObject()));
             });
        break;
    case PositionFeature::Definition:
    case PositionFeature::Declaration:
    case PositionFeature::Implementation: {
        QString method;
        switch (feature) {
        case PositionFeature::Definition:
            method = QStringLiteral("textDocument/definition");
            break;
        case PositionFeature::Declaration:
            method = QStringLiteral("textDocument/declaration");
            break;
        case PositionFeature::Implementation:
            method = QStringLiteral("textDocument/implementation");
            break;
        default:
            break;
        }
        send(method, params, uri, version, true,
             [this, feature, uri, version](const QJsonObject &response) {
                 const QJsonValue result = response.value("result");
                 const QJsonArray locations = result.toArray();
                 const LspLocation location = LspResultDecoder::location(
                     result.isArray() && !locations.isEmpty()
                         ? locations.first().toObject()
                         : result.toObject());
                 switch (feature) {
                 case PositionFeature::Definition:
                     emit definitionResult(uri, version, location);
                     break;
                 case PositionFeature::Declaration:
                     emit declarationResult(uri, version, location);
                     break;
                 case PositionFeature::Implementation:
                     emit implementationResult(uri, version, location);
                     break;
                 default:
                     break;
                 }
             });
        break;
    }
    case PositionFeature::References: {
        QJsonObject referencesParams = params;
        referencesParams["context"] =
            QJsonObject{{"includeDeclaration", true}};
        send("textDocument/references", referencesParams, uri, version, true,
             [this, uri, version](const QJsonObject &response) {
                 emit referencesResult(
                     uri, version,
                     LspResultDecoder::locations(
                         response.value("result")));
             });
        break;
    }
    case PositionFeature::DocumentHighlight:
        send("textDocument/documentHighlight", params, uri, version, true,
             [this, uri, version](const QJsonObject &response) {
                 emit documentHighlightsResult(
                     uri, version,
                     LspResultDecoder::documentHighlights(
                         response.value("result")));
             });
        break;
    case PositionFeature::SignatureHelp:
        send("textDocument/signatureHelp", params, uri, version, true,
             [this, uri, version](const QJsonObject &response) {
                 emit signatureHelpResult(
                     uri, version,
                     LspResultDecoder::signatureHelp(
                         response.value("result").toObject()));
             });
        break;
    }
}

void LspFeatureRequestRouter::requestDocument(
    DocumentFeature feature, const QString &uri, int version)
{
    QJsonObject params{{"textDocument", QJsonObject{{"uri", uri}}}};
    QString method;
    bool cancellable = true;
    switch (feature) {
    case DocumentFeature::SemanticTokens:
        method = QStringLiteral("textDocument/semanticTokens/full");
        break;
    case DocumentFeature::Formatting:
        method = QStringLiteral("textDocument/formatting");
        cancellable = false;
        params["options"] = QJsonObject{{"tabSize", 4},
                                        {"insertSpaces", true}};
        break;
    case DocumentFeature::DocumentSymbols:
        method = QStringLiteral("textDocument/documentSymbol");
        break;
    case DocumentFeature::FoldingRanges:
        method = QStringLiteral("textDocument/foldingRange");
        break;
    }

    send(method, params, uri, version, cancellable,
         [this, feature, uri, version](const QJsonObject &response) {
             const QJsonValue result = response.value("result");
             switch (feature) {
             case DocumentFeature::SemanticTokens:
                 emit semanticTokensResult(
                     uri, version,
                     result.toObject().value("data").toArray());
                 break;
             case DocumentFeature::Formatting:
                 emit formattingResult(
                     uri, version, LspResultDecoder::textEdits(result));
                 break;
             case DocumentFeature::DocumentSymbols:
                 emit documentSymbolsResult(uri, version, result.toArray());
                 break;
             case DocumentFeature::FoldingRanges:
                 emit foldingRangesResult(uri, version, result.toArray());
                 break;
             }
         });
}

void LspFeatureRequestRouter::requestRename(
    const QString &uri, int version, const LspPosition &position,
    const QString &newName)
{
    QJsonObject params = positionParams(uri, position);
    params["newName"] = newName;
    send("textDocument/rename", params, uri, version, false,
         [this, uri, version](const QJsonObject &response) {
             emit renameResult(uri, version,
                               response.value("result").toObject());
         });
}

void LspFeatureRequestRouter::requestCodeActions(
    const QString &uri, int version, const LspRange &range,
    const QList<LspDiagnostic> &diagnostics)
{
    QJsonArray diagnosticArray;
    for (const LspDiagnostic &diagnostic : diagnostics) {
        diagnosticArray.append(
            QJsonObject{{"range", rangeObject(diagnostic.range)},
                        {"severity", diagnostic.severity},
                        {"message", diagnostic.message},
                        {"source", diagnostic.source}});
    }
    const QJsonObject params{
        {"textDocument", QJsonObject{{"uri", uri}}},
        {"range", rangeObject(range)},
        {"context", QJsonObject{{"diagnostics", diagnosticArray},
                                {"only", QJsonArray{"refactor.extract"}}}}};
    send("textDocument/codeAction", params, uri, version, false,
         [this, uri, version](const QJsonObject &response) {
             emit codeActionsResult(uri, version,
                                    response.value("result").toArray());
         });
}

void LspFeatureRequestRouter::resolveCompletion(
    const QString &uri, int version, const QJsonObject &item)
{
    send("completionItem/resolve", item, uri, version, true,
         [this, uri, version](const QJsonObject &response) {
             emit completionResolved(
                 uri, version,
                 LspResultDecoder::completionItem(
                     response.value("result").toObject(), true));
         });
}

qint64 LspFeatureRequestRouter::executeWorkspaceCommand(
    const QString &command, const QJsonValue &args,
    std::function<void(const QJsonObject &)> callback)
{
    const qint64 id = send(
        "workspace/executeCommand",
        QJsonObject{{"command", command}, {"arguments", args}}, {}, -1, false,
        [this, command, callback = std::move(callback)](
            const QJsonObject &response) {
            const QJsonValue result = response.value("result");
            if (response.contains("error")) {
                const QJsonObject error = response.value("error").toObject();
                emit commandResult(
                    command, false,
                    error.value("message").toString(
                        QStringLiteral("workspace/executeCommand failed")));
            } else {
                emit commandResult(command, true, result);
            }
            if (callback)
                callback(response);
        });
    if (id < 0) {
        emit commandResult(
            command, false,
            QStringLiteral(
                "LSP is not ready; workspace/executeCommand was not sent"));
    }
    return id;
}
