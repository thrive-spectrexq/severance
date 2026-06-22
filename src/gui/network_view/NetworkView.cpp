#include "NetworkView.hpp"
#include "core/network/NetworkManager.hpp"
#include "core/process/ProcessManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>

namespace severance::gui::network_view {

using namespace core::network;

NetworkView::NetworkView(QWidget* parent) : QWidget(parent) {
  setupUI();

  m_RefreshTimer = new QTimer(this);
  connect(m_RefreshTimer, &QTimer::timeout, this, &NetworkView::Refresh);
  m_RefreshTimer->start(1000);

  Refresh();
}

NetworkView::~NetworkView() = default;

void NetworkView::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(12);

  // Top Bar
  auto* topBar = new QHBoxLayout();
  m_SearchBox = new QLineEdit(this);
  m_SearchBox->setPlaceholderText("Filter connections by IP, Port, or Process...");
  m_SearchBox->setMinimumWidth(300);
  connect(m_SearchBox, &QLineEdit::textChanged, this, &NetworkView::onSearchTextChanged);
  topBar->addWidget(m_SearchBox);
  topBar->addStretch();
  layout->addLayout(topBar);

  // Table
  m_Table = new QTableWidget(this);
  m_Table->setColumnCount(7);
  m_Table->setHorizontalHeaderLabels({"Process", "PID", "Protocol", "Local Address", "Local Port", "Remote Address", "Remote Port", "State"});
  m_Table->horizontalHeader()->setStretchLastSection(true);
  m_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Table->verticalHeader()->setVisible(false);
  m_Table->setShowGrid(false);

  m_Table->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_Table, &QTableWidget::customContextMenuRequested, this, &NetworkView::onContextMenuRequested);

  layout->addWidget(m_Table);
}

QString NetworkView::formatState(int stateEnum) {
  switch(static_cast<ConnectionState>(stateEnum)) {
    case ConnectionState::CLOSED: return "CLOSED";
    case ConnectionState::LISTEN: return "LISTEN";
    case ConnectionState::SYN_SENT: return "SYN_SENT";
    case ConnectionState::SYN_RCVD: return "SYN_RCVD";
    case ConnectionState::ESTABLISHED: return "ESTABLISHED";
    case ConnectionState::FIN_WAIT1: return "FIN_WAIT1";
    case ConnectionState::FIN_WAIT2: return "FIN_WAIT2";
    case ConnectionState::CLOSE_WAIT: return "CLOSE_WAIT";
    case ConnectionState::CLOSING: return "CLOSING";
    case ConnectionState::LAST_ACK: return "LAST_ACK";
    case ConnectionState::TIME_WAIT: return "TIME_WAIT";
    case ConnectionState::DELETE_TCB: return "DELETE_TCB";
    default: return "";
  }
}

void NetworkView::Refresh() {
  auto connections = NetworkManager::GetInstance().GetActiveConnections();
  QString filter = m_SearchBox->text().toLower();

  // Suspend rendering
  m_Table->setUpdatesEnabled(false);
  m_Table->setRowCount(0);

  // Fetch processes once to avoid querying the OS hundreds of times
  core::process::ProcessManager procMgr;
  auto procList = procMgr.GetRunningProcesses();

  int row = 0;
  for (const auto& conn : connections) {
    // Basic Process Name correlation
    QString procName = "Unknown";
    for (const auto& p : procList) {
      if (p.pid == conn.pid) {
        procName = QString::fromStdString(p.name);
        break;
      }
    }

    QString protoStr = conn.protocol == ConnectionProtocol::TCP ? "TCP" : "UDP";
    QString stateStr = formatState(static_cast<int>(conn.state));

    // Filtering
    if (!filter.isEmpty()) {
      bool match = procName.toLower().contains(filter) ||
                   QString::number(conn.pid).contains(filter) ||
                   conn.localIp.contains(filter) ||
                   QString::number(conn.localPort).contains(filter) ||
                   conn.remoteIp.contains(filter) ||
                   QString::number(conn.remotePort).contains(filter) ||
                   stateStr.toLower().contains(filter);
      if (!match) continue;
    }

    m_Table->insertRow(row);

    m_Table->setItem(row, 0, new QTableWidgetItem(procName));
    m_Table->setItem(row, 1, new QTableWidgetItem(QString::number(conn.pid)));
    m_Table->setItem(row, 2, new QTableWidgetItem(protoStr));
    m_Table->setItem(row, 3, new QTableWidgetItem(conn.localIp));
    m_Table->setItem(row, 4, new QTableWidgetItem(QString::number(conn.localPort)));
    m_Table->setItem(row, 5, new QTableWidgetItem(conn.remoteIp));
    m_Table->setItem(row, 6, new QTableWidgetItem(QString::number(conn.remotePort)));
    m_Table->setItem(row, 7, new QTableWidgetItem(stateStr));

    row++;
  }

  m_Table->setUpdatesEnabled(true);
}

void NetworkView::onSearchTextChanged(const QString& text) {
  Refresh();
}

void NetworkView::onContextMenuRequested(const QPoint& pos) {
  QTableWidgetItem* item = m_Table->itemAt(pos);
  if (!item) return;

  int row = item->row();
  QString pidStr = m_Table->item(row, 1)->text();
  QString remoteIpStr = m_Table->item(row, 5)->text();
  uint32_t pid = pidStr.toUInt();

  QMenu menu(this);
  auto* killAction = menu.addAction("Kill Process");
  auto* copyIpAction = menu.addAction("Copy Remote IP");

  connect(killAction, &QAction::triggered, [this, pid]() {
    core::process::ProcessManager procMgr;
    procMgr.KillProcess(pid);
    Refresh();
  });

  connect(copyIpAction, &QAction::triggered, [remoteIpStr]() {
    QApplication::clipboard()->setText(remoteIpStr);
  });

  menu.exec(m_Table->viewport()->mapToGlobal(pos));
}

} // namespace severance::gui::network_view
