#include "FileDetailPanel.hpp"
#include <QVBoxLayout>

namespace severance::gui::file_view {

FileDetailPanel::FileDetailPanel(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void FileDetailPanel::setupUI() {
    setLayout(new QVBoxLayout());
}

void FileDetailPanel::Clear() {}
void FileDetailPanel::LoadFileEvent(const QString&, const QString&, uint32_t, const QString&, const QString&) {}

} // namespace severance::gui::file_view
