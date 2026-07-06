#include "SessionView.hpp"
#include "core/session/SessionManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include <QTime>
#include <QStandardPaths>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QScrollArea>
#include <algorithm>

namespace severance::gui::session_view {

class BiometricGraph : public QWidget {
public:
    explicit BiometricGraph(const QString& title, QColor color, int maxValue, QWidget* parent = nullptr)
        : QWidget(parent), m_Title(title), m_Color(color), m_MaxValue(maxValue) {
        setMinimumHeight(80);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void addValue(int val) {
        m_Values.append(val);
        if (m_Values.size() > 50) m_Values.removeFirst();
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // draw background
        painter.setBrush(QColor("#0D1117"));
        painter.setPen(QColor("#30363D"));
        painter.drawRoundedRect(rect().adjusted(0,0,-1,-1), 6, 6);

        // title
        painter.setPen(QColor("#E6EDF3"));
        QFont f = painter.font();
        f.setBold(true);
        painter.setFont(f);
        painter.drawText(QRect(12, 8, width() - 24, 20), Qt::AlignLeft, m_Title);

        if (m_Values.isEmpty()) return;

        // Draw line graph
        float stepX = (width() - 24) / 50.0f;
        
        QPainterPath path;
        for (int i = 0; i < m_Values.size(); ++i) {
            float x = 12 + i * stepX;
            float y = height() - 12 - ((float)m_Values[i] / m_MaxValue) * (height() - 40);
            if (i == 0) path.moveTo(x, y);
            else path.lineTo(x, y);
        }
        
        // Glow effect
        painter.setPen(QPen(QColor(m_Color.red(), m_Color.green(), m_Color.blue(), 40), 6));
        painter.drawPath(path);
        painter.setPen(QPen(QColor(m_Color.red(), m_Color.green(), m_Color.blue(), 100), 3));
        painter.drawPath(path);
        painter.setPen(QPen(m_Color, 1.5));
        painter.drawPath(path);
        
        // current value text
        painter.setPen(m_Color);
        painter.drawText(QRect(12, 8, width() - 24, 20), Qt::AlignRight, QString::number(m_Values.last()));
    }

private:
    QString m_Title;
    QColor m_Color;
    int m_MaxValue;
    QList<int> m_Values;
};

SessionView::SessionView(QWidget *parent) : QWidget(parent) {
  setupUI();

  auto* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &SessionView::updateStatus);
  timer->start(1000);
  
  m_BiometricTimer = new QTimer(this);
  connect(m_BiometricTimer, &QTimer::timeout, this, &SessionView::updateBiometrics);
  m_BiometricTimer->start(100); // 10 FPS updates
  
