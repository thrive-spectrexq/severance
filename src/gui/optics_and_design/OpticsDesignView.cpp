#include "OpticsDesignView.hpp"
#include "gui/theme/Theme.hpp"
#include <QFrame>
#include <QPainter>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QPushButton>
#include <QPolygonF>
#include <QPainterPath>
#include <tuple>

namespace severance::gui::optics_and_design {

// For the clickable widget
class ClickableFrame : public QFrame {
public:
  ClickableFrame(QWidget* parent = nullptr) : QFrame(parent) {}
  std::function<void()> onClicked;
protected:
  void mousePressEvent(QMouseEvent* event) override {
    if (event->button() == Qt::LeftButton && onClicked) {
      onClicked();
    }
  }
};

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

  auto* createBtn = new QPushButton("[ CREATE NEW O&D ARTWORK ]", this);
  createBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #161B22;
      color: #39FF14;
      border: 1px solid #30363D;
      padding: 8px;
      font-family: monospace;
      font-weight: bold;
    }
    QPushButton:hover {
      background-color: #39FF14;
      color: #0D1117;
    }
  )");
  connect(createBtn, &QPushButton::clicked, this, &OpticsDesignView::onCreateArtwork);
  layout->addWidget(createBtn);

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

  // Overlay
  m_Overlay = new QWidget(this);
  m_Overlay->setStyleSheet("background-color: rgba(13, 17, 23, 240);");
  auto* overlayLayout = new QVBoxLayout(m_Overlay);
  
  m_OverlayTitle = new QLabel(m_Overlay);
  m_OverlayTitle->setStyleSheet("color: white; font-size: 28px; font-weight: bold; margin-top: 20px; background-color: transparent;");
  m_OverlayTitle->setAlignment(Qt::AlignCenter);
  
  m_OverlayImage = new QLabel(m_Overlay);
  m_OverlayImage->setAlignment(Qt::AlignCenter);
  m_OverlayImage->setStyleSheet("background-color: transparent;");
  
  auto* closeBtn = new QPushButton("CLOSE INSPECTION", m_Overlay);
  closeBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #F85149;
      color: white;
      border: none;
      padding: 10px 20px;
      font-family: monospace;
      font-weight: bold;
      border-radius: 4px;
      margin-bottom: 20px;
    }
    QPushButton:hover {
      background-color: #DA3633;
    }
  )");
  connect(closeBtn, &QPushButton::clicked, [this]() { m_Overlay->hide(); });
  
  overlayLayout->addWidget(m_OverlayTitle);
  overlayLayout->addStretch();
  overlayLayout->addWidget(m_OverlayImage, 1);
  overlayLayout->addStretch();
  overlayLayout->addWidget(closeBtn, 0, Qt::AlignCenter);
  
  m_Overlay->hide();
}

void OpticsDesignView::resizeEvent(QResizeEvent* event) {
  if (m_Overlay) {
    m_Overlay->resize(event->size());
  }
  QWidget::resizeEvent(event);
}

