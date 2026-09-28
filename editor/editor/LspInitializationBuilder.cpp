#include "LspInitializationBuilder.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonValue>
#include <QUrl>

QJsonObject LspInitializationBuilder::build(
    const LspInitializationOptions &options)
{
    const bool isClangd = options.initMode == QLatin1String("clangd");
    const QJsonObject textDocument{
        {"synchronization",
         QJsonObject{{"dynamicRegistration", true},
                     {"willSave", true},
                     {"willSaveWaitUntil", false},
                     {"didSave", true}}},
        {"completion",
         QJsonObject{
             {"dynamicRegistration", true},
             {"completionItem",
              QJsonObject{
                  {"snippetSupport", true},
                  {"resolveSupport",
                   QJsonObject{{"properties",
                                QJsonArray{"detail", "documentation"}}}}}}}},
        {"hover",
         QJsonObject{{"dynamicRegistration", true},
                     {"contentFormat",
                      QJsonArray{"markdown", "plaintext"}}}},
        {"definition", QJsonObject{{"dynamicRegistration", true}}},
        {"declaration", QJsonObject{{"dynamicRegistration", true}}},
        {"implementation", QJsonObject{{"dynamicRegistration", true}}},
        {"references", QJsonObject{{"dynamicRegistration", true}}},
        {"documentHighlight", QJsonObject{{"dynamicRegistration", true}}},
        {"documentSymbol",
         QJsonObject{{"dynamicRegistration", true},
                     {"hierarchicalDocumentSymbolSupport", true}}},
        {"rename", QJsonObject{{"dynamicRegistration", true}}},
        {"formatting", QJsonObject{{"dynamicRegistration", true}}},
        {"foldingRange", QJsonObject{{"dynamicRegistration", true}}},
        {"codeAction", QJsonObject{{"dynamicRegistration", true}}},
        {"semanticTokens",
         QJsonObject{{"dynamicRegistration", true},
                     {"requests", QJsonObject{{"full", true}}},
                     {"formats", QJsonArray{"relative"}}}}};

    const QJsonObject workspaceEdit{
        {QStringLiteral("documentChanges"), true},
        {QStringLiteral("resourceOperations"),
         QJsonArray{QStringLiteral("create"), QStringLiteral("rename"),
                    QStringLiteral("delete")}},
        {QStringLiteral("failureHandling"), QStringLiteral("transactional")}};
    const QJsonObject workspaceCapabilities{
        {QStringLiteral("applyEdit"), true},
        {QStringLiteral("configuration"), true},
        {QStringLiteral("didChangeConfiguration"),
         QJsonObject{{QStringLiteral("dynamicRegistration"), true}}},
        {QStringLiteral("workspaceEdit"), workspaceEdit}};

    QJsonObject params{
        {QStringLiteral("processId"), QJsonValue::Null},
        {QStringLiteral("capabilities"),
         QJsonObject{{QStringLiteral("general"),
                      QJsonObject{{QStringLiteral("positionEncodings"),
                                   QJsonArray{QStringLiteral("utf-16")}}}},
                     {QStringLiteral("workspace"), workspaceCapabilities},
                     {QStringLiteral("textDocument"), textDocument}}}};
    if (!isClangd) {
        QJsonObject capabilities = params.value("capabilities").toObject();
        capabilities.insert(
            QStringLiteral("experimental"),
            QJsonObject{{QStringLiteral("zith"),
                         QJsonObject{{QStringLiteral("requestSaveAll"), true}}}});
        params.insert(QStringLiteral("capabilities"), capabilities);
    }

    const QString root = options.workspaceRoot.isEmpty()
                             ? QDir::currentPath()
                             : options.workspaceRoot;
    params["rootUri"] = QUrl::fromLocalFile(root).toString();
    params["rootPath"] = root;

    if (!isClangd) {
        const QJsonObject zithOptions{
            {"frontend",
             QJsonObject{{"enabled", true},
                         {"warmupStdlib", true},
                         {"statusNotifications", true},
                         {"maxWorkers", 0}}}};
        QJsonObject initializationOptions{{"zith", zithOptions}};
        if (!options.stdlibPath.isEmpty())
            initializationOptions["stdlibPath"] = options.stdlibPath;
        params["initializationOptions"] = initializationOptions;
    }

    return params;
}
