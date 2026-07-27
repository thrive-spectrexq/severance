#include "SecurityView.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QDialog>
#include <QMessageBox>
#include <cstdlib>
#include <vector>

namespace severance::gui::security_view {

KiersProtectionWidget::KiersProtectionWidget(QWidget* parent) : QWidget(parent) {
  setFixedSize(120, 120);
  m_animTimer = new QTimer(this);
  connect(m_animTimer, &QTimer::timeout, this, &KiersProtectionWidget::updateAnim);
  m_animTimer->start(50);
}

void KiersProtectionWidget::triggerBoost() {
  m_speed = 10;
  QTimer::singleShot(2500, this, [this]() { m_speed = 2; });
}

void KiersProtectionWidget::updateAnim() {
    m_rotation = (m_rotation + m_speed) % 360;
    update();
}

void KiersProtectionWidget::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  int side = qMin(width(), height());
  painter.translate(width() / 2, height() / 2);
  painter.rotate(m_rotation);

  QPen outerPen(m_speed > 2 ? QColor("#39FF14") : QColor("#00E5FF"), 2);
  painter.setPen(outerPen);
  painter.drawArc(-side/2 + 10, -side/2 + 10, side - 20, side - 20, 0, 360 * 16);

  QPen dashPen(QColor("#1A7A5C"), 1.5, Qt::DashLine);
  painter.setPen(dashPen);
  painter.drawArc(-side/2 + 18, -side/2 + 18, side - 36, side - 36, -m_rotation * 16, 360 * 16);

  painter.setPen(Qt::NoPen);
  painter.setBrush(m_speed > 2 ? QColor("#39FF14") : QColor("#00E5FF"));
  painter.drawEllipse(-10, -10, 20, 20);
}

SecurityDashboardWidget::SecurityDashboardWidget(QWidget* parent) : QWidget(parent) {
  setFixedHeight(160);
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(24, 16, 24, 16);
  
  m_ProtectionWidget = new KiersProtectionWidget(this);
  layout->addWidget(m_ProtectionWidget);
  
  auto* titleLabel = new QLabel("SEVERANCE CHIP DIAGNOSTICS & VIGILANCE", this);
  titleLabel->setStyleSheet("color: #00E5FF; font-family: 'Courier New', Courier, monospace; font-weight: bold; font-size: 18px; letter-spacing: 2px;");
  layout->addWidget(titleLabel, 1, Qt::AlignCenter);
}

SecurityView::SecurityView(QWidget* parent) : QWidget(parent) {
  setupUI();
  m_UiTimer = new QTimer(this);
  connect(m_UiTimer, &QTimer::timeout, this, &SecurityView::updateTable);
  m_UiTimer->start(1000); 
  updateTable();
}

SecurityView::~SecurityView() = default;

void SecurityView::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(20);

  m_DashboardWidget = new SecurityDashboardWidget(this);
  layout->addWidget(m_DashboardWidget);

  auto* controlBar = new QHBoxLayout();
  m_RecalibrateBtn = new QPushButton("[ RECALIBRATE CHIP SIGNALS ]", this);
  m_RecalibrateBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #161B22;
      color: #39FF14;
      border: 1px solid #30363D;
      padding: 8px 16px;
      font-family: monospace;
      font-weight: bold;
    }
    QPushButton:hover {
      background-color: #39FF14;
      color: #0D1117;
    }
  )");
  connect(m_RecalibrateBtn, &QPushButton::clicked, this, &SecurityView::onRecalibrateClicked);
  controlBar->addWidget(m_RecalibrateBtn);
  controlBar->addStretch();
  layout->addLayout(controlBar);

  m_ChipTable = new QTableWidget();
  m_ChipTable->setColumnCount(6);
  m_ChipTable->setHorizontalHeaderLabels({"Innie", "Chip ID", "Signal", "Memory Partition", "Containment", "Status"});
  m_ChipTable->horizontalHeader()->setStretchLastSection(true);
  m_ChipTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  m_ChipTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_ChipTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_ChipTable->verticalHeader()->setVisible(false);
  m_ChipTable->setShowGrid(false);
  
  connect(m_ChipTable, &QTableWidget::cellDoubleClicked, this, &SecurityView::onCellDoubleClicked);

  m_ChipTable->setStyleSheet(R"(
    QTableWidget {
      background-color: #0A0F14;
      color: #E0FFFF;
      font-family: "Courier New", Courier, monospace;
      font-size: 13px;
      border: 1px solid #1A7A5C;
    }
    QTableWidget::item:selected {
      background-color: #1A7A5C;
      color: #FFFFFF;
    }
    QHeaderView::section {
      background-color: #0A0F14;
      color: #00E5FF;
      font-weight: bold;
      border: 1px solid #1A7A5C;
    }
  )");

  layout->addWidget(m_ChipTable);
}

