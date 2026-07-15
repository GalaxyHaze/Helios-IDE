#ifndef LSPCLIENT_H
#define LSPCLIENT_H

#include <QByteArray>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <functional>

struct LspPosition { int line = 0; int character = 0; };
struct LspRange { LspPosition start; LspPosition end; };
struct LspTextChange { LspRange range; QString text; };
struct LspDiagnostic { LspRange range; int severity = 0; QString message; QString source; };

struct LspCompletionItem {
    QString label;
    int kind = 0;
    QString detail;
    QString insertText;
    int insertTextFormat = 1;
    QJsonObject rawItem;
};

struct LspLocation { QString uri; LspRange range; };
struct LspHoverInfo { QString contents; LspRange range; };
struct LspSignatureHelp { QString activeSignature; int activeParameter = 0; QStringList parameters; };

class LspClient : public QObject
{
    Q_OBJECT

public:
    explicit LspClient(QObject *parent = nullptr);
    ~LspClient() override;

    bool start(const QString &serverPath, const QString &stdlibPath = {}, const QString &workspaceRoot = {});
    void stop();
    bool isRunning() const;
    bool isReady() const;
    int documentSyncKind() const;
    int documentVersion(const QString &uri) const;

    bool hasCompletionProvider() const { return m_hasCompletionProvider; }
    bool hasHoverProvider() const { return m_hasHoverProvider; }
    bool hasDefinitionProvider() const { return m_hasDefinitionProvider; }
    bool hasImplementationProvider() const { return m_hasImplementationProvider; }
    bool hasDeclarationProvider() const { return m_hasDeclarationProvider; }
    bool hasReferencesProvider() const { return m_hasReferencesProvider; }
    bool hasDocumentHighlightProvider() const { return m_hasDocumentHighlightProvider; }
    bool hasRenameProvider() const { return m_hasRenameProvider; }
    bool hasDocumentSymbolProvider() const { return m_hasDocumentSymbolProvider; }
    bool hasFormattingProvider() const { return m_hasFormattingProvider; }
    bool hasFoldingRangeProvider() const { return m_hasFoldingRangeProvider; }
    bool hasCodeActionProvider() const { return m_hasCodeActionProvider; }
    bool hasSemanticTokensProvider() const { return m_hasSemanticTokensProvider; }

    void openDocument(const QString &uri, const QString &languageId, const QString &text, int version = 1);
    void changeDocument(const QString &uri, const QList<LspTextChange> &changes, int version);
    void changeDocumentFull(const QString &uri, const QString &fullText, int version);
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

signals:
    void initialized();
    void serverError(const QString &message);
    void serverStopped();
    void processStopped(bool expected);
    void diagnosticsReceived(const QString &uri, int version, const QList<LspDiagnostic> &diagnostics);
    void completionResults(const QString &uri, int version, const QList<LspCompletionItem> &items);
    void completionResolved(const QString &uri, int version, const LspCompletionItem &item);
    void hoverResult(const QString &uri, int version, const LspHoverInfo &info);
    void definitionResult(const QString &uri, int version, const LspLocation &location);
    void declarationResult(const QString &uri, int version, const LspLocation &location);
    void implementationResult(const QString &uri, int version, const LspLocation &location);
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

private slots:
    void onReadyRead();
    void onReadyReadError();
    void onProcessStarted();
    void onProcessError(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    static constexpr qsizetype MaxMessageSize = 64LL * 1024 * 1024;
    struct DocumentState { int version = 1; bool open = false; };
    struct PendingRequest {
        qint64 id = 0;
        QString method;
        QString uri;
        int version = -1;
        bool cancellable = false;
        std::function<void(const QJsonObject &)> callback;
        QTimer *timeout = nullptr;
    };

    void launch();
    void resetSessionState();
    bool sendMessage(const QJsonObject &msg);
    void handleResponse(const QJsonObject &msg);
    void handleNotification(const QJsonObject &msg);
    void processBuffer();
    qint64 nextId();
    qint64 sendRequest(const QString &method, const QJsonObject &params, const QString &uri, int version,
                       bool cancellable, std::function<void(const QJsonObject &)> callback);
    void cancelRequest(qint64 id);
    void cancelRequestsForUri(const QString &uri);
    void failPendingRequests(const QString &reason);
    void beginShutdown();
    void finishShutdownEscalation();
    void recordStderr(const QByteArray &chunk);
    void parseServerCapabilities(const QJsonObject &caps);
    QJsonObject positionParams(const QString &uri, const LspPosition &pos) const;
    static LspRange rangeFromJson(const QJsonObject &range);
    static LspLocation locationFromJson(const QJsonObject &value);
    static QList<LspLocation> locationsFromJson(const QJsonValue &value);

    QProcess *m_process = nullptr;
    QByteArray m_buffer;
    QByteArray m_stderrPartial;
    QStringList m_stderrLines;
    qint64 m_nextId = 0;
    bool m_initialized = false;
    bool m_shutdownRequested = false;
    bool m_exitSent = false;
    bool m_stopping = false;
    int m_syncKind = 1;
    QTimer *m_shutdownTimer = nullptr;
    QHash<QString, DocumentState> m_documents;
    QHash<qint64, PendingRequest> m_pendingRequests;
    QHash<QString, qint64> m_replaceableRequests;
    QString m_serverPath, m_stdlibPath, m_workspaceRoot;
    bool m_startPending = false;

    bool m_hasCompletionProvider = false, m_hasHoverProvider = false, m_hasDefinitionProvider = false;
    bool m_hasImplementationProvider = false, m_hasDeclarationProvider = false, m_hasReferencesProvider = false;
    bool m_hasDocumentHighlightProvider = false, m_hasRenameProvider = false, m_hasDocumentSymbolProvider = false;
    bool m_hasFormattingProvider = false, m_hasFoldingRangeProvider = false, m_hasCodeActionProvider = false;
    bool m_hasSemanticTokensProvider = false;
};

#endif
