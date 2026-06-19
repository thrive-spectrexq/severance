#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <memory>
#include <functional>

namespace severance::core::ai {

class AiEngine : public QObject {
  Q_OBJECT

public:
  static AiEngine& GetInstance() {
    static AiEngine instance;
    return instance;
  }

  void initialize(const QString& endpointUrl = "http://localhost:11434");

  // Single response via callback
  void query(const QString& prompt, std::function<void(const QString&, bool success)> callback);

  // Streaming response
  // Returns a QNetworkReply that the caller can connect readyRead() to.
  // Using Ollama's /api/generate with stream=true
  QNetworkReply* queryStreaming(const QString& prompt);

  // High level contextual queries
  void systemQuery(const QString& question);
  void analyzeProcess(int pid, const QString& processName, const QString& activityContext);
  void explainAnomaly(const QString& anomalyData);

signals:
  // Emitted when a streaming chunk arrives
  void responseChunkReceived(const QString& chunk);
  // Emitted when generation finishes
  void responseFinished();
  // Emitted on error
  void errorOccurred(const QString& errorMsg);

private:
  AiEngine();
  ~AiEngine() override;

  QString m_EndpointUrl;
  QString m_ModelName{"llama3"}; // Default model
  std::unique_ptr<QNetworkAccessManager> m_NetworkManager;
};

} // namespace severance::core::ai
