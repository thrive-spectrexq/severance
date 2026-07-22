#include "SecurityView.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <cstdlib>
#include <vector>

namespace severance::gui::security_view {

KiersProtectionWidget::KiersProtectionWidget(QWidget* parent) : QWidget(parent) {
  setFixedSize(120, 120);
  m_animTimer = new QTimer(this);
  connect(m_animTimer, &QTimer::timeout, this, &KiersProtectionWidget::updateAnim);
  m_animTimer->start(50);
}

void KiersProtectionWidget::updateAnim() {
    m_rotation = (m_rotation + 2) % 360;
    update();
}

void KiersProtectionWidget::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  int side = qMin(width(), height());
  painter.translate(width() / 2, height() / 2);
  painter.rotate(m_rotation);

  QPen outerPen(QColor("#00E5FF"), 2);
  painter.setPen(outerPen);
  painter.drawArc(-side/2 + 10, -side/2 + 10, side - 20, side - 20, 0, 360 * 16);

  QPen dashPen(QColor("#1A7A5C"), 1.5, Qt::DashLine);
  painter.setPen(dashPen);
  painter.drawArc(-side/2 + 18, -side/2 + 18, side - 36, side - 36, -m_rotation * 16, 360 * 16);

  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor("#00E5FF"));
  painter.drawEllipse(-10, -10, 20, 20);
}

SecurityDashboardWidget::SecurityDashboardWidget(QWidget* parent) : QWidget(parent) {
  setFixedHeight(160);
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(24, 16, 24, 16);
  layout->addWidget(new KiersProtectionWidget(this));
  auto* titleLabel = new QLabel("SEVERANCE CHIP DIAGNOSTICS", this);
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
  layout->setSpacing(24);

  layout->addWidget(new SecurityDashboardWidget(this));

  m_ChipTable = new QTableWidget();
  m_ChipTable->setColumnCount(6);
  m_ChipTable->setHorizontalHeaderLabels({"Innie", "Chip ID", "Signal", "Memory Partition", "Containment", "Status"});
  m_ChipTable->horizontalHeader()->setStretchLastSection(true);
  m_ChipTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  m_ChipTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_ChipTable->verticalHeader()->setVisible(false);
  m_ChipTable->setShowGrid(false);
  
  m_ChipTable->setStyleSheet(R"(
    QTableWidget {
      background-color: #0A0F14;
      color: #E0FFFF;
      font-family: "Courier New", Courier, monospace;
      font-size: 13px;
      border: 1px solid #1A7A5C;
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
