#include "SecurityView.hpp"
#include "gui/theme/Theme.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QHeaderView>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QPainterPath>
#include <cstdlib>

namespace severance::gui::security_view {

KiersProtectionWidget::KiersProtectionWidget(QWidget* parent) : QWidget(parent) {
  setFixedSize(120, 120);
  auto* anim = new QPropertyAnimation(this, "rotation", this);
  anim->setDuration(6000);
  anim->setStartValue(0);
  anim->setEndValue(360);
  anim->setLoopCount(-1);
  anim->start();

  auto* shadow = new QGraphicsDropShadowEffect(this);
  shadow->setBlurRadius(25);
  shadow->setColor(QColor(theme::Colors::AccentGlow));
  shadow->setOffset(0, 0);
  setGraphicsEffect(shadow);
}

void KiersProtectionWidget::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  int side = qMin(width(), height());
  painter.translate(width() / 2, height() / 2);
  painter.rotate(m_rotation);

  // Outer ring
  QPen outerPen(QColor(theme::Colors::ChartTeal), 2);
  painter.setPen(outerPen);
  painter.drawArc(-side/2 + 10, -side/2 + 10, side - 20, side - 20, 0, 360 * 16);

  // Dash ring
  QPen dashPen(QColor(theme::Colors::TextPrimary), 1.5, Qt::DashLine);
  painter.setPen(dashPen);
  painter.drawArc(-side/2 + 18, -side/2 + 18, side - 36, side - 36, -m_rotation * 16, 360 * 16);

  // Kier's Shield / Drop motif
  painter.setPen(Qt::NoPen);
  QColor shieldColor = QColor(theme::Colors::ChartGlow);
  shieldColor.setAlpha(200);
  painter.setBrush(shieldColor);

  QPainterPath path;
  path.moveTo(0, -30);
  path.lineTo(25, -10);
  path.lineTo(15, 25);
  path.lineTo(0, 35);
  path.lineTo(-15, 25);
  path.lineTo(-25, -10);
  path.closeSubpath();
  painter.drawPath(path);

  // Inner detail (lock hole)
  painter.setBrush(QColor(theme::Colors::BgPrimary));
  painter.drawEllipse(QPoint(0, -2), 6, 6);
  painter.drawRect(-3, -2, 6, 14);
}

SecurityDashboardWidget::SecurityDashboardWidget(QWidget* parent) : QWidget(parent) {
  setFixedHeight(160);
  
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(24, 16, 24, 16);
  layout->setSpacing(32);

  // Shield on the left
  m_shieldWidget = new KiersProtectionWidget(this);
  layout->addWidget(m_shieldWidget);

  // Text title
  auto* rightLayout = new QVBoxLayout();
  layout->addLayout(rightLayout, 1);

  auto* titleLabel = new QLabel("VIGILANCE MATRIX — KIER'S OMNIPRESENT GAZE", this);
  titleLabel->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 14px; letter-spacing: 3px;").arg(theme::Colors::ChartGlow));
  rightLayout->addWidget(titleLabel);
  rightLayout->addStretch();
  
  m_animTimer = new QTimer(this);
  connect(m_animTimer, &QTimer::timeout, this, [this]() {
      // Fluctuating detector value
      int currentCode = m_codeDetectorValue;
      currentCode += (rand() % 15) - 7;
      if (currentCode < 0) currentCode = 0;
      if (currentCode > 100) currentCode = 100;
      setCodeDetectorValue(currentCode);

      // Firewall occasionally drops then recovers
      int fw = m_firewallValue;
      if (rand() % 100 < 8) {
          fw -= (rand() % 20);
      } else {
          fw += 3;
      }
      if (fw < 0) fw = 0;
      if (fw > 100) fw = 100;
      setFirewallValue(fw);
  });
  m_animTimer->start(200);

  m_codeDetectorValue = 0;
  m_firewallValue = 100;
}

void SecurityDashboardWidget::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // Draw a clinical background frame
  QRect r = rect();
  painter.fillRect(r, QColor(theme::Colors::BgTertiary));

  QPen borderPen(QColor(theme::Colors::Border), 2);
  painter.setPen(borderPen);
  painter.drawRect(r.adjusted(1, 1, -1, -1));

  // Draw horizontal bars for the detectors
  int startX = m_shieldWidget->width() + 56;
  int startY = 60;
  int barWidth = width() - startX - 32;
  int barHeight = 12;

  painter.setPen(QColor(theme::Colors::TextSecondary));
  painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
  painter.drawText(startX, startY - 8, QString("MACRODATA CODE DETECTOR FLUX [%1%]").arg(m_codeDetectorValue));

  painter.fillRect(startX, startY, barWidth, barHeight, QColor(theme::Colors::BgPrimary));
  QColor codeColor = m_codeDetectorValue > 80 ? QColor(theme::Colors::ChartAmber) : QColor(theme::Colors::ChartTeal);
  painter.fillRect(startX, startY, barWidth * m_codeDetectorValue / 100.0, barHeight, codeColor);

  painter.setPen(QColor(theme::Colors::BgSecondary));
  for (int i = 1; i < 20; ++i) {
      int x = startX + (barWidth * i / 20);
      painter.drawLine(x, startY, x, startY + barHeight);
  }

  // Firewall Status
  startY += 50;
  painter.setPen(QColor(theme::Colors::TextSecondary));
  painter.drawText(startX, startY - 8, QString("FIREWALL INTEGRITY [%1%]").arg(m_firewallValue));

  painter.fillRect(startX, startY, barWidth, barHeight, QColor(theme::Colors::BgPrimary));
  QColor fwColor = m_firewallValue < 40 ? QColor(theme::Colors::Error) : QColor(theme::Colors::Success);
  painter.fillRect(startX, startY, barWidth * m_firewallValue / 100.0, barHeight, fwColor);

  painter.setPen(QColor(theme::Colors::BgSecondary));
  for (int i = 1; i < 20; ++i) {
      int x = startX + (barWidth * i / 20);
      painter.drawLine(x, startY, x, startY + barHeight);
  }
}


