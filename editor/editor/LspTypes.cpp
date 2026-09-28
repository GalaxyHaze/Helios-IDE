#include "LspTypes.h"

namespace
{
bool providerEnabled(const QJsonObject &capabilities, const char *name)
{
    const QJsonValue value = capabilities.value(QLatin1String(name));
    return !value.isUndefined() && !value.isNull() && value.toBool(true);
}
}

LspServerCapabilities LspServerCapabilities::fromJson(
    const QJsonObject &capabilities)
{
    LspServerCapabilities result;
    result.completionProvider =
        providerEnabled(capabilities, "completionProvider");
    result.hoverProvider = providerEnabled(capabilities, "hoverProvider");
    result.signatureHelpProvider =
        providerEnabled(capabilities, "signatureHelpProvider");
    result.definitionProvider =
        providerEnabled(capabilities, "definitionProvider");
    result.declarationProvider =
        providerEnabled(capabilities, "declarationProvider");
    result.implementationProvider =
        providerEnabled(capabilities, "implementationProvider");
    result.referencesProvider =
        providerEnabled(capabilities, "referencesProvider");
    result.documentHighlightProvider =
        providerEnabled(capabilities, "documentHighlightProvider");
    result.documentSymbolProvider =
        providerEnabled(capabilities, "documentSymbolProvider");
    result.renameProvider = providerEnabled(capabilities, "renameProvider");
    result.formattingProvider =
        providerEnabled(capabilities, "documentFormattingProvider");
    result.foldingRangeProvider =
        providerEnabled(capabilities, "foldingRangeProvider");
    result.codeActionProvider =
        providerEnabled(capabilities, "codeActionProvider");
    result.semanticTokensProvider =
        providerEnabled(capabilities, "semanticTokensProvider");
    result.executeCommandProvider =
        providerEnabled(capabilities, "executeCommandProvider");

    const QJsonValue sync = capabilities.value("textDocumentSync");
    result.documentSyncKind =
        sync.isObject() ? sync.toObject().value("change").toInt(1)
                        : sync.toInt(1);
    return result;
}