QWidget* OpticsDesignView::createArtworkWidget(const QString& title, const QString& type, const QString& colorHex, const QPixmap& customPixmap) {
  auto* container = new ClickableFrame(this);
  container->setFixedSize(250, 320);
  container->setStyleSheet(QString(R"(
    QFrame {
      background-color: %1;
      border: 1px solid %2;
      border-radius: 8px;
    }
    QFrame:hover {
      border: 1px solid #39FF14;
    }
  )").arg(theme::Colors::Surface).arg(theme::Colors::Border));

  auto* layout = new QVBoxLayout(container);
  layout->setContentsMargins(12, 12, 12, 12);

  // Canvas
  auto* canvas = new QLabel(container);
  canvas->setFixedSize(224, 224);
  
  QPixmap pix;
  if (!customPixmap.isNull()) {
    pix = customPixmap;
  } else {
    pix = QPixmap(800, 800);
    pix.fill(QColor("#0D1117"));
    
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    
    QColor accentColor(colorHex);
    QLinearGradient grad(0, 0, 800, 800);
    grad.setColorAt(0, accentColor.darker(150));
    grad.setColorAt(1, accentColor.lighter(120));
    
    p.setBrush(grad);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(40, 40, 720, 720, 60, 60);
    
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(255,255,255, 60), 8));
    p.drawEllipse(120, 120, 560, 560);
    p.drawEllipse(240, 240, 320, 320);
    p.drawLine(400, 40, 400, 760);
    p.drawLine(40, 400, 760, 400);
  }
  
  canvas->setPixmap(pix.scaled(224, 224, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  layout->addWidget(canvas);

  // Text
  auto* lblTitle = new QLabel(title, container);
  lblTitle->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 14px; margin-top: 8px; background-color: transparent; border: none;").arg(theme::Colors::TextPrimary));
  lblTitle->setAlignment(Qt::AlignCenter);
  layout->addWidget(lblTitle);

  auto* lblType = new QLabel(type, container);
  lblType->setStyleSheet(QString("color: %1; font-size: 11px; background-color: transparent; border: none;").arg(theme::Colors::TextSecondary));
  lblType->setAlignment(Qt::AlignCenter);
  layout->addWidget(lblType);

  container->onClicked = [this, pix, title]() {
    onArtworkClicked(pix, title);
  };

  return container;
}

QPixmap OpticsDesignView::generateProceduralArtwork() {
  QPixmap pix(800, 800);
  pix.fill(QColor("#0D1117"));
  QPainter p(&pix);
  p.setRenderHint(QPainter::Antialiasing);

  auto rng = QRandomGenerator::global();
  
  // Background gradient
  QColor color1 = QColor::fromHsv(rng->bounded(360), rng->bounded(100, 200), rng->bounded(100, 200));
  QColor color2 = QColor::fromHsv(rng->bounded(360), rng->bounded(100, 200), rng->bounded(100, 200));
  QLinearGradient grad(0, 0, 800, 800);
  grad.setColorAt(0, color1);
  grad.setColorAt(1, color2);
  p.fillRect(0, 0, 800, 800, grad);

  // Draw some polygons and logos
  int shapes = rng->bounded(5, 15);
  for (int i = 0; i < shapes; ++i) {
    p.setBrush(QColor::fromHsv(rng->bounded(360), rng->bounded(150, 255), rng->bounded(150, 255), rng->bounded(100, 200)));
    p.setPen(Qt::NoPen);
    
    QPolygonF poly;
    int points = rng->bounded(3, 8);
    for (int j = 0; j < points; ++j) {
      poly << QPointF(rng->bounded(800), rng->bounded(800));
    }
    p.drawPolygon(poly);
  }
  
  // Draw abstract lumon drops/lines
  p.setBrush(Qt::NoBrush);
  p.setPen(QPen(QColor(255, 255, 255, 150), 3));
  for (int i = 0; i < 5; ++i) {
    int r = rng->bounded(50, 300);
    int cx = rng->bounded(100, 700);
    int cy = rng->bounded(100, 700);
    p.drawEllipse(QPoint(cx, cy), r, r);
    p.drawLine(cx, 0, cx, 800);
    p.drawLine(0, cy, 800, cy);
  }

  return pix;
}

void OpticsDesignView::onCreateArtwork() {
  QStringList titles = {
    "The Macrodata Incidence",
    "Grim Barbarity of O&D",
    "Grief and the Eagans",
    "Youthful Kier",
    "The Water Drop",
    "Helly's Rebellion"
  };
  QString title = titles.at(QRandomGenerator::global()->bounded(titles.size())) + " - " + QString::number(QRandomGenerator::global()->bounded(100, 999));
  
  QPixmap pix = generateProceduralArtwork();
  
  auto* w = createArtworkWidget(title, "PROCEDURAL GENERATION", "#000000", pix);
  
  m_GalleryLayout->addWidget(w, m_GalleryItemCount / 4, m_GalleryItemCount % 4);
  m_GalleryItemCount++;
}

void OpticsDesignView::onArtworkClicked(const QPixmap& pixmap, const QString& title) {
  m_OverlayTitle->setText(title);
  m_OverlayImage->setPixmap(pixmap.scaled(600, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  m_Overlay->show();
  m_Overlay->raise();
}

void OpticsDesignView::loadArtworks() {
  auto artworks = {
    std::make_tuple("Kier Overcomes Bad Hat", "HISTORICAL INDEX", theme::Colors::ChartBlue),
    std::make_tuple("The Four Tempers", "PSYCHOLOGICAL PROFILE", theme::Colors::ChartPurple),
    std::make_tuple("The Waffle Party of 1892", "INCENTIVE VISUALIZATION", theme::Colors::ChartOrange),
    std::make_tuple("The Lexington Incident", "SECURITY ANOMALY", theme::Colors::ChartRed)
  };
  
  for (const auto& a : artworks) {
    auto w = createArtworkWidget(std::get<0>(a), std::get<1>(a), std::get<2>(a));
    m_GalleryLayout->addWidget(w, m_GalleryItemCount / 4, m_GalleryItemCount % 4);
    m_GalleryItemCount++;
  }
}

} // namespace severance::gui::optics_and_design
