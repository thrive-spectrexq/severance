#include "ProcessDetailPanel.hpp"
#include <QVBoxLayout>

namespace severance::gui::process_view {

ProcessDetailPanel::ProcessDetailPanel(QWidget* parent) : QWidget(parent) {
    setLayout(new QVBoxLayout());
}

void ProcessDetailPanel::Clear() {}
void ProcessDetailPanel::LoadProcess(uint32_t) {}

} // namespace severance::gui::process_view
