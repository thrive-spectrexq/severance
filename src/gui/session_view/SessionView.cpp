#include "SessionView.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include <QTime>
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

        painter.setBrush(QColor("#0A0F14"));
        painter.setPen(QColor("#1A7A5C"));
        painter.drawRoundedRect(rect().adjusted(0,0,-1,-1), 6, 6);

        painter.setPen(QColor("#E0FFFF"));
        QFont f = painter.font();
        f.setBold(true);
        painter.setFont(f);
        painter.drawText(QRect(12, 8, width() - 24, 20), Qt::AlignLeft, m_Title);

        if (m_Values.isEmpty()) return;

        float stepX = (width() - 24) / 50.0f;
        
        QPainterPath path;
        for (int i = 0; i < m_Values.size(); ++i) {
            float x = 12 + i * stepX;
            float y = height() - 12 - ((float)m_Values[i] / m_MaxValue) * (height() - 40);
            if (i == 0) path.moveTo(x, y);
            else path.lineTo(x, y);
        }
        
        painter.setPen(QPen(QColor(m_Color.red(), m_Color.green(), m_Color.blue(), 40), 6));
        painter.drawPath(path);
        painter.setPen(QPen(QColor(m_Color.red(), m_Color.green(), m_Color.blue(), 100), 3));
        painter.drawPath(path);
        painter.setPen(QPen(m_Color, 1.5));
        painter.drawPath(path);
        
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
  m_BiometricTimer->start(100);
  
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

  auto* header = new QLabel("OBSERVATION PROTOCOL", this);
  header->setStyleSheet("font-family: 'Courier New', Courier, monospace; font-size: 20px; font-weight: bold; color: #00E5FF;");
  scrollLayout->addWidget(header);

  auto* bioLabel = new QLabel("Biometric Telemetry", this);
  bioLabel->setStyleSheet("font-weight: bold; color: #E0FFFF; font-size: 16px; font-family: 'Courier New', Courier, monospace;");
  scrollLayout->addWidget(bioLabel);

  auto* bioLayout = new QHBoxLayout();
  m_BrainwaveGraph = new BiometricGraph("Brainwave Freq. (Hz)", QColor("#00E5FF"), 100, this);
  m_StressGraph = new BiometricGraph("Stress Level", QColor("#DA3633"), 100, this);
  m_ChipRejectionGraph = new BiometricGraph("Chip Rejection Idx", QColor("#1A7A5C"), 100, this);
  
  bioLayout->addWidget(m_BrainwaveGraph);
  bioLayout->addWidget(m_StressGraph);
  bioLayout->addWidget(m_ChipRejectionGraph);
  scrollLayout->addLayout(bioLayout);

  auto* controlsPane = new QWidget(this);
  controlsPane->setStyleSheet("background-color: #0A0F14; border: 1px solid #1A7A5C; border-radius: 6px; font-family: 'Courier New', Courier, monospace;");
  auto* controlsLayout = new QVBoxLayout(controlsPane);

  auto* topRow = new QHBoxLayout();
  m_RecordBtn = new QPushButton("START OBSERVATION", this);
  m_RecordBtn->setFixedSize(160, 36);
  m_RecordBtn->setStyleSheet("background-color: #DA3633; color: white; font-weight: bold; border-radius: 4px;");
  connect(m_RecordBtn, &QPushButton::clicked, this, &SessionView::onToggleRecording);

  m_StatusLabel = new QLabel("No active observation", this);
  m_StatusLabel->setStyleSheet("color: #00E5FF; font-size: 14px;");
  
  m_EventCountLabel = new QLabel("Events: 0", this);
  m_EventCountLabel->setStyleSheet("color: #00E5FF; font-size: 14px;");

  topRow->addWidget(m_RecordBtn);
  topRow->addSpacing(20);
  topRow->addWidget(m_StatusLabel);
  topRow->addStretch();
  topRow->addWidget(m_EventCountLabel);
  controlsLayout->addLayout(topRow);

  controlsLayout->addSpacing(20);
  auto* annLabel = new QLabel("Qualitative Annotations", this);
  annLabel->setStyleSheet("font-weight: bold; color: #E0FFFF;");
  controlsLayout->addWidget(annLabel);

  auto* annRow = new QHBoxLayout();
  m_AnnotationInput = new QLineEdit(this);
  m_AnnotationInput->setPlaceholderText("Enter observation...");
  m_AnnotationInput->setStyleSheet("background-color: #0A0F14; color: #E0FFFF; border: 1px solid #1A7A5C; border-radius: 4px; padding: 4px;");
  m_AnnotationInput->setEnabled(false);

  m_AddAnnotationBtn = new QPushButton("Log Observation", this);
  m_AddAnnotationBtn->setStyleSheet("background-color: #1A7A5C; color: white; border-radius: 4px; padding: 4px 12px;");
  m_AddAnnotationBtn->setEnabled(false);
  connect(m_AddAnnotationBtn, &QPushButton::clicked, this, &SessionView::onAddAnnotation);
  connect(m_AnnotationInput, &QLineEdit::returnPressed, this, &SessionView::onAddAnnotation);

  annRow->addWidget(m_AnnotationInput);
  annRow->addWidget(m_AddAnnotationBtn);
  controlsLayout->addLayout(annRow);

  m_AnnotationList = new QListWidget(this);
  m_AnnotationList->setStyleSheet("background-color: #0A0F14; color: #E0FFFF; border: 1px solid #1A7A5C; border-radius: 4px;");
  controlsLayout->addWidget(m_AnnotationList);

  scrollLayout->addWidget(controlsPane);

  auto* exportsPane = new QWidget(this);
  exportsPane->setStyleSheet("background-color: #0A0F14; border: 1px solid #1A7A5C; border-radius: 6px; font-family: 'Courier New', Courier, monospace;");
  auto* exportsLayout = new QVBoxLayout(exportsPane);

  auto* exportLabel = new QLabel("Observation Archive", this);
  exportLabel->setStyleSheet("font-weight: bold; color: #E0FFFF;");
  exportsLayout->addWidget(exportLabel);

  auto* exportRow = new QHBoxLayout();
  m_ExportMdBtn = new QPushButton("Export Standard Ledger", this);
  m_ExportMdBtn->setStyleSheet("background-color: #00E5FF; color: black; border-radius: 4px; padding: 6px 16px; font-weight: bold;");
  connect(m_ExportMdBtn, &QPushButton::clicked, this, &SessionView::onExportMarkdown);

  m_ExportJsonBtn = new QPushButton("Export Raw Telemetry", this);
  m_ExportJsonBtn->setStyleSheet("background-color: #1A7A5C; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold;");
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
  if (m_isRecording) {
    m_isRecording = false;
    m_RecordBtn->setText("START OBSERVATION");
    m_RecordBtn->setStyleSheet("background-color: #DA3633; color: white; font-weight: bold; border-radius: 4px;");
    m_StatusLabel->setText("No active observation");
    m_AnnotationInput->setEnabled(false);
    m_AddAnnotationBtn->setEnabled(false);
  } else {
    m_isRecording = true;
    m_RecordBtn->setText("STOP OBSERVATION");
    m_RecordBtn->setStyleSheet("background-color: #1A7A5C; color: white; font-weight: bold; border-radius: 4px;");
    m_StatusLabel->setText("Recording: incident_helly_r");
    m_AnnotationInput->setEnabled(true);
    m_AddAnnotationBtn->setEnabled(true);
    m_AnnotationList->clear();
  }
}

void SessionView::onAddAnnotation() {
  QString note = m_AnnotationInput->text().trimmed();
  if (note.isEmpty()) return;

  if (m_isRecording) {
    m_AnnotationList->addItem(QString("[%1] %2").arg(QTime::currentTime().toString("HH:mm:ss")).arg(note));
    m_AnnotationInput->clear();
  }
}

void SessionView::updateStatus() {
  if (m_isRecording) {
    m_EventCountLabel->setText(QString("Events: %1").arg(m_AnnotationList->count() * 7 + 13));
  } else {
    m_EventCountLabel->setText("Events: 0");
  }
}

void SessionView::updateBiometrics() {
  if (!m_isRecording) {
    m_BrainwaveGraph->addValue(0);
    m_StressGraph->addValue(0);
    m_ChipRejectionGraph->addValue(0);
    return;
  }
  
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
  QMessageBox::information(this, "Export", "Report filed with the Department of Vigilance.");
}

void SessionView::onExportJson() {
  QMessageBox::information(this, "Export", "Report filed with the Department of Vigilance.");
}

} // namespace severance::gui::session_view
