#ifndef LSPCLIENT_H
#define LSPCLIENT_H

#include <QByteArray>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QUrl>
#include <functional>

struct LspPosition {
  int line = 0;
  int character = 0;
};

struct LspRange {
  LspPosition start;
  LspPosition end;
};

struct LspTextChange {
  LspRange range;
  QString text;
};

struct LspDiagnostic {
  LspRange range;
  int severity = 0;
  QString message;
  QString source;
};

struct LspCompletionItem {
  QString label;
  int kind = 0;
  QString detail;
  QString insertText;
  int insertTextFormat = 1;
};

struct LspLocation {
  QString uri;
  LspRange range;
};

struct LspHoverInfo {
  QString contents;
  LspRange range;
};

struct LspSignatureHelp {
  QString activeSignature;
  int activeParameter = 0;
  QStringList parameters;
};

class LspClient : public QObject {
  Q_OBJECT

public:
  explicit LspClient(QObject *parent = nullptr);
  ~LspClient();

  bool start(const QString &serverPath, const QString &stdlibPath = {}, const QString &workspaceRoot = {});
  void stop();
  bool isRunning() const;

  void openDocument(const QString &uri, const QString &languageId,
                    const QString &text);
  void changeDocument(const QString &uri, const QList<LspTextChange> &changes,
                      int version);
  void closeDocument(const QString &uri);
  void saveDocument(const QString &uri);

  void requestCompletion(const QString &uri, const LspPosition &pos);
  void requestHover(const QString &uri, const LspPosition &pos);
  void requestDefinition(const QString &uri, const LspPosition &pos);
  void requestSignatureHelp(const QString &uri, const LspPosition &pos);
  void requestSemanticTokens(const QString &uri);
  void requestFormatting(const QString &uri);
  void requestDocumentSymbols(const QString &uri);

signals:
  void initialized();
  void serverError(const QString &message);
  void diagnosticsReceived(const QString &uri,
                           const QList<LspDiagnostic> &diagnostics);
  void completionResults(const QList<LspCompletionItem> &items);
  void hoverResult(const LspHoverInfo &info);
  void definitionResult(const LspLocation &location);
  void signatureHelpResult(const LspSignatureHelp &help);
  void semanticTokensResult(const QJsonArray &tokens);
  void formattingResult(const QList<QPair<LspRange, QString>> &edits);
  void documentSymbolsResult(const QJsonArray &symbols);
  void logMessage(const QString &message);

private slots:
  void onReadyRead();
  void onProcessError(QProcess::ProcessError error);
  void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
  void sendMessage(const QJsonObject &msg);
  void handleResponse(const QJsonObject &msg);
  void handleNotification(const QJsonObject &msg);
  void processBuffer();

  int nextId();
  void sendRequest(const QString &method, const QJsonObject &params,
                   std::function<void(const QJsonObject &)> callback = nullptr);

  QProcess *m_process = nullptr;
  QByteArray m_buffer;
  int m_nextId = 0;
  bool m_initialized = false;
  bool m_shutdownRequested = false;

  struct PendingRequest {
    std::function<void(const QJsonObject &)> callback;
  };
  QHash<int, PendingRequest> m_pendingRequests;
};

#endif // LSPCLIENT_H
