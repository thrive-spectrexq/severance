#include "OpticsDesignView.hpp"
#include "gui/theme/Theme.hpp"
#include <QFrame>
#include <QPainter>
#include <QLinearGradient>

namespace severance::gui::optics_and_design {

OpticsDesignView::OpticsDesignView(QWidget* parent) : QWidget(parent) {
  setupUI();
  loadArtworks();
}

OpticsDesignView::~OpticsDesignView() = default;

void OpticsDesignView::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(20);

  auto* title = new QLabel("OPTICS & DESIGN", this);
  title->setStyleSheet(QString("color: %1; font-size: 24px; font-weight: 300; letter-spacing: 4px;").arg(theme::Colors::TextPrimary));
  layout->addWidget(title);

  auto* subtitle = new QLabel("Curation and rendering of visual anomalies and departmental aesthetics.", this);
  subtitle->setStyleSheet(QString("color: %1; font-size: 14px; font-style: italic;").arg(theme::Colors::TextSecondary));
  layout->addWidget(subtitle);

  m_ScrollArea = new QScrollArea(this);
  m_ScrollArea->setWidgetResizable(true);
  m_ScrollArea->setFrameShape(QFrame::NoFrame);
  m_ScrollArea->setStyleSheet("background-color: transparent;");

  auto* galleryWidget = new QWidget(m_ScrollArea);
  galleryWidget->setStyleSheet("background-color: transparent;");
  
  m_GalleryLayout = new QGridLayout(galleryWidget);
  m_GalleryLayout->setSpacing(24);
  
  m_ScrollArea->setWidget(galleryWidget);
  layout->addWidget(m_ScrollArea);
}

QWidget* OpticsDesignView::createArtworkWidget(const QString& title, const QString& type, const QString& colorHex) {
  auto* container = new QFrame(this);
  container->setFixedSize(250, 320);
  container->setStyleSheet(QString(R"(
    QFrame {
      background-color: %1;
      border: 1px solid %2;
      border-radius: 8px;
    }
  )").arg(theme::Colors::Surface).arg(theme::Colors::Border));

  auto* layout = new QVBoxLayout(container);
  layout->setContentsMargins(12, 12, 12, 12);

  // Canvas
  auto* canvas = new QLabel(container);
  canvas->setFixedSize(224, 224);
  
  QPixmap pix(224, 224);
  pix.fill(QColor("#0D1117"));
  
  QPainter p(&pix);
  p.setRenderHint(QPainter::Antialiasing);
  
  QColor accentColor(colorHex);
  QLinearGradient grad(0, 0, 224, 224);
  grad.setColorAt(0, accentColor.darker(150));
  grad.setColorAt(1, accentColor.lighter(120));
  
  p.setBrush(grad);
  p.setPen(Qt::NoPen);
  p.drawRoundedRect(12, 12, 200, 200, 16, 16);
  
  // Draw some abstract geometry
  p.setBrush(Qt::NoBrush);
  p.setPen(QPen(QColor(255,255,255, 60), 2));
  p.drawEllipse(30, 30, 164, 164);
  p.drawEllipse(60, 60, 104, 104);
  p.drawLine(112, 12, 112, 212);
  p.drawLine(12, 112, 212, 112);
  
  canvas->setPixmap(pix);
  layout->addWidget(canvas);

  // Text
  auto* lblTitle = new QLabel(title, container);
  lblTitle->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 14px; margin-top: 8px;").arg(theme::Colors::TextPrimary));
  lblTitle->setAlignment(Qt::AlignCenter);
  layout->addWidget(lblTitle);

  auto* lblType = new QLabel(type, container);
  lblType->setStyleSheet(QString("color: %1; font-size: 11px;").arg(theme::Colors::TextSecondary));
  lblType->setAlignment(Qt::AlignCenter);
  layout->addWidget(lblType);

  return container;
}

void OpticsDesignView::loadArtworks() {
  m_GalleryLayout->addWidget(createArtworkWidget("The Kier Collection", "HISTORICAL INDEX", theme::Colors::ChartBlue), 0, 0);
  m_GalleryLayout->addWidget(createArtworkWidget("Macrodata Topography", "NEURAL MAP", theme::Colors::ChartPurple), 0, 1);
  m_GalleryLayout->addWidget(createArtworkWidget("Waffle Party", "INCENTIVE VISUALIZATION", theme::Colors::ChartOrange), 0, 2);
  m_GalleryLayout->addWidget(createArtworkWidget("The Break Room", "PSYCHOLOGICAL PROFILE", theme::Colors::ChartRed), 0, 3);
  m_GalleryLayout->addWidget(createArtworkWidget("Perimeter Scan", "SECURITY FEED", theme::Colors::ChartGreen), 1, 0);
  m_GalleryLayout->addWidget(createArtworkWidget("Defiant Jazz", "AUDITORY ANOMALY", theme::Colors::ChartYellow), 1, 1);
}

} // namespace severance::gui::optics_and_design
