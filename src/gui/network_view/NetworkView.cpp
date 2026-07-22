#include "NetworkView.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QTime>

namespace severance::gui::network_view {

NetworkView::NetworkView(QWidget* parent) : QWidget(parent) {
    setupUI();
    m_RefreshTimer = new QTimer(this);
    connect(m_RefreshTimer, &QTimer::timeout, this, &NetworkView::onTimerTick);
    m_RefreshTimer->start(1000);
}

void NetworkView::setupUI() {
    auto* layout = new QVBoxLayout(this);
    setStyleSheet("background-color: #0A0F14; color: #E0FFFF; font-family: 'Courier New', Consolas, monospace;");

    auto* titleLabel = new QLabel("LUMON INTERNAL COMMUNICATIONS MONITOR", this);
    titleLabel->setStyleSheet("color: #00E5FF; font-size: 20px; font-weight: bold; padding: 10px; border: 1px solid #00E5FF;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    m_Table = new QTableWidget(5, 6, this);
    m_Table->setHorizontalHeaderLabels({"Channel", "Source", "Destination", "Protocol", "Status", "Duration"});
    m_Table->horizontalHeader()->setStretchLastSection(true);
    m_Table->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #0F1A24; color: #00E5FF; border: 1px solid #00E5FF; }");
    m_Table->verticalHeader()->setVisible(false);
    m_Table->setStyleSheet("QTableWidget { gridline-color: #00E5FF; border: 1px solid #00E5FF; background-color: #0A0F14; } QTableWidget::item { padding: 5px; }");
    m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    QString data[5][6] = {
        {"ALPHA-7", "MDR Terminal 4", "Management Hub", "LUMEN-SEC", "Active", "02:14:33"},
        {"BETA-3", "O&D Station 2", "Archive Vault", "LUMEN-INT", "Idle", "00:00:00"},
        {"GAMMA-1", "Break Room", "Compliance Core", "LUMEN-PRI", "Monitoring", "01:45:12"},
        {"DELTA-9", "Elevator Control", "Chip Array", "LUMEN-SEC", "Active", "04:22:01"},
        {"OMEGA-0", "Board Interface", "Unknown", "CLASSIFIED", "Encrypted", "??:??:??"}
    };

    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 6; ++c) {
            auto* item = new QTableWidgetItem(data[r][c]);
            if (c == 4) {
                if (data[r][c] == "Active") item->setForeground(QColor("#00FF00"));
                else if (data[r][c] == "Idle") item->setForeground(QColor("#888888"));
                else if (data[r][c] == "Monitoring") item->setForeground(QColor("#FFA500"));
                else if (data[r][c] == "Encrypted") item->setForeground(QColor("#FF0000"));
            }
            m_Table->setItem(r, c, item);
        }
    }
    
    m_Table->resizeColumnsToContents();
    layout->addWidget(m_Table);
}

void NetworkView::onTimerTick() {
    for (int r = 0; r < 5; ++r) {
        if (m_Table->item(r, 4)->text() == "Active" || m_Table->item(r, 4)->text() == "Monitoring") {
            QString durStr = m_Table->item(r, 5)->text();
            auto time = QTime::fromString(durStr, "hh:mm:ss");
            if (time.isValid()) {
                time = time.addSecs(1);
                m_Table->item(r, 5)->setText(time.toString("hh:mm:ss"));
            }
        }
    }
}

void NetworkView::Refresh() {}
void NetworkView::onSearchTextChanged(const QString&) {}
void NetworkView::onContextMenuRequested(const QPoint&) {}
void NetworkView::onSelectionChanged() {}
QString NetworkView::formatState(int) { return QString(); }

} // namespace severance::gui::network_view