void SecurityView::onRecalibrateClicked() {
  if (m_DashboardWidget && m_DashboardWidget->protectionWidget()) {
    m_DashboardWidget->protectionWidget()->triggerBoost();
  }
  updateTable();
  QMessageBox::information(this, "Chip Signal Recalibration", 
                           "All Severance Chip RF frequencies recalibrated to Lumon standard. Partition containment verified.");
}

void SecurityView::onCellDoubleClicked(int row, int column) {
  Q_UNUSED(column);
  if (row < 0 || row >= m_ChipTable->rowCount()) {
    return;
  }

  QString innie = m_ChipTable->item(row, 0)->text();
  QString chipId = m_ChipTable->item(row, 1)->text();
  QString signal = m_ChipTable->item(row, 2)->text();
  QString partition = m_ChipTable->item(row, 3)->text();
  QString containment = m_ChipTable->item(row, 4)->text();
  QString status = m_ChipTable->item(row, 5)->text();

  QDialog dlg(this);
  dlg.setWindowTitle(QString("Chip Diagnostics — %1").arg(innie));
  dlg.resize(450, 320);
  dlg.setStyleSheet(R"(
    QDialog { background-color: #050B09; color: #20F8D5; font-family: 'Courier New', Consolas, monospace; }
    QLabel { font-size: 13px; margin: 4px 0; }
    QPushButton { background-color: #208A7C; color: #050B09; border: none; padding: 8px 16px; font-weight: bold; border-radius: 3px; }
    QPushButton:hover { background-color: #20F8D5; }
  )");

  auto* layout = new QVBoxLayout(&dlg);
  
  auto* title = new QLabel(QString("SEVERANCE CHIP PARAMETERS: %1").arg(innie), &dlg);
  title->setStyleSheet("font-size: 16px; font-weight: bold; color: #00E5FF; margin-bottom: 10px;");
  layout->addWidget(title);

  layout->addWidget(new QLabel(QString("Chip Identifier: %1").arg(chipId), &dlg));
  layout->addWidget(new QLabel(QString("Transceiver Signal: %1").arg(signal), &dlg));
  layout->addWidget(new QLabel(QString("Spatial Memory Partition: %1").arg(partition), &dlg));
  layout->addWidget(new QLabel(QString("Containment Compliance: %1").arg(containment), &dlg));
  layout->addWidget(new QLabel(QString("Diagnostic Status: %1").arg(status), &dlg));
  layout->addWidget(new QLabel("Firmware Version: Lumon-ChipOS v9.4.2-rel", &dlg));

  layout->addSpacing(15);

  auto* closeBtn = new QPushButton("CLOSE DIAGNOSTICS", &dlg);
  connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
  layout->addWidget(closeBtn, 0, Qt::AlignCenter);

  dlg.exec();
}

void SecurityView::updateTable() {
  m_ChipTable->setUpdatesEnabled(false);
  m_ChipTable->setRowCount(0);

  struct ChipData {
      QString innie, id, part, status;
      float baseSignal, baseCont;
  };
  
  static std::vector<ChipData> chips = {
      {"Mark S.", "SC-0421-A7", "Intact", "NOMINAL", 98.2f, 100.0f},
      {"Helly R.", "SC-0522-B3", "Degraded", "WARNING", 87.4f, 94.0f},
      {"Irving B.", "SC-0319-C1", "Intact", "NOMINAL", 99.8f, 100.0f},
      {"Dylan G.", "SC-0418-D5", "Intact", "NOMINAL", 95.1f, 99.0f}
  };

  for (const auto& chip : chips) {
    int row = m_ChipTable->rowCount();
    m_ChipTable->insertRow(row);
    
    float sig = chip.baseSignal + (std::rand() % 20 - 10) / 10.0f;
    if (sig > 100.0f) sig = 100.0f;
    float cont = chip.baseCont + (std::rand() % 10 - 5) / 10.0f;
    if (cont > 100.0f) cont = 100.0f;

    m_ChipTable->setItem(row, 0, new QTableWidgetItem(chip.innie));
    m_ChipTable->setItem(row, 1, new QTableWidgetItem(chip.id));
    m_ChipTable->setItem(row, 2, new QTableWidgetItem(QString::number(sig, 'f', 1) + "%"));
    m_ChipTable->setItem(row, 3, new QTableWidgetItem(chip.part));
    m_ChipTable->setItem(row, 4, new QTableWidgetItem(QString::number(cont, 'f', 1) + "%"));
    
    auto* statusItem = new QTableWidgetItem(chip.status);
    if (chip.status == "WARNING") {
        statusItem->setForeground(QColor("#DA3633")); // red-ish
    } else {
        statusItem->setForeground(QColor("#3FB950")); // green-ish
    }
    m_ChipTable->setItem(row, 5, statusItem);
  }
  m_ChipTable->setUpdatesEnabled(true);
}

} // namespace severance::gui::security_view
