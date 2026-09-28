#ifndef LSPCLIENT_H
#define LSPCLIENT_H

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <functional>

#include "LspProcessTransport.h"
#include "LspDocumentProtocol.h"
#include "LspFeatureRequestRouter.h"
#include "LspRequestSender.h"
#include "LspServerMessageDispatcher.h"
#include "LspSessionLifecycle.h"
#include "LspTypes.h"

class TestHelios;

class LspClient : public QObject
{
    Q_OBJECT

public:
    enum class Capability
    {
        Completion,
        Hover,
        SignatureHelp,
        Definition,
        Implementation,
        Declaration,
        References,
        DocumentHighlight,
        Rename,
        DocumentSymbol,
        Formatting,
        FoldingRange,
        CodeAction,
        SemanticTokens,
        ExecuteCommand
    };

    explicit LspClient(QObject *parent = nullptr);
    ~LspClient() override;

#ifdef HELIOS_UNIT_TESTING
    void waitForFinishedForTesting(int timeoutMs);
    void setReadyForTesting(bool ready);
#endif

    bool start(const LspStartOptions &options);
    bool start(const QString &serverPath, const QString &stdlibPath = {},
               const QString &workspaceRoot = {},
               const QString &initMode = {});
    void stop();
    bool isRunning() const;
    bool isReady() const;
    int documentSyncKind() const { return m_capabilities.documentSyncKind; }
    int documentVersion(const QString &uri) const;

    bool supports(Capability capability) const;

    void openDocument(const QString &uri, const QString &languageId, const QString &text, int version = 1);
    bool changeDocument(const QString &uri,
                        const QList<LspTextChange> &changes, int version);
    bool changeDocumentFull(const QString &uri, const QString &fullText,
                            int version);
    void closeDocument(const QString &uri);
    void saveDocument(const QString &uri);

    void requestCompletion(const QString &uri, int version, const LspPosition &pos);
    void requestHover(const QString &uri, int version, const LspPosition &pos);
    void requestDefinition(const QString &uri, int version, const LspPosition &pos);
    void requestDeclaration(const QString &uri, int version, const LspPosition &pos);
    void requestImplementation(const QString &uri, int version, const LspPosition &pos);
    void requestReferences(const QString &uri, int version, const LspPosition &pos);
    void requestDocumentHighlight(const QString &uri, int version, const LspPosition &pos);
    void requestSignatureHelp(const QString &uri, int version, const LspPosition &pos);
    void requestSemanticTokens(const QString &uri, int version);
    void requestFormatting(const QString &uri, int version);
    void requestDocumentSymbols(const QString &uri, int version);
    void requestFoldingRanges(const QString &uri, int version);
    void requestRename(const QString &uri, int version, const LspPosition &pos, const QString &newName);
    void requestCodeActions(const QString &uri, int version, const LspRange &range, const QList<LspDiagnostic> &diagnostics);
    void resolveCompletion(const QString &uri, int version, const QJsonObject &item);
    qint64 executeWorkspaceCommand(const QString &command, const QJsonValue &args,
                                   std::function<void(const QJsonObject &)> callback = {});

signals:
    void initialized();
    void serverError(const QString &message);
    void serverStopped();
    void processStopped(bool expected);
    void diagnosticsReceived(const QString &uri, int version, const QList<LspDiagnostic> &diagnostics);
    void completionResults(const QString &uri, int version, const QList<LspCompletionItem> &items);
    void completionResolved(const QString &uri, int version, const LspCompletionItem &item);
    void hoverResult(const QString &uri, int version, const LspHoverInfo &info);
    void definitionResult(const QString &uri, int version,
                          const QList<LspLocation> &locations);
    void declarationResult(const QString &uri, int version,
                           const QList<LspLocation> &locations);
    void implementationResult(const QString &uri, int version,
                              const QList<LspLocation> &locations);
    void referencesResult(const QString &uri, int version, const QList<LspLocation> &locations);
    void documentHighlightsResult(const QString &uri, int version, const QList<LspRange> &ranges);
    void signatureHelpResult(const QString &uri, int version, const LspSignatureHelp &help);
    void semanticTokensResult(const QString &uri, int version, const QJsonArray &tokens);
    void formattingResult(const QString &uri, int version, const QList<QPair<LspRange, QString>> &edits);
    void documentSymbolsResult(const QString &uri, int version, const QJsonArray &symbols);
    void foldingRangesResult(const QString &uri, int version, const QJsonArray &ranges);
    void renameResult(const QString &uri, int version, const QJsonObject &edit);
    void codeActionsResult(const QString &uri, int version, const QJsonArray &actions);
    void saveAllRequested();
    void logMessage(const QString &message);
    void showMessage(const QString &message);
    void processOutputReceived(const QString &taskId, const QString &chunk);
    void processExitReceived(const QString &taskId, int exitCode);
    void workDoneProgressReceived(const QString &token, const QString &kind,
                                  const QString &message);
    void commandResult(const QString &command, bool success, const QJsonValue &result);
    void frontendStatusReceived(const QJsonObject &status);
    void metricsReceived(const QJsonObject &metrics);

private slots:
    void onProcessFinished(const LspProcessResult &result);

private:
    friend class TestHelios;
    bool sendMessage(const QJsonObject &msg);
    void dispatchMessage(const QJsonObject &msg);
    qint64 sendRequest(const QString &method, const QJsonObject &params, const QString &uri, int version,
                       bool cancellable, std::function<void(const QJsonObject &)> callback);
    void cancelRequest(qint64 id);
    void cancelRequestsForUri(const QString &uri);
    void recordStderr(const QByteArray &chunk);
    void parseServerCapabilities(const QJsonObject &caps);
    QString unexpectedProcessExitMessage(const LspProcessResult &result) const;

    LspProcessTransport m_transport;
    LspRequestSender m_requestSender;
    LspDocumentProtocol m_documentProtocol;
    LspFeatureRequestRouter m_featureRequestRouter;
    LspServerMessageDispatcher m_serverMessageDispatcher;
    LspSessionLifecycle m_sessionLifecycle;
    QByteArray m_stderrPartial;
    QStringList m_stderrLines;

    LspServerCapabilities m_capabilities;
};

#endif
