#include "NetworkDetailPanel.hpp"
#include <QVBoxLayout>

namespace severance::gui::network_view {

NetworkDetailPanel::NetworkDetailPanel(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void NetworkDetailPanel::setupUI() {
    setLayout(new QVBoxLayout());
}

void NetworkDetailPanel::Clear() {}
void NetworkDetailPanel::LoadConnection(uint32_t, const QString&, const QString&, uint16_t, const QString&, uint16_t, const QString&) {}

} // namespace severance::gui::network_view