SecurityView::SecurityView(QWidget* parent) : QWidget(parent) {
  setupUI();

  // Register callbacks to receive data
  core::security::FimManager::GetInstance().RegisterCallback(
    [this](const core::security::FimEvent& ev) { this->onFimEvent(ev); });

  core::security::EventLogManager::GetInstance().RegisterCallback(
    [this](const core::security::WindowsEvent& ev) { this->onLogEvent(ev); });

  m_UiTimer = new QTimer(this);
  connect(m_UiTimer, &QTimer::timeout, this, &SecurityView::updateTables);
  m_UiTimer->start(500); // UI update rate
}

SecurityView::~SecurityView() = default;



void SecurityView::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(24);

  // Top Dashboard
  m_Dashboard = new SecurityDashboardWidget(this);
  layout->addWidget(m_Dashboard);

  auto* tabs = new QTabWidget(this);
  tabs->setStyleSheet(QString(
    "QTabBar::tab { background: %1; color: %2; padding: 8px 16px; border: 1px solid %3; }"
    "QTabBar::tab:selected { background: %4; color: %5; border-bottom-color: %4; font-weight: bold; }"
    "QTabWidget::pane { border: 1px solid %3; background: %1; }"
  ).arg(theme::Colors::BgSecondary, theme::Colors::TextSecondary, theme::Colors::Border, theme::Colors::BgHover, theme::Colors::TextPrimary));

  // FIM Section (Document Surveillance)
  m_FimTable = new QTableWidget();
  m_FimTable->setColumnCount(4);
  m_FimTable->setHorizontalHeaderLabels({"ACTION", "DESIGNATION", "PRIOR DESIGNATION", "SHA-256 HASH"});
  m_FimTable->horizontalHeader()->setStretchLastSection(true);
  m_FimTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_FimTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_FimTable->verticalHeader()->setVisible(false);
  m_FimTable->setShowGrid(false);
  m_FimTable->setStyleSheet(QString("background-color: %1; color: %2;").arg(theme::Colors::BgPrimary, theme::Colors::TextPrimary));
  tabs->addTab(m_FimTable, "DOCUMENT SURVEILLANCE");

  // Event Log Section (Vigilance Event Ledger)
  m_LogTable = new QTableWidget();
  m_LogTable->setColumnCount(1);
  m_LogTable->setHorizontalHeaderLabels({"RAW XML DATA"});
  m_LogTable->horizontalHeader()->setStretchLastSection(true);
  m_LogTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_LogTable->verticalHeader()->setVisible(false);
  m_LogTable->setShowGrid(false);
  m_LogTable->setStyleSheet(QString("background-color: %1; color: %2;").arg(theme::Colors::BgPrimary, theme::Colors::TextPrimary));
  tabs->addTab(m_LogTable, "VIGILANCE EVENT LEDGER");

  // Anomalies Section
  m_AnomaliesTable = new QTableWidget();
  m_AnomaliesTable->setColumnCount(3);
  m_AnomaliesTable->setHorizontalHeaderLabels({"INNIE PID", "DESIGNATION", "STATE"});
  m_AnomaliesTable->horizontalHeader()->setStretchLastSection(true);
  m_AnomaliesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_AnomaliesTable->verticalHeader()->setVisible(false);
  m_AnomaliesTable->setShowGrid(false);
  m_AnomaliesTable->setStyleSheet(QString("background-color: %1; color: %2;").arg(theme::Colors::BgPrimary, theme::Colors::TextPrimary));
  tabs->addTab(m_AnomaliesTable, "MACRODATA ANOMALIES");

  // Directives Section
  m_DirectivesTable = new QTableWidget();
  m_DirectivesTable->setColumnCount(3);
  m_DirectivesTable->setHorizontalHeaderLabels({"DIRECTIVE NAME", "CONDITION", "ACTION"});
  m_DirectivesTable->horizontalHeader()->setStretchLastSection(true);
  m_DirectivesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_DirectivesTable->verticalHeader()->setVisible(false);
  m_DirectivesTable->setShowGrid(false);
  m_DirectivesTable->setStyleSheet(QString("background-color: %1; color: %2;").arg(theme::Colors::BgPrimary, theme::Colors::TextPrimary));
  tabs->addTab(m_DirectivesTable, "CONTAINMENT DIRECTIVES");

  layout->addWidget(tabs);
}

