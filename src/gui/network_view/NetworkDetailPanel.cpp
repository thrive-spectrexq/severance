#include "NetworkDetailPanel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHostInfo>
#include <QHBoxLayout>

namespace severance::gui::network_view {

NetworkDetailPanel::NetworkDetailPanel(QWidget* parent) : QWidget(parent) {
  setupUI();
}

NetworkDetailPanel::~NetworkDetailPanel() = default;

void NetworkDetailPanel::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(12);

  // Header
  auto* headerLayout = new QVBoxLayout();
  headerLayout->setSpacing(4);
  m_TitleLabel = new QLabel("Select a communication node", this);
  m_TitleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #E6EDF3;");
  
  m_SubtitleLabel = new QLabel("", this);
  m_SubtitleLabel->setStyleSheet("font-size: 13px; color: #8B949E;");

  headerLayout->addWidget(m_TitleLabel);
  headerLayout->addWidget(m_SubtitleLabel);
  layout->addLayout(headerLayout);

  // Tabs
  m_Tabs = new QTabWidget(this);
  
  // Overview Tab
  auto* overviewTab = new QWidget(this);
  auto* overviewLayout = new QVBoxLayout(overviewTab);
  m_OverviewContent = new QTextEdit(this);
  m_OverviewContent->setReadOnly(true);
  m_OverviewContent->setStyleSheet("background: transparent; border: none; color: #F1F5F9; font-size: 13px;");
  overviewLayout->addWidget(m_OverviewContent);
  m_Tabs->addTab(overviewTab, "OVERVIEW");

  // Traffic Graph Tab (Placeholder)
  auto* graphTab = new QWidget(this);
  auto* graphLayout = new QVBoxLayout(graphTab);
  m_GraphPlaceholder = new QLabel("[Historical Traffic Graph Placeholder]\nWill display bytes/sec over time.", this);
  m_GraphPlaceholder->setAlignment(Qt::AlignCenter);
  m_GraphPlaceholder->setStyleSheet("color: #64748B; font-style: italic;");
  graphLayout->addWidget(m_GraphPlaceholder);
  m_Tabs->addTab(graphTab, "TRAFFIC HISTORY");

  // DNS Tab (Placeholder)
  auto* dnsTab = new QWidget(this);
  auto* dnsLayout = new QVBoxLayout(dnsTab);
  m_DnsContent = new QTextEdit(this);
  m_DnsContent->setReadOnly(true);
  m_DnsContent->setStyleSheet("background: transparent; border: none; color: #94A3B8; font-family: monospace;");
  dnsLayout->addWidget(m_DnsContent);
  m_Tabs->addTab(dnsTab, "DNS RESOLUTION");

  // DPI Tab (Placeholder)
  auto* dpiTab = new QWidget(this);
  auto* dpiLayout = new QVBoxLayout(dpiTab);
  auto* dpiLbl = new QLabel("[Deep Packet Inspection Placeholder]\nRequires NDIS/WFP driver integration.", this);
  dpiLbl->setAlignment(Qt::AlignCenter);
  dpiLbl->setStyleSheet("color: #64748B; font-style: italic;");
  dpiLayout->addWidget(dpiLbl);
  m_Tabs->addTab(dpiTab, "DEEP PACKET SURVEILLANCE");

  layout->addWidget(m_Tabs);
}

void NetworkDetailPanel::Clear() {
  m_TitleLabel->setText("Select a communication node");
  m_SubtitleLabel->setText("");
  m_OverviewContent->clear();
  m_DnsContent->clear();
}

void NetworkDetailPanel::LoadConnection(uint32_t pid, const QString& procName, const QString& localIp, uint16_t localPort, const QString& remoteIp, uint16_t remotePort, const QString& state) {
  m_TitleLabel->setText(procName.isEmpty() ? "Unidentified Procedure" : procName);
  m_SubtitleLabel->setText(QString("ID: %1 | Status: %2").arg(pid).arg(state));

  QString overviewHtml = QString(R"(
    <h3>Node Particulars</h3>
    <table cellpadding="4">
      <tr><td style="color:#8B949E;">Procedure Designation:</td><td>%1</td></tr>
      <tr><td style="color:#8B949E;">ID:</td><td>%2</td></tr>
      <tr><td style="color:#8B949E;">Internal Address:</td><td>%3:%4</td></tr>
      <tr><td style="color:#8B949E;">External Address:</td><td>%5:%6</td></tr>
      <tr><td style="color:#8B949E;">Status:</td><td>%7</td></tr>
    </table>
    <br/>
    <p style="color:#8B949E;"><i>Procedure attribution utilizes active ETW ledgers. Reverse DNS resolution is pending.</i></p>
  )").arg(procName).arg(pid).arg(localIp).arg(localPort).arg(remoteIp).arg(remotePort).arg(state);

  m_OverviewContent->setHtml(overviewHtml);

  m_DnsContent->setPlainText(QString("Resolving hostname for %1...").arg(remoteIp));
  
  // Async DNS Resolution
  QHostInfo::lookupHost(remoteIp, this, [this, remoteIp](const QHostInfo& host) {
    if (host.error() != QHostInfo::NoError) {
      m_DnsContent->setPlainText(QString("Reverse DNS lookup failed for %1:\n%2").arg(remoteIp).arg(host.errorString()));
    } else {
      m_DnsContent->setPlainText(QString("Hostname resolved for %1:\n%2").arg(remoteIp).arg(host.hostName()));
    }
  });
}

} // namespace severance::gui::network_view
