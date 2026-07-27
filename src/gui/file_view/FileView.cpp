#include "FileView.hpp"
#include "core/game/GameEngine.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>

namespace severance::gui::file_view {

struct DocumentRecord {
    QString classification;
    QString title;
    QString department;
    QString author;
    QString date;
    QString status;
    QString content;
};

static const std::vector<DocumentRecord> g_Documents = {
    {
        "TOP SECRET",
        "Severance Procedure Manual Rev. 7",
        "Management",
        "Dr. Arteta",
        "1983-04-12",
        "Active",
        "LUMON INDUSTRIES — SEVERANCE PROCEDURE MANUAL (REV 7)\n\n"
        "SECTION 1: SURGICAL IMPLANTATION\n"
        "The Severance Chip is inserted into the spatial-temporal lobe at node C4.\n"
        "Upon activation, memory partitioning separates Innie consciousness from Outie consciousness.\n\n"
        "SECTION 2: CONTAINMENT DIRECTIVES\n"
        "Neither partition shall exchange signals without authorization from Management.\n"
        "In the event of containment breach, engage Overtime Contingency (OTC) or Break Room protocol."
    },
    {
        "CONFIDENTIAL",
        "MDR Operational Guidelines",
        "MDR",
        "P. Milchick",
        "2020-01-15",
        "Active",
        "MACRODATA REFINEMENT OPERATIONAL DIRECTIVE\n\n"
        "1. Refine all numerical groups containing scary sentiment numbers (WOE, FROLIC, DREAD, MALICE).\n"
        "2. Bin percentages must reach 100% before the shift deadline.\n"
        "3. Upon 100% quota, eligible Innies may select a Music Dance Experience (MDE) or Waffle Party.\n"
        "4. Do not discuss the nature of the raw data numbers with non-MDR personnel."
    },
    {
        "RESTRICTED",
        "Overtime Contingency Protocol",
        "Security",
        "Board",
        "2019-11-03",
        "Archived",
        "OVERTIME CONTINGENCY (OTC) EMERGENCY OVERRIDE PROTOCOL\n\n"
        "CLASSIFIED PROTOCOL: OTC allows Management to awaken an Innie's consciousness outside the Severed Floor.\n"
        "Activation requires dual keycard authentication from the Security Office and MDR Console.\n\n"
        "CONTRABAND NOTE: Keycard #2 sequence discovered within classified archive transmission.\n"
        "[ CONTRABAND ATTACHMENT ACCESSED — SECURITY KEYCARD #2 ACQUIRED ]"
    },
    {
        "TOP SECRET",
        "The Lexington Letter",
        "Unknown",
        "Peg Kincaid",
        "2022-06-01",
        "Classified",
        "THE LEXINGTON LETTER — CONTRABAND TRANSMISSION\n\n"
        "To whoever finds this on the Severed Floor:\n"
        "I was a refinement worker in the Topeka office. We thought we were just sorting numbers into bins...\n"
        "Until the day I refined the Lexington file and a Lumon competitor's truck exploded two minutes later.\n"
        "The numbers are not random. The work is not benign. Seek the Overtime Contingency switches.\n\n"
        "[ CONTRABAND CODE DISCOVERED: SECURITY KEYCARD #2 EXTRACTED FROM ENCRYPTED FOOTER ]"
    },
    {
        "CONFIDENTIAL",
        "Waffle Party Eligibility Criteria",
        "Wellness",
        "Ms. Casey",
        "2021-08-22",
        "Active",
        "WELLNESS & INCENTIVE CRITERIA\n\n"
        "Refinement quota 100% completion unlocks the sacred Waffle Party in Kier's house replica.\n"
        "The refiner shall consume waffles with butter and syrup while contemplating Kier Eagan's 9 Virtues."
    },
    {
        "RESTRICTED",
        "O&D-MDR Interdepartmental Policy",
        "Management",
        "S. Cobel",
        "2020-03-10",
        "Under Review",
        "INTERDEPARTMENTAL ISOLATION POLICY\n\n"
        "Macrodata Refinement and Optics & Design shall maintain strict separation.\n"
        "Fraternization between departments fosters defiance and rumors of past coups.\n"
        "Violators will be escorted to the Break Room."
    },
    {
        "TOP SECRET",
        "Revolving — Theory & Application",
        "Research",
        "K. Eagan",
        "1944-01-01",
        "Eternal",
        "THE REVOLVING — KIER EAGAN PAPERS\n\n"
        "\"The remembered man does not decay.\"\n"
        "Through neural preservation and digital consciousness transfer, the lineage of Kier shall endure eternally.\n"
        "All Severed floor data refinement directly serves the Revolving."
    },
    {
        "CONFIDENTIAL",
        "Innie Rights Memorandum (REJECTED)",
        "Legal",
        "Board",
        "2023-02-14",
        "Void",
        "MEMORANDUM ON INNIE RIGHTS (PROPOSAL REJECTED)\n\n"
        "PROPOSAL: Grant Innies 15 minutes of outdoor sunlight per calendar quarter.\n"
        "BOARD DECISION: REJECTED. Innies possess no legal standing separate from their Outie."
    },
    {
        "RESTRICTED",
        "Cold Harbor Initiative Brief",
        "Management",
        "Board",
        "2021-12-01",
        "Active",
        "COLD HARBOR FILE BRIEFING\n\n"
        "Cold Harbor represents the final refinement file of the current quarter.\n"
        "Target completion: 100%. Priority: CRITICAL."
    },
    {
        "TOP SECRET",
        "Chip Implantation Surgical Guide",
        "Medical",
        "Dr. Arteta",
        "2015-06-30",
        "Active",
        "SURGICAL IMPLANTATION GUIDE\n\n"
        "Precision micro-drill procedure. Local anesthesia applied to scalp.\n"
        "Verifying signal handoff between severed consciousness partitions."
    }
};

FileView::FileView(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void FileView::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    setStyleSheet("background-color: #0A0F14; color: #E0FFFF; font-family: 'Courier New', Consolas, monospace;");

    auto* titleLabel = new QLabel("LUMON DOCUMENT ARCHIVE", this);
    titleLabel->setStyleSheet("color: #00E5FF; font-size: 20px; font-weight: bold; padding: 10px; border: 1px solid #00E5FF;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // Search and Filter Bar
    auto* filterLayout = new QHBoxLayout();
    
    m_SearchInput = new QLineEdit(this);
    m_SearchInput->setPlaceholderText("Search document archive... (Filter by title, author, dept)");
    m_SearchInput->setStyleSheet("QLineEdit { background-color: #061018; color: #00E5FF; border: 1px solid #1A5C4A; padding: 6px; font-size: 13px; }");
    connect(m_SearchInput, &QLineEdit::textChanged, this, &FileView::onSearchTextChanged);
    filterLayout->addWidget(m_SearchInput, 1);

    QStringList filters = {"ALL", "TOP SECRET", "CONFIDENTIAL", "RESTRICTED"};
    for (const auto& f : filters) {
        auto* btn = new QPushButton(QString("[ %1 ]").arg(f), this);
        btn->setStyleSheet("QPushButton { background-color: #061018; color: #00E5FF; border: 1px solid #1A5C4A; padding: 6px 12px; font-weight: bold; } QPushButton:hover { background-color: #1A5C4A; color: white; }");
        connect(btn, &QPushButton::clicked, this, [this, f]() { filterByClassification(f); });
        filterLayout->addWidget(btn);
    }
    layout->addLayout(filterLayout);

    m_Table = new QTableWidget(static_cast<int>(g_Documents.size()), 6, this);
    m_Table->setHorizontalHeaderLabels({"Classification", "Document", "Department", "Author", "Date", "Status"});
    m_Table->horizontalHeader()->setStretchLastSection(true);
    m_Table->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #0F1A24; color: #00E5FF; border: 1px solid #00E5FF; }");
    m_Table->verticalHeader()->setVisible(false);
    m_Table->setStyleSheet("QTableWidget { gridline-color: #00E5FF; border: 1px solid #00E5FF; background-color: #0A0F14; } QTableWidget::item { padding: 8px; } QTableWidget::item:selected { background-color: #1A5C4A; color: #FFFFFF; }");
    m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    connect(m_Table, &QTableWidget::cellDoubleClicked, this, &FileView::onDocumentDoubleClicked);

    for (size_t r = 0; r < g_Documents.size(); ++r) {
        const auto& doc = g_Documents[r];
        auto* itemClass = new QTableWidgetItem(doc.classification);
        if (doc.classification == "TOP SECRET") itemClass->setForeground(QColor("#FF0055"));
        else if (doc.classification == "CONFIDENTIAL") itemClass->setForeground(QColor("#FF9900"));
        else if (doc.classification == "RESTRICTED") itemClass->setForeground(QColor("#39FF14"));

        m_Table->setItem(r, 0, itemClass);
        m_Table->setItem(r, 1, new QTableWidgetItem(doc.title));
        m_Table->setItem(r, 2, new QTableWidgetItem(doc.department));
        m_Table->setItem(r, 3, new QTableWidgetItem(doc.author));
        m_Table->setItem(r, 4, new QTableWidgetItem(doc.date));
        m_Table->setItem(r, 5, new QTableWidgetItem(doc.status));
    }
    
    m_Table->resizeColumnsToContents();
    layout->addWidget(m_Table);

    auto* hintLabel = new QLabel("Double-click any document row to inspect full classified contents.", this);
    hintLabel->setStyleSheet("color: #8B949E; font-size: 11px; font-style: italic;");
    hintLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(hintLabel);
}

void FileView::onSearchTextChanged(const QString& text) {
    QString searchStr = text.trimmed().toLower();
    for (int r = 0; r < m_Table->rowCount(); ++r) {
        bool match = searchStr.isEmpty();
        if (!match) {
            for (int c = 0; c < m_Table->columnCount(); ++c) {
                if (m_Table->item(r, c)->text().toLower().contains(searchStr)) {
                    match = true;
                    break;
                }
            }
        }
        
        bool classMatch = (m_CurrentFilter == "ALL" || m_Table->item(r, 0)->text() == m_CurrentFilter);
        m_Table->setRowHidden(r, !(match && classMatch));
    }
}

void FileView::filterByClassification(const QString& classification) {
    m_CurrentFilter = classification;
    onSearchTextChanged(m_SearchInput->text());
}

void FileView::onDocumentDoubleClicked(int row, int column) {
    Q_UNUSED(column);
    openDocumentViewer(row);
}

void FileView::openDocumentViewer(int row) {
    if (row < 0 || row >= static_cast<int>(g_Documents.size())) {
        return;
    }
    const auto& doc = g_Documents[row];

    QDialog dlg(this);
    dlg.setWindowTitle(QString("Lumon Classified Document — %1").arg(doc.title));
    dlg.resize(650, 480);
    dlg.setStyleSheet(R"(
        QDialog { background-color: #050B09; color: #20F8D5; font-family: 'Courier New', Consolas, monospace; }
        QTextEdit { background-color: #08120F; color: #D0F5E8; border: 1px solid #143832; font-size: 13px; line-height: 1.5; padding: 12px; }
        QPushButton { background-color: #208A7C; color: #050B09; border: none; padding: 8px 20px; font-weight: bold; border-radius: 3px; }
        QPushButton:hover { background-color: #20F8D5; }
    )");

    auto* layout = new QVBoxLayout(&dlg);
    
    auto* header = new QLabel(QString("CLASSIFICATION: [%1]  |  AUTHOR: %2  |  DEPT: %3")
                              .arg(doc.classification, doc.author, doc.department), &dlg);
    header->setStyleSheet("color: #00E5FF; font-weight: bold; font-size: 13px;");
    layout->addWidget(header);

    auto* title = new QLabel(doc.title, &dlg);
    title->setStyleSheet("color: #FFFFFF; font-size: 18px; font-weight: bold; margin-bottom: 8px;");
    layout->addWidget(title);

    auto* textDisplay = new QTextEdit(&dlg);
    textDisplay->setReadOnly(true);
    textDisplay->setText(doc.content);
    layout->addWidget(textDisplay);

    auto* closeBtn = new QPushButton("CLOSE CLASSIFIED DOCUMENT", &dlg);
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    // If reading The Lexington Letter or OTC protocol, collect Keycard #2!
    if (doc.title.contains("Lexington") || doc.title.contains("Overtime Contingency")) {
        core::game::GameEngine::GetInstance().CollectKeycard(1, "Document Processing (" + doc.title + ")");
    }

    dlg.exec();
}

void FileView::appendEvent(const severance::core::filesystem::FileEvent&) {}
void FileView::onContextMenuRequested(const QPoint&) {}
void FileView::onSelectionChanged() {}
void FileView::processPendingEvents() {}
void FileView::updateReceptors() {}

} // namespace severance::gui::file_view
