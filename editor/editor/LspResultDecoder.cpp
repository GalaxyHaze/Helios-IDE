#include "LspResultDecoder.h"

LspRange LspResultDecoder::range(const QJsonObject &value)
{
    const QJsonObject start = value.value("start").toObject();
    const QJsonObject end = value.value("end").toObject();
    return {{start.value("line").toInt(),
              start.value("character").toInt()},
            {end.value("line").toInt(),
             end.value("character").toInt()}};
}

LspLocation LspResultDecoder::location(const QJsonObject &value)
{
    LspLocation result;
    if (value.contains("targetUri")) {
        result.uri = value.value("targetUri").toString();
        result.range =
            range(value.value("targetSelectionRange").toObject());
        if (result.range.start.line == 0 &&
            result.range.start.character == 0 &&
            result.range.end.line == 0 &&
            result.range.end.character == 0) {
            result.range = range(value.value("targetRange").toObject());
        }
    } else {
        result.uri = value.value("uri").toString();
        result.range = range(value.value("range").toObject());
    }
    return result;
}

QList<LspLocation> LspResultDecoder::locations(const QJsonValue &value)
{
    QList<LspLocation> result;
    if (value.isArray()) {
        for (const QJsonValue &entry : value.toArray())
            result.append(location(entry.toObject()));
    } else if (value.isObject()) {
        result.append(location(value.toObject()));
    }
    return result;
}

QList<LspSemanticToken> LspResultDecoder::semanticTokens(
    const QJsonValue &value)
{
    const QJsonArray data =
        value.isObject() ? value.toObject().value("data").toArray()
                         : value.toArray();
    QList<LspSemanticToken> result;
    int line = 0;
    int character = 0;
    for (int index = 0; index + 4 < data.size(); index += 5) {
        const int deltaLine = data.at(index).toInt();
        const int deltaStart = data.at(index + 1).toInt();
        const int length = data.at(index + 2).toInt();
        if (deltaLine < 0 || deltaStart < 0 || length <= 0)
            continue;

        line += deltaLine;
        character = deltaLine == 0 ? character + deltaStart : deltaStart;
        result.append({{{line, character},
                        {line, character + length}},
                       data.at(index + 3).toInt(),
                       data.at(index + 4).toInt()});
    }
    return result;
}

QList<LspFoldingRange> LspResultDecoder::foldingRanges(
    const QJsonValue &value)
{
    QList<LspFoldingRange> result;
    for (const QJsonValue &entry : value.toArray()) {
        const QJsonObject object = entry.toObject();
        if (!object.contains(QStringLiteral("startLine")) ||
            !object.contains(QStringLiteral("endLine"))) {
            continue;
        }
        const int startLine = object.value(QStringLiteral("startLine")).toInt(-1);
        const int endLine = object.value(QStringLiteral("endLine")).toInt(-1);
        if (startLine < 0 || endLine <= startLine)
            continue;
        result.append(
            {startLine,
             object.value(QStringLiteral("startCharacter")).toInt(0),
             endLine,
             object.value(QStringLiteral("endCharacter")).toInt(0)});
    }
    return result;
}

QList<LspCompletionItem> LspResultDecoder::completionItems(
    const QJsonValue &value)
{
    const QJsonArray values =
        value.isArray() ? value.toArray()
                        : value.toObject().value("items").toArray();
    QList<LspCompletionItem> result;
    for (const QJsonValue &entry : values)
        result.append(completionItem(entry.toObject()));
    return result;
}

LspCompletionItem LspResultDecoder::completionItem(
    const QJsonObject &value,
    bool includeDocumentation)
{
    LspCompletionItem result;
    result.label = value.value("label").toString();
    result.kind = value.value("kind").toInt();
    result.detail = value.value("detail").toString();
    result.insertText =
        value.value("insertText").toString(result.label);
    result.insertTextFormat = value.value("insertTextFormat").toInt(1);
    if (includeDocumentation) {
        const QJsonValue documentation = value.value("documentation");
        if (documentation.isString()) {
            result.detail += "\n" + documentation.toString();
        } else if (documentation.isObject()) {
            result.detail +=
                "\n" + documentation.toObject().value("value").toString();
        }
    }
    result.rawItem = value;
    return result;
}

LspHoverInfo LspResultDecoder::hover(const QJsonObject &value)
{
    LspHoverInfo result;
    const QJsonValue contents = value.value("contents");
    if (contents.isString()) {
        result.contents = contents.toString();
    } else if (contents.isObject()) {
        result.contents = contents.toObject().value("value").toString();
    } else {
        for (const QJsonValue &entry : contents.toArray()) {
            result.contents +=
                (result.contents.isEmpty() ? QString() : "\n\n") +
                (entry.isString()
                     ? entry.toString()
                     : entry.toObject().value("value").toString());
        }
    }
    result.range = range(value.value("range").toObject());
    return result;
}

LspSignatureHelp LspResultDecoder::signatureHelp(const QJsonObject &value)
{
    LspSignatureHelp result;
    const QJsonArray signatures = value.value("signatures").toArray();
    if (!signatures.isEmpty()) {
        const QJsonObject signature = signatures.first().toObject();
        result.activeSignature = signature.value("label").toString();
        for (const QJsonValue &parameter :
             signature.value("parameters").toArray()) {
            result.parameters.append(
                parameter.toObject().value("label").toString());
        }
    }
    result.activeParameter = value.value("activeParameter").toInt();
    return result;
}

QList<LspRange> LspResultDecoder::documentHighlights(
    const QJsonValue &value)
{
    QList<LspRange> result;
    for (const QJsonValue &entry : value.toArray())
        result.append(range(entry.toObject().value("range").toObject()));
    return result;
}

QList<QPair<LspRange, QString>> LspResultDecoder::textEdits(
    const QJsonValue &value)
{
    QList<QPair<LspRange, QString>> result;
    for (const QJsonValue &entry : value.toArray()) {
        const QJsonObject object = entry.toObject();
        result.append({range(object.value("range").toObject()),
                       object.value("newText").toString()});
    }
    return result;
}

QList<LspDiagnostic> LspResultDecoder::diagnostics(
    const QJsonArray &values)
{
    QList<LspDiagnostic> result;
    for (const QJsonValue &entry : values) {
        const QJsonObject value = entry.toObject();
        LspDiagnostic diagnostic;
        diagnostic.range = range(value.value("range").toObject());
        diagnostic.severity = value.value("severity").toInt();
        diagnostic.message = value.value("message").toString();
        diagnostic.source = value.value("source").toString();
        result.append(diagnostic);
    }
    return result;
}
