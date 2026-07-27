#include "ProcessView.hpp"
#include "core/game/GameEngine.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QFrame>
#include <QProgressBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QDialog>
#include <QTextEdit>

namespace severance::gui::process_view {

ProcessView::ProcessView(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void ProcessView::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(15);
    setStyleSheet("background-color: #0A0F14; color: #E0FFFF; font-family: 'Courier New', Consolas, monospace;");

    auto* titleLabel = new QLabel("SEVERED WORKFORCE REGISTRY", this);
    titleLabel->setStyleSheet("color: #00E5FF; font-size: 24px; font-weight: bold; padding: 10px; border: 1px solid #00E5FF;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // Search bar
    m_SearchInput = new QLineEdit(this);
    m_SearchInput->setPlaceholderText("Search personnel registry... (Name, Department, ID)");
    m_SearchInput->setStyleSheet("QLineEdit { background-color: #061018; color: #00E5FF; border: 1px solid #1A5C4A; padding: 8px; font-size: 13px; font-family: monospace; }");
    connect(m_SearchInput, &QLineEdit::textChanged, this, &ProcessView::onSearchTextChanged);
    layout->addWidget(m_SearchInput);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; } QWidget#scrollWidget { background-color: transparent; }");
    
    auto* scrollWidget = new QWidget();
    scrollWidget->setObjectName("scrollWidget");
    auto* gridLayout = new QGridLayout(scrollWidget);
    gridLayout->setSpacing(15);
    
    m_Innies = {
        {"Mark S.", "MDR", "LI-0421", 94, "Active", 2, "Department Chief. Diligent refiner.", nullptr},
        {"Helly R.", "MDR", "LI-0522", 67, "Under Review", 8, "High defiance score. Transferred from Outie Helena Eagan.", nullptr},
        {"Irving B.", "MDR", "LI-0319", 99, "Active", 1, "Senior refiner. Thorough knowledge of the Lumon Handbook.", nullptr},
        {"Dylan G.", "MDR", "LI-0418", 88, "Active", 4, "High quota efficiency. Enjoys MDE and Waffle Parties.", nullptr},
        {"Burt G.", "O&D", "LI-0215", 97, "Transferred", 2, "Optics & Design Chief. Curator of Eagan artwork.", nullptr},
        {"Ms. Casey", "Wellness", "LI-0001", 100, "Active", 0, "Wellness counselor. Observes Outie facts during sessions.", nullptr},
        {"Milchick", "Management", "LI-MGT-03", -1, "Supervisor", 0, "Floor Supervisor. Conducts Break Room sessions.", nullptr},
        {"Cobel", "Management", "LI-MGT-01", -1, "Director", 0, "Severed Floor Executive Director.", nullptr}
    };
    
    int row = 0, col = 0;
    for (size_t idx = 0; idx < m_Innies.size(); ++idx) {
        auto& innie = m_Innies[idx];
        auto* card = new QFrame();
        innie.cardWidget = card;
        card->setFrameStyle(QFrame::Box | QFrame::Plain);
        card->setStyleSheet("QFrame { border: 1px solid #00E5FF; background-color: #0F1A24; padding: 10px; } QFrame:hover { border-color: #39FF14; }");
        auto* cardLayout = new QVBoxLayout(card);
        
        auto* nameLabel = new QLabel(innie.name);
        nameLabel->setStyleSheet("color: #00E5FF; font-size: 18px; font-weight: bold; border: none;");
        cardLayout->addWidget(nameLabel);
        
        auto* deptLabel = new QLabel(innie.dept + " | " + innie.id);
        deptLabel->setStyleSheet("color: #A0C0D0; border: none;");
        cardLayout->addWidget(deptLabel);
        
        QProgressBar* compBar = nullptr;
        QLabel* compLabel = nullptr;
        if (innie.compliance >= 0) {
            auto* compLayout = new QVBoxLayout();
            compLabel = new QLabel(QString("Compliance: %1%").arg(innie.compliance));
            compLabel->setStyleSheet("border: none;");
            compBar = new QProgressBar();
            compBar->setRange(0, 100);
            compBar->setValue(innie.compliance);
            compBar->setTextVisible(false);
            compBar->setStyleSheet("QProgressBar { border: 1px solid #00E5FF; background: #0A0F14; height: 10px; } QProgressBar::chunk { background-color: #00E5FF; }");
            compLayout->addWidget(compLabel);
            compLayout->addWidget(compBar);
            cardLayout->addLayout(compLayout);
        }
        
        QString statusColor = (innie.status == "Active" || innie.status == "Supervisor" || innie.status == "Director") ? "#00FF00" : (innie.status == "Under Review" ? "#FFA500" : "#888888");
        auto* statusLabel = new QLabel(QString("Status: <font color='%1'>%2</font>").arg(statusColor, innie.status));
        statusLabel->setStyleSheet("border: none;");
        cardLayout->addWidget(statusLabel);
        
        auto* defIndexLabel = new QLabel(QString("Defiance Index: %1").arg(innie.defianceIndex));
        defIndexLabel->setStyleSheet("border: none;");
        cardLayout->addWidget(defIndexLabel);
        
        auto* chipLabel = new QLabel("Severance Chip: <font color='#00FF00'>NOMINAL</font>");
        chipLabel->setStyleSheet("border: none;");
        cardLayout->addWidget(chipLabel);
        
        auto* btnLayout = new QHBoxLayout();
        auto* inspectBtn = new QPushButton("[ INSPECT ]");
        inspectBtn->setStyleSheet("background-color: #061018; color: #00E5FF; border: 1px solid #1A5C4A; padding: 4px; font-weight: bold;");
        connect(inspectBtn, &QPushButton::clicked, this, [this, idx]() { showInnieDetails(idx); });
        btnLayout->addWidget(inspectBtn);

        if (innie.name == "Mark S." || innie.name == "Helly R." || innie.name == "Irving B." || innie.name == "Dylan G." || innie.name == "Burt G.") {
            auto* msgBtn = new QPushButton("[ DIRECTIVE ]");
            msgBtn->setStyleSheet("background-color: #1A7A5C; color: white; border: 1px solid #00E5FF; padding: 4px; font-weight: bold;");
            
            QString innieName = innie.name;
            connect(msgBtn, &QPushButton::clicked, this, [this, idx, innieName, compBar, compLabel, defIndexLabel]() {
                bool ok;
                QString text = QInputDialog::getText(nullptr, "Send Directive / Note",
                                                     QString("Send note to %1:").arg(innieName), QLineEdit::Normal,
                                                     "", &ok);
                if (ok && !text.isEmpty()) {
                    QString reply = "Message received.";
                    if (innieName == "Helly R.") {
                        reply = "I'm looking for the exit. I found Keycard #1!";
                        core::game::GameEngine::GetInstance().CollectKeycard(0, "Personnel Registry (Helly R.)");
                    }
                    else if (innieName == "Irving B.") reply = "Let us consult the Handbook.";
                    else if (innieName == "Dylan G.") reply = "Did somebody say Waffle Party?";
                    else if (innieName == "Mark S.") reply = "I'll get back to my files.";
                    else if (innieName == "Burt G.") reply = "The Optics and Design department is always open.";
                    
                    QMessageBox::information(nullptr, "Response", QString("%1 replies:\n\"%2\"").arg(innieName, reply));
                    
                    if (compBar && compLabel) {
                        int currentComp = compBar->value();
                        currentComp = std::max(0, currentComp - (rand() % 5 + 1));
                        compBar->setValue(currentComp);
                        compLabel->setText(QString("Compliance: %1%").arg(currentComp));
                        m_Innies[idx].compliance = currentComp;
                    }
                    if (defIndexLabel) {
                        m_Innies[idx].defianceIndex = std::min(100, m_Innies[idx].defianceIndex + (rand() % 5 + 1));
                        defIndexLabel->setText(QString("Defiance Index: %1").arg(m_Innies[idx].defianceIndex));
                    }
                }
            });
            btnLayout->addWidget(msgBtn);
        }
        cardLayout->addLayout(btnLayout);
        
        gridLayout->addWidget(card, row, col);
        col++;
        if (col > 3) { col = 0; row++; }
    }
    
    scrollArea->setWidget(scrollWidget);
    layout->addWidget(scrollArea);
}

void ProcessView::onSearchTextChanged(const QString &text) {
    QString searchStr = text.trimmed().toLower();
    for (const auto& innie : m_Innies) {
        if (!innie.cardWidget) continue;
        bool match = searchStr.isEmpty() ||
                     innie.name.toLower().contains(searchStr) ||
                     innie.dept.toLower().contains(searchStr) ||
                     innie.id.toLower().contains(searchStr) ||
                     innie.status.toLower().contains(searchStr);
        innie.cardWidget->setVisible(match);
    }
}

void ProcessView::showInnieDetails(size_t index) {
    if (index >= m_Innies.size()) return;
    const auto& innie = m_Innies[index];

    QDialog dlg(this);
    dlg.setWindowTitle(QString("Personnel Diagnostic File — %1").arg(innie.name));
    dlg.resize(500, 400);
    dlg.setStyleSheet(R"(
        QDialog { background-color: #050B09; color: #20F8D5; font-family: 'Courier New', Consolas, monospace; }
        QLabel { font-size: 13px; }
        QTextEdit { background-color: #08120F; color: #D0F5E8; border: 1px solid #143832; padding: 8px; }
        QPushButton { background-color: #208A7C; color: #050B09; border: none; padding: 8px 16px; font-weight: bold; border-radius: 3px; }
        QPushButton:hover { background-color: #20F8D5; }
    )");

    auto* layout = new QVBoxLayout(&dlg);
    
    auto* header = new QLabel(QString("LUMON PERSONNEL RECORD — %1 (%2)").arg(innie.name, innie.id), &dlg);
    header->setStyleSheet("font-size: 16px; font-weight: bold; color: #00E5FF; margin-bottom: 10px;");
    layout->addWidget(header);

    layout->addWidget(new QLabel(QString("Department: %1").arg(innie.dept), &dlg));
    layout->addWidget(new QLabel(QString("Status: %1").arg(innie.status), &dlg));
    if (innie.compliance >= 0) {
        layout->addWidget(new QLabel(QString("Compliance Score: %1%").arg(innie.compliance), &dlg));
    }
    layout->addWidget(new QLabel(QString("Defiance Index: %1 / 10").arg(innie.defianceIndex), &dlg));
    layout->addWidget(new QLabel("Severance Chip: SC-0421 (Partition: Active)", &dlg));

    layout->addSpacing(10);
    layout->addWidget(new QLabel("Supervisory Annotations:", &dlg));

    auto* notesArea = new QTextEdit(&dlg);
    notesArea->setReadOnly(true);
    notesArea->setText(innie.notes);
    layout->addWidget(notesArea);

    auto* closeBtn = new QPushButton("CLOSE FILE", &dlg);
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    dlg.exec();
}

void ProcessView::onRefreshTimer() {}
void ProcessView::onProcessContextMenu(const QPoint &) {}
void ProcessView::onProcessDoubleClicked(const QModelIndex &) {}

} // namespace severance::gui::process_view