  updateStatus();
}

void SessionView::setupUI() {
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  auto* scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setStyleSheet("background-color: transparent;");

  auto* scrollContent = new QWidget();
  auto* scrollLayout = new QVBoxLayout(scrollContent);
  scrollLayout->setContentsMargins(24, 24, 24, 24);
  scrollLayout->setSpacing(20);

  // Header
  auto* header = new QLabel("Observation Protocol", this);
  header->setStyleSheet("font-size: 20px; font-weight: bold; color: #E6EDF3;");
  scrollLayout->addWidget(header);

  auto* desc = new QLabel("Monitor Innie vitals, log macrodata, and maintain the Observation Archive.", this);
  desc->setStyleSheet("color: #8B949E; font-size: 14px;");
  scrollLayout->addWidget(desc);
  scrollLayout->addSpacing(10);

  // Biometric Telemetry Panel
  auto* bioLabel = new QLabel("Biometric Telemetry", this);
  bioLabel->setStyleSheet("font-weight: bold; color: #E6EDF3; font-size: 16px;");
  scrollLayout->addWidget(bioLabel);

  auto* bioLayout = new QHBoxLayout();
  m_BrainwaveGraph = new BiometricGraph("Brainwave Freq. (Hz)", QColor("#58A6FF"), 100, this);
  m_StressGraph = new BiometricGraph("Stress Level", QColor("#DA3633"), 100, this);
  m_ChipRejectionGraph = new BiometricGraph("Chip Rejection Idx", QColor("#3FB950"), 100, this);
  
  bioLayout->addWidget(m_BrainwaveGraph);
  bioLayout->addWidget(m_StressGraph);
  bioLayout->addWidget(m_ChipRejectionGraph);
  scrollLayout->addLayout(bioLayout);

  // Controls Panel
  auto* controlsPane = new QWidget(this);
  controlsPane->setStyleSheet("background-color: #0D1117; border: 1px solid #30363D; border-radius: 6px;");
  auto* controlsLayout = new QVBoxLayout(controlsPane);

  auto* topRow = new QHBoxLayout();
  m_RecordBtn = new QPushButton("Initiate Observation", this);
  m_RecordBtn->setFixedSize(140, 36);
  m_RecordBtn->setStyleSheet("background-color: #DA3633; color: white; font-weight: bold; border-radius: 4px;");
  connect(m_RecordBtn, &QPushButton::clicked, this, &SessionView::onToggleRecording);

  m_StatusLabel = new QLabel("Status: Ready", this);
  m_StatusLabel->setStyleSheet("color: #8B949E; font-size: 14px;");
  
  m_EventCountLabel = new QLabel("Events: 0", this);
  m_EventCountLabel->setStyleSheet("color: #8B949E; font-size: 14px;");

  topRow->addWidget(m_RecordBtn);
  topRow->addSpacing(20);
  topRow->addWidget(m_StatusLabel);
  topRow->addStretch();
  topRow->addWidget(m_EventCountLabel);
  controlsLayout->addLayout(topRow);

  // Annotations
  controlsLayout->addSpacing(20);
  auto* annLabel = new QLabel("Qualitative Annotations", this);
  annLabel->setStyleSheet("font-weight: bold; color: #E6EDF3;");
  controlsLayout->addWidget(annLabel);

  auto* annRow = new QHBoxLayout();
  m_AnnotationInput = new QLineEdit(this);
  m_AnnotationInput->setPlaceholderText("Enter observation...");
  m_AnnotationInput->setStyleSheet("background-color: #010409; color: #E6EDF3; border: 1px solid #30363D; border-radius: 4px; padding: 4px;");
  m_AnnotationInput->setEnabled(false);

  m_AddAnnotationBtn = new QPushButton("Log Observation", this);
  m_AddAnnotationBtn->setStyleSheet("background-color: #238636; color: white; border-radius: 4px; padding: 4px 12px;");
  m_AddAnnotationBtn->setEnabled(false);
  connect(m_AddAnnotationBtn, &QPushButton::clicked, this, &SessionView::onAddAnnotation);
  connect(m_AnnotationInput, &QLineEdit::returnPressed, this, &SessionView::onAddAnnotation);

  annRow->addWidget(m_AnnotationInput);
  annRow->addWidget(m_AddAnnotationBtn);
  controlsLayout->addLayout(annRow);

  m_AnnotationList = new QListWidget(this);
  m_AnnotationList->setStyleSheet("background-color: #010409; color: #E6EDF3; border: 1px solid #30363D; border-radius: 4px;");
  controlsLayout->addWidget(m_AnnotationList);

  scrollLayout->addWidget(controlsPane);

  // Exports Panel
  auto* exportsPane = new QWidget(this);
  exportsPane->setStyleSheet("background-color: #0D1117; border: 1px solid #30363D; border-radius: 6px;");
  auto* exportsLayout = new QVBoxLayout(exportsPane);

  auto* exportLabel = new QLabel("Observation Archive", this);
  exportLabel->setStyleSheet("font-weight: bold; color: #E6EDF3;");
  exportsLayout->addWidget(exportLabel);

  auto* exportRow = new QHBoxLayout();
  m_ExportMdBtn = new QPushButton("Export Standard Ledger", this);
  m_ExportMdBtn->setStyleSheet("background-color: #1F6FEB; color: white; border-radius: 4px; padding: 6px 16px;");
  connect(m_ExportMdBtn, &QPushButton::clicked, this, &SessionView::onExportMarkdown);

  m_ExportJsonBtn = new QPushButton("Export Raw Telemetry", this);
  m_ExportJsonBtn->setStyleSheet("background-color: #30363D; color: white; border-radius: 4px; padding: 6px 16px;");
  connect(m_ExportJsonBtn, &QPushButton::clicked, this, &SessionView::onExportJson);

  exportRow->addWidget(m_ExportMdBtn);
  exportRow->addWidget(m_ExportJsonBtn);
  exportRow->addStretch();
  exportsLayout->addLayout(exportRow);

  scrollLayout->addWidget(exportsPane);
  scrollLayout->addStretch();
  
  scrollArea->setWidget(scrollContent);
  mainLayout->addWidget(scrollArea);
}

void SessionView::onToggleRecording() {
  auto& sm = core::session::SessionManager::GetInstance();
  if (sm.IsRecording()) {
    sm.StopRecording();
    m_RecordBtn->setText("Initiate Observation");
    m_RecordBtn->setStyleSheet("background-color: #DA3633; color: white; font-weight: bold; border-radius: 4px;");
    m_StatusLabel->setText("Status: Stopped");
    m_StatusLabel->setStyleSheet("color: #8B949E; font-size: 14px;");
    m_AnnotationInput->setEnabled(false);
    m_AddAnnotationBtn->setEnabled(false);
  } else {
    sm.StartRecording("Observation Session");
    m_RecordBtn->setText("Cease Observation");
    m_RecordBtn->setStyleSheet("background-color: #30363D; color: #E6EDF3; font-weight: bold; border-radius: 4px;");
    m_StatusLabel->setText("Status: OBSERVING");
    m_StatusLabel->setStyleSheet("color: #DA3633; font-weight: bold; font-size: 14px;");
    m_AnnotationInput->setEnabled(true);
    m_AddAnnotationBtn->setEnabled(true);
    m_AnnotationList->clear();
  }
}

void SessionView::onAddAnnotation() {
  QString note = m_AnnotationInput->text().trimmed();
  if (note.isEmpty()) return;

  auto& sm = core::session::SessionManager::GetInstance();
  if (sm.IsRecording()) {
    sm.AddAnnotation(note.toStdString());
    m_AnnotationList->addItem(QString("[%1] %2").arg(QTime::currentTime().toString("HH:mm:ss")).arg(note));
    m_AnnotationInput->clear();
  }
}

void SessionView::updateStatus() {
  auto& sm = core::session::SessionManager::GetInstance();
  if (sm.IsRecording()) {
    m_EventCountLabel->setText("Events: Observing...");
  } else {
    m_EventCountLabel->setText("Events: Stopped");
  }
}

void SessionView::updateBiometrics() {
  auto& sm = core::session::SessionManager::GetInstance();
  if (!sm.IsRecording()) {
    // Flatline when not observing
    m_BrainwaveGraph->addValue(0);
    m_StressGraph->addValue(0);
    m_ChipRejectionGraph->addValue(0);
    return;
  }
  
  // Random fluctuation for "Biometric Telemetry"
  static int brainwave = 40;
  static int stress = 20;
  static int chipRejection = 5;

  brainwave = std::clamp(brainwave + QRandomGenerator::global()->bounded(-5, 6), 10, 90);
  stress = std::clamp(stress + QRandomGenerator::global()->bounded(-3, 4), 0, 100);
  chipRejection = std::clamp(chipRejection + QRandomGenerator::global()->bounded(-1, 2), 0, 30);

  m_BrainwaveGraph->addValue(brainwave);
  m_StressGraph->addValue(stress);
  m_ChipRejectionGraph->addValue(chipRejection);
}

void SessionView::onExportMarkdown() {
  QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/SeveranceReports";
  auto& sm = core::session::SessionManager::GetInstance();
  std::string file = sm.ExportMarkdown(path.toStdString());
  QMessageBox::information(this, "Export Success", QString("Standard Ledger exported to:\n%1").arg(QString::fromStdString(file)));
  QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(file)));
}

void SessionView::onExportJson() {
  QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/SeveranceReports";
  auto& sm = core::session::SessionManager::GetInstance();
  std::string file = sm.ExportSession(path.toStdString());
  QMessageBox::information(this, "Export Success", QString("Raw Telemetry exported to:\n%1").arg(QString::fromStdString(file)));
  QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(file)));
}

} // namespace severance::gui::session_view