void SecurityView::onFimEvent(const core::security::FimEvent& ev) {
  QMutexLocker lock(&m_QueueMutex);
  m_PendingFimEvents.push_back(ev);
}

void SecurityView::onLogEvent(const core::security::WindowsEvent& ev) {
  // VERY basic filtering for now
  // We check the XML for string indicators of high severity
  if (ev.xmlData.find("Level>2</Level>") != std::string::npos || // Error
      ev.xmlData.find("Level>1</Level>") != std::string::npos || // Critical
      ev.xmlData.find("Level>3</Level>") != std::string::npos || // Warning
      ev.xmlData.find("EventID>4624</EventID>") != std::string::npos || // Logon
      ev.xmlData.find("EventID>4625</EventID>") != std::string::npos) { // Failed Logon

    QMutexLocker lock(&m_QueueMutex);
    m_PendingLogEvents.push_back(ev);
  }
}

void SecurityView::updateTables() {
  std::vector<core::security::FimEvent> fimBatch;
  std::vector<core::security::WindowsEvent> logBatch;

  {
    QMutexLocker lock(&m_QueueMutex);
    fimBatch.swap(m_PendingFimEvents);
    logBatch.swap(m_PendingLogEvents);
  }

  // Update FIM Table
  if (!fimBatch.empty()) {
    m_FimTable->setUpdatesEnabled(false);
    for (const auto& ev : fimBatch) {
      int row = 0; // Insert at top
      m_FimTable->insertRow(row);
      
      QString actionStr;
      switch (ev.action) {
        case core::security::FimEvent::Action::Added: actionStr = "Added"; break;
        case core::security::FimEvent::Action::Modified: actionStr = "Modified"; break;
        case core::security::FimEvent::Action::Removed: actionStr = "Removed"; break;
        case core::security::FimEvent::Action::Renamed: actionStr = "Renamed"; break;
      }

      m_FimTable->setItem(row, 0, new QTableWidgetItem(actionStr));
      m_FimTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(ev.filePath)));
      m_FimTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(ev.oldFilePath)));
      m_FimTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(ev.fileHash)));
    }
    // Limit to 1000 rows
    while (m_FimTable->rowCount() > 1000) m_FimTable->removeRow(1000);
    m_FimTable->setUpdatesEnabled(true);
  }

  // Update Log Table
  if (!logBatch.empty()) {
    m_LogTable->setUpdatesEnabled(false);
    for (const auto& ev : logBatch) {
      int row = 0;
      m_LogTable->insertRow(row);
      
      m_LogTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(ev.xmlData)));
    }
    // Limit to 1000 rows
    while (m_LogTable->rowCount() > 1000) m_LogTable->removeRow(1000);
    m_LogTable->setUpdatesEnabled(true);
  }

  // Update Directives Table
  auto directives = core::security::ActiveResponse::GetInstance().GetDirectives();
  m_DirectivesTable->setUpdatesEnabled(false);
  m_DirectivesTable->setRowCount(0);
  for (const auto& dir : directives) {
    int row = m_DirectivesTable->rowCount();
    m_DirectivesTable->insertRow(row);
    m_DirectivesTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(dir.name)));
    m_DirectivesTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(dir.condition)));
    m_DirectivesTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(dir.action)));
  }
  m_DirectivesTable->setUpdatesEnabled(true);

  // Update Anomalies Table
  auto anomalies = core::correlation::CorrelationEngine::GetInstance().GetAllProfiles();
  m_AnomaliesTable->setUpdatesEnabled(false);
  m_AnomaliesTable->setRowCount(0);
  for (const auto& prof : anomalies) {
    if (prof.state != core::correlation::InnieState::Idle) {
      int row = m_AnomaliesTable->rowCount();
      m_AnomaliesTable->insertRow(row);
      m_AnomaliesTable->setItem(row, 0, new QTableWidgetItem(QString::number(prof.pid)));
      m_AnomaliesTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(prof.name)));
      
      QString stateStr;
      switch (prof.state) {
        case core::correlation::InnieState::Macrodata_Refinement_Detected: stateStr = "MACRODATA REFINEMENT DETECTED"; break;
        case core::correlation::InnieState::Kier_Alert_Triggered: stateStr = "KIER ALERT TRIGGERED"; break;
        default: stateStr = "IDLE"; break;
      }
      
      auto* item = new QTableWidgetItem(stateStr);
      if (prof.state == core::correlation::InnieState::Kier_Alert_Triggered) {
          item->setForeground(QColor(theme::Colors::Error));
      } else {
          item->setForeground(QColor(theme::Colors::Warning));
      }
      m_AnomaliesTable->setItem(row, 2, item);
    }
  }
  m_AnomaliesTable->setUpdatesEnabled(true);
}

} // namespace severance::gui::security_view
