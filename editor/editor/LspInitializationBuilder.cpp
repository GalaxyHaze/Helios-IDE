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
        {"completion",
         QJsonObject{
             {"completionItem",
              QJsonObject{
                  {"snippetSupport", true},
                  {"resolveSupport",
                   QJsonObject{{"properties",
                                QJsonArray{"detail", "documentation"}}}}}}}},
        {"hover",
         QJsonObject{{"contentFormat", QJsonArray{"markdown", "plaintext"}}}},
        {"definition", QJsonObject{}},
        {"declaration", QJsonObject{}},
        {"implementation", QJsonObject{}},
        {"references", QJsonObject{}},
        {"documentHighlight", QJsonObject{}},
        {"documentSymbol",
         QJsonObject{{"hierarchicalDocumentSymbolSupport", true}}},
        {"rename", QJsonObject{}},
        {"formatting", QJsonObject{}},
        {"foldingRange", QJsonObject{}},
        {"codeAction", QJsonObject{}},
        {"semanticTokens", QJsonObject{{"requests", QJsonObject{{"full", true}}},
                                       {"formats", QJsonArray{"relative"}}}}};

    QJsonObject params{{"processId", QJsonValue::Null},
                       {"capabilities",
                        QJsonObject{{"textDocument", textDocument}}}};
    if (!isClangd) {
        params["capabilities"] =
            QJsonObject{{"textDocument", textDocument},
                        {"experimental",
                         QJsonObject{{"zith",
                                      QJsonObject{{"requestSaveAll", true}}}}}};
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
