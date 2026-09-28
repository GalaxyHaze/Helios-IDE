#ifndef LSPTYPES_H
#define LSPTYPES_H

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>

struct LspPosition
{
    int line = 0;
    int character = 0;
};

struct LspRange
{
    LspPosition start;
    LspPosition end;
};

struct LspTextChange
{
    LspRange range;
    QString text;
};

struct LspDiagnostic
{
    LspRange range;
    int severity = 0;
    QString message;
    QString source;
};

struct LspSemanticToken
{
    LspRange range;
    int tokenType = 0;
    int modifiers = 0;
};

struct LspFoldingRange
{
    int startLine = 0;
    int startCharacter = 0;
    int endLine = 0;
    int endCharacter = 0;
};

struct LspCompletionItem
{
    QString label;
    int kind = 0;
    QString detail;
    QString insertText;
    int insertTextFormat = 1;
    QJsonObject rawItem;
};

struct LspLocation
{
    QString uri;
    LspRange range;
};

struct LspHoverInfo
{
    QString contents;
    LspRange range;
};

struct LspSignatureHelp
{
    QString activeSignature;
    int activeParameter = 0;
    QStringList parameters;
};

struct LspStartOptions
{
    QString serverPath;
    QString stdlibPath;
    QString workspaceRoot;
    QString initMode;
};

struct LspServerCapabilities
{
    bool completionProvider = false;
    bool hoverProvider = false;
    bool signatureHelpProvider = false;
    bool definitionProvider = false;
    bool implementationProvider = false;
    bool declarationProvider = false;
    bool referencesProvider = false;
    bool documentHighlightProvider = false;
    bool renameProvider = false;
    bool documentSymbolProvider = false;
    bool formattingProvider = false;
    bool foldingRangeProvider = false;
    bool codeActionProvider = false;
    bool semanticTokensProvider = false;
    bool executeCommandProvider = false;
    int documentSyncKind = 1;

    static LspServerCapabilities fromJson(const QJsonObject &capabilities);
};

#endif
