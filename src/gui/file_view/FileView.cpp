#include "FileView.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>

namespace severance::gui::file_view {

FileView::FileView(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void FileView::setupUI() {
    auto* layout = new QVBoxLayout(this);
    setStyleSheet("background-color: #0A0F14; color: #E0FFFF; font-family: 'Courier New', Consolas, monospace;");

    auto* titleLabel = new QLabel("LUMON DOCUMENT ARCHIVE", this);
    titleLabel->setStyleSheet("color: #00E5FF; font-size: 20px; font-weight: bold; padding: 10px; border: 1px solid #00E5FF;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    m_Table = new QTableWidget(10, 6, this);
    m_Table->setHorizontalHeaderLabels({"Classification", "Document", "Department", "Author", "Date", "Status"});
    m_Table->horizontalHeader()->setStretchLastSection(true);
    m_Table->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #0F1A24; color: #00E5FF; border: 1px solid #00E5FF; }");
    m_Table->verticalHeader()->setVisible(false);
    m_Table->setStyleSheet("QTableWidget { gridline-color: #00E5FF; border: 1px solid #00E5FF; background-color: #0A0F14; } QTableWidget::item { padding: 5px; }");
    m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    QString data[10][6] = {
        {"TOP SECRET", "Severance Procedure Manual Rev. 7", "Management", "Dr. Arteta", "1983-04-12", "Active"},
        {"CONFIDENTIAL", "MDR Operational Guidelines", "MDR", "P. Milchick", "2020-01-15", "Active"},
        {"RESTRICTED", "Overtime Contingency Protocol", "Security", "Board", "2019-11-03", "Archived"},
        {"TOP SECRET", "The Lexington Letter", "Unknown", "Unknown", "2022-06-01", "Classified"},
        {"CONFIDENTIAL", "Waffle Party Eligibility Criteria", "Wellness", "Ms. Casey", "2021-08-22", "Active"},
        {"RESTRICTED", "O&D-MDR Interdepartmental Policy", "Management", "S. Cobel", "2020-03-10", "Under Review"},
        {"TOP SECRET", "Revolving — Theory & Application", "Research", "K. Eagan", "1944-01-01", "Eternal"},
        {"CONFIDENTIAL", "Innie Rights Memorandum (REJECTED)", "Legal", "Board", "2023-02-14", "Void"},
        {"RESTRICTED", "Cold Harbor Initiative Brief", "Management", "Board", "2021-12-01", "Active"},
        {"TOP SECRET", "Chip Implantation Surgical Guide", "Medical", "Dr. Arteta", "2015-06-30", "Active"}
    };

    for (int r = 0; r < 10; ++r) {
        for (int c = 0; c < 6; ++c) {
            auto* item = new QTableWidgetItem(data[r][c]);
            if (c == 0) {
                if (data[r][c] == "TOP SECRET") item->setForeground(QColor("#FF0000"));
                else if (data[r][c] == "CONFIDENTIAL") item->setForeground(QColor("#FFA500"));
                else if (data[r][c] == "RESTRICTED") item->setForeground(QColor("#FFFF00"));
            }
            m_Table->setItem(r, c, item);
        }
    }
    
    m_Table->resizeColumnsToContents();
    layout->addWidget(m_Table);
}

void FileView::appendEvent(const severance::core::filesystem::FileEvent&) {}
void FileView::onSearchTextChanged(const QString&) {}
void FileView::onContextMenuRequested(const QPoint&) {}
void FileView::onSelectionChanged() {}
void FileView::processPendingEvents() {}
void FileView::updateReceptors() {}

} // namespace severance::gui::file_view
