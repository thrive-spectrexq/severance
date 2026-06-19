#include "AiEngine.hpp"
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QJsonParseError>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#include "core/metrics/WindowsMetricsProvider.hpp"
#endif

namespace severance::core::ai {

AiEngine::AiEngine() : m_NetworkManager(std::make_unique<QNetworkAccessManager>(this)) {
  m_EndpointUrl = "http://localhost:11434"; // default Ollama
}

AiEngine::~AiEngine() = default;

void AiEngine::initialize(const QString& endpointUrl) {
  if (!endpointUrl.isEmpty()) {
    m_EndpointUrl = endpointUrl;
  }
  spdlog::info("AiEngine initialized with endpoint: {}", m_EndpointUrl.toStdString());
}

void AiEngine::query(const QString& prompt, std::function<void(const QString&, bool)> callback) {
  QUrl url(m_EndpointUrl + "/api/generate");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["model"] = m_ModelName;
  json["prompt"] = prompt;
  json["stream"] = false;

  QNetworkReply* reply = m_NetworkManager->post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
    if (reply->error() == QNetworkReply::NoError) {
      QByteArray responseBytes = reply->readAll();
      QJsonDocument jsonDoc = QJsonDocument::fromJson(responseBytes);
      if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        QString responseText = jsonDoc.object()["response"].toString();
        callback(responseText, true);
      } else {
        callback("Failed to parse JSON response.", false);
      }
    } else {
      callback(reply->errorString(), false);
    }
    reply->deleteLater();
  });
}

QNetworkReply* AiEngine::queryStreaming(const QString& prompt) {
  QUrl url(m_EndpointUrl + "/api/generate");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["model"] = m_ModelName;
  json["prompt"] = prompt;
  json["stream"] = true;

  QNetworkReply* reply = m_NetworkManager->post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
    while (reply->canReadLine()) {
      QByteArray line = reply->readLine().trimmed();
      if (line.isEmpty()) continue;

      QJsonParseError error;
      QJsonDocument jsonDoc = QJsonDocument::fromJson(line, &error);
      
      if (error.error == QJsonParseError::NoError && jsonDoc.isObject()) {
        QString chunk = jsonDoc.object()["response"].toString();
        emit responseChunkReceived(chunk);
        
        bool done = jsonDoc.object()["done"].toBool();
        if (done) {
          emit responseFinished();
        }
      }
    }
  });

  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    if (reply->error() != QNetworkReply::NoError) {
      emit errorOccurred(reply->errorString());
    }
    reply->deleteLater();
  });

  return reply;
}

void AiEngine::analyzeProcess(int pid, const QString& processName, const QString& activityContext) {
  QString prompt = QString(
    "You are an expert system administrator and security analyst. "
    "Please analyze the following process behavior and tell me if it looks suspicious, "
    "and what it's likely doing in plain English.\n\n"
    "Process Name: %1\nPID: %2\nRecent Activity Context:\n%3\n\n"
    "Keep your explanation concise and formatting clean."
  ).arg(processName).arg(pid).arg(activityContext);

  queryStreaming(prompt);
}

void AiEngine::explainAnomaly(const QString& anomalyData) {
  QString prompt = QString(
    "Please explain the following system anomaly in simple terms and suggest "
    "possible causes or mitigations:\n\n%1"
  ).arg(anomalyData);

  queryStreaming(prompt);
}

void AiEngine::systemQuery(const QString& question) {
  QString context = "System Metrics Context:\n";
#ifdef _WIN32
  core::metrics::WindowsMetricsProvider provider;
  auto snapshot = provider.GetSnapshot();
  context += QString("CPU Usage: %1%\n").arg(snapshot.cpu.globalUsagePercent);
  context += QString("Memory Usage: %1 GB / %2 GB\n").arg(snapshot.memory.usedBytes / 1073741824.0).arg(snapshot.memory.totalBytes / 1073741824.0);
  context += QString("Network: %1 Kbps Recv, %2 Kbps Sent\n").arg(snapshot.network.totalBytesReceivedPerSec * 8.0 / 1000.0).arg(snapshot.network.totalBytesSentPerSec * 8.0 / 1000.0);
#else
  context += "Metrics not available on this platform.\n";
#endif

  QString prompt = QString(
    "You are Severance AI, a system monitoring assistant.\n"
    "Based on the following live system metrics, please answer the user's question.\n\n"
    "%1\n\n"
    "User Question: %2"
  ).arg(context).arg(question);

  queryStreaming(prompt);
}

} // namespace severance::core::ai
