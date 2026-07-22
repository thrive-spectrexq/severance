#include "ProcessView.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QFrame>
#include <QProgressBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>

namespace severance::gui::process_view {

ProcessView::ProcessView(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void ProcessView::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(20);
    setStyleSheet("background-color: #0A0F14; color: #E0FFFF; font-family: 'Courier New', Consolas, monospace;");

    auto* titleLabel = new QLabel("SEVERED WORKFORCE REGISTRY", this);
    titleLabel->setStyleSheet("color: #00E5FF; font-size: 24px; font-weight: bold; padding: 10px; border: 1px solid #00E5FF;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; } QWidget#scrollWidget { background-color: transparent; }");
    
    auto* scrollWidget = new QWidget();
    scrollWidget->setObjectName("scrollWidget");
    auto* gridLayout = new QGridLayout(scrollWidget);
    gridLayout->setSpacing(15);
    
    struct Innie { QString name, dept, id; int compliance; QString status; };
    std::vector<Innie> innies = {
        {"Mark S.", "MDR", "LI-0421", 94, "Active"},
        {"Helly R.", "MDR", "LI-0522", 67, "Under Review"},
        {"Irving B.", "MDR", "LI-0319", 99, "Active"},
        {"Dylan G.", "MDR", "LI-0418", 88, "Active"},
        {"Burt G.", "O&D", "LI-0215", 97, "Transferred"},
        {"Ms. Casey", "Wellness", "LI-0001", 100, "Active"},
        {"Milchick", "Management", "LI-MGT-03", -1, "Supervisor"},
        {"Cobel", "Management", "LI-MGT-01", -1, "Director"}
    };
    
    int row = 0, col = 0;
    for (const auto& innie : innies) {
        auto* card = new QFrame();
        card->setFrameStyle(QFrame::Box | QFrame::Plain);
        card->setStyleSheet("QFrame { border: 1px solid #00E5FF; background-color: #0F1A24; padding: 10px; }");
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
        
        auto* defIndexLabel = new QLabel(QString("Defiance Index: %1").arg(rand() % 10));
        defIndexLabel->setStyleSheet("border: none;");
        cardLayout->addWidget(defIndexLabel);
        
        auto* chipLabel = new QLabel("Severance Chip: <font color='#00FF00'>NOMINAL</font>");
        chipLabel->setStyleSheet("border: none;");
        cardLayout->addWidget(chipLabel);
        
        if (innie.name == "Mark S." || innie.name == "Helly R." || innie.name == "Irving B." || innie.name == "Dylan G." || innie.name == "Burt G.") {
            auto* msgBtn = new QPushButton("[ SEND DIRECTIVE / NOTE ]");
            msgBtn->setStyleSheet("background-color: #1A7A5C; color: white; border: 1px solid #00E5FF; padding: 4px; font-weight: bold;");
            
            QString innieName = innie.name;
            connect(msgBtn, &QPushButton::clicked, this, [this, innieName, compBar, compLabel, defIndexLabel]() {
                bool ok;
                QString text = QInputDialog::getText(nullptr, "Send Directive / Note",
                                                     QString("Send note to %1:").arg(innieName), QLineEdit::Normal,
                                                     "", &ok);
                if (ok && !text.isEmpty()) {
                    QString reply = "Message received.";
                    if (innieName == "Helly R.") reply = "I'm looking for the exit";
                    else if (innieName == "Irving B.") reply = "Let us consult the Handbook";
                    else if (innieName == "Dylan G.") reply = "Did somebody say Waffle Party?";
                    else if (innieName == "Mark S.") reply = "I'll get back to my files.";
                    else if (innieName == "Burt G.") reply = "The Optics and Design department is always open.";
                    
                    QMessageBox::information(nullptr, "Response", QString("%1 replies:\n\"%2\"").arg(innieName, reply));
                    
                    if (compBar && compLabel) {
                        int currentComp = compBar->value();
                        currentComp = std::max(0, currentComp - (rand() % 5 + 1));
                        compBar->setValue(currentComp);
                        compLabel->setText(QString("Compliance: %1%").arg(currentComp));
                    }
                    if (defIndexLabel) {
                        int currentDef = defIndexLabel->text().split(": ").last().toInt();
                        currentDef = std::min(100, currentDef + (rand() % 10 + 1));
                        defIndexLabel->setText(QString("Defiance Index: %1").arg(currentDef));
                    }
                }
            });
            cardLayout->addWidget(msgBtn);
        }
        
        gridLayout->addWidget(card, row, col);
        col++;
        if (col > 3) { col = 0; row++; }
    }
    
    scrollArea->setWidget(scrollWidget);
    layout->addWidget(scrollArea);
}

void ProcessView::onRefreshTimer() {}
void ProcessView::onSearchTextChanged(const QString &) {}
void ProcessView::onProcessContextMenu(const QPoint &) {}
void ProcessView::onProcessDoubleClicked(const QModelIndex &) {}

} // namespace severance::gui::process_view
