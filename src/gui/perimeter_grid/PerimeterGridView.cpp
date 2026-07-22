#include "PerimeterGridView.hpp"
#include "core/game/GameEngine.hpp"

#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QRandomGenerator>
#include <QGraphicsSceneMouseEvent>

namespace severance::gui::perimeter_grid {

// ---------------------------------------------------------
// RoomItem
// ---------------------------------------------------------

RoomItem::RoomItem(const QString& name, const QRectF& rect, const QColor& color, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_name(name), m_rect(rect), m_color(color)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
}

QRectF RoomItem::boundingRect() const {
    return m_rect;
}

void RoomItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->setPen(QPen(QColor("#00FF41").darker(200), 1));
    painter->setBrush(m_color);
    painter->drawRect(m_rect);

    painter->setPen(m_locked ? Qt::red : QColor("#00FF41"));
    QFont font("Courier", 12, QFont::Bold);
    painter->setFont(font);
    
    QRectF tr = painter->fontMetrics().boundingRect(m_name);
    painter->drawText(m_rect.center() - QPointF(tr.width() / 2, -tr.height() / 2), m_name);
    
    if (m_locked) {
        QRectF lr = painter->fontMetrics().boundingRect("LOCKED");
        painter->drawText(m_rect.center() - QPointF(lr.width() / 2, -tr.height() / 2 - 20), "LOCKED");
    }
}

void RoomItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(this);
    }
    QGraphicsObject::mousePressEvent(event);
}

// ---------------------------------------------------------
// ElevatorItem
// ---------------------------------------------------------

ElevatorItem::ElevatorItem(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    m_anim = new QPropertyAnimation(this, "floorProgress", this);
    m_anim->setDuration(2000);
    m_anim->setEasingCurve(QEasingCurve::InOutCubic);
}

QRectF ElevatorItem::boundingRect() const {
    return QRectF(-60, -60, 120, 120);
}

void ElevatorItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    // Elevator shaft
    painter->setPen(QPen(QColor("#00FF41"), 2));
    painter->setBrush(QColor("#0A0A0A"));
    painter->drawRect(-50, -50, 100, 100);

    // Inner cabin (moves based on floorProgress)
    // 0.0 is Severed Floor, 1.0 is Non-Severed Floor
    int yOffset = static_cast<int>(m_floorProgress * 40.0);
    
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#00FF41").darker(150));
    painter->drawRect(-40, -40 + yOffset, 80, 80 - yOffset);

    // Text
    painter->setPen(QColor("#00FF41"));
    QFont font("Courier", 8, QFont::Bold);
    painter->setFont(font);

    QString text = m_floorProgress < 0.5 ? "SEVERED\nFLOOR" : "NON\nSEVERED";
    painter->drawText(boundingRect(), Qt::AlignCenter, text);
}

void ElevatorItem::setFloorProgress(qreal progress) {
    m_floorProgress = progress;
    update();
}

void ElevatorItem::toggleFloor() {
    if (m_anim->state() == QAbstractAnimation::Running)
        return;

    m_isSevered = !m_isSevered;
    m_anim->setStartValue(m_floorProgress);
    m_anim->setEndValue(m_isSevered ? 0.0 : 1.0);
    m_anim->start();
}

// ---------------------------------------------------------
// MotionSensorItem
// ---------------------------------------------------------

MotionSensorItem::MotionSensorItem(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    m_radiusAnim = new QPropertyAnimation(this, "pingRadius", this);
    m_radiusAnim->setDuration(1000);
    m_radiusAnim->setEasingCurve(QEasingCurve::OutQuad);

    m_opacityAnim = new QPropertyAnimation(this, "pingOpacity", this);
    m_opacityAnim->setDuration(1000);
    m_opacityAnim->setEasingCurve(QEasingCurve::OutQuad);
}

QRectF MotionSensorItem::boundingRect() const {
    return QRectF(-30, -30, 60, 60);
}

void MotionSensorItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    // Base sensor dot
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#00FF41"));
    painter->drawEllipse(QPointF(0, 0), 3, 3);

    if (m_pingOpacity > 0.0) {
        QColor pingColor("#00FF41");
        pingColor.setAlphaF(m_pingOpacity);
        painter->setPen(QPen(pingColor, 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0, 0), m_pingRadius, m_pingRadius);
    }
}

void MotionSensorItem::triggerPing() {
    m_radiusAnim->setStartValue(3.0);
    m_radiusAnim->setEndValue(25.0);
    m_radiusAnim->start();

    m_opacityAnim->setStartValue(1.0);
    m_opacityAnim->setEndValue(0.0);
    m_opacityAnim->start();
}


// ---------------------------------------------------------
// PerimeterGridView
// ---------------------------------------------------------

PerimeterGridView::PerimeterGridView(QWidget* parent)
    : QGraphicsView(parent), m_scene(new QGraphicsScene(this))
{
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(QColor("#050505"));
    setFrameShape(QFrame::NoFrame);

    setupScene();
    setupUI();

    m_sensorTimer = new QTimer(this);
    connect(m_sensorTimer, &QTimer::timeout, this, &PerimeterGridView::randomizeSensors);
    m_sensorTimer->start(1500);
}

PerimeterGridView::~PerimeterGridView() = default;

void PerimeterGridView::setupUI() {
    m_otcButton = new QPushButton("OVERTIME CONTINGENCY (OTC)", this);
    m_otcButton->setStyleSheet(
        "QPushButton { background-color: #550000; color: #FF0000; font-family: 'Courier New'; font-weight: bold; font-size: 16px; border: 2px solid #FF0000; padding: 10px; }"
        "QPushButton:hover { background-color: #880000; }"
    );
    connect(m_otcButton, &QPushButton::clicked, this, &PerimeterGridView::toggleOTC);

    m_telemetryPanel = new QFrame(this);
    m_telemetryPanel->setStyleSheet("QFrame { background-color: rgba(10, 10, 10, 200); border: 1px solid #00FF41; }");
    m_telemetryPanel->setVisible(false);
    
    auto* layout = new QVBoxLayout(m_telemetryPanel);
    m_telemetryLabel = new QLabel(m_telemetryPanel);
    m_telemetryLabel->setStyleSheet("QLabel { color: #00FF41; font-family: 'Courier New'; font-size: 14px; border: none; background: transparent; }");
    layout->addWidget(m_telemetryLabel);

    m_otcTimer = new QTimer(this);
    connect(m_otcTimer, &QTimer::timeout, this, &PerimeterGridView::onOTCPulse);
}

void PerimeterGridView::toggleOTC() {
    m_otcActive = !m_otcActive;
    
    if (m_otcActive) {
        core::game::GameEngine::GetInstance().TriggerOtcSwitch("Perimeter Security Suite");
        m_otcButton->setStyleSheet(
            "QPushButton { background-color: #FF0000; color: #FFFFFF; font-family: 'Courier New'; font-weight: bold; font-size: 16px; border: 2px solid #FFFFFF; padding: 10px; }"
        );
        m_otcTimer->start(500);
        for (auto* room : m_rooms) {
            room->setLocked(true);
        }
    } else {
        m_otcButton->setStyleSheet(
            "QPushButton { background-color: #550000; color: #FF0000; font-family: 'Courier New'; font-weight: bold; font-size: 16px; border: 2px solid #FF0000; padding: 10px; }"
            "QPushButton:hover { background-color: #880000; }"
        );
        m_otcTimer->stop();
        m_otcFlash = false;
        viewport()->update();
        for (auto* room : m_rooms) {
            room->setLocked(false);
        }
    }
}

void PerimeterGridView::onOTCPulse() {
    m_otcFlash = !m_otcFlash;
    viewport()->update();
}

void PerimeterGridView::onRoomClicked(RoomItem* room) {
    m_telemetryPanel->setVisible(true);
    QString info = QString("ROOM: %1\n\n"
                           "INNIE COUNT: %2\n"
                           "CLEARANCE: LEVEL %3\n"
                           "CAMERA: LIVE FEED — 1080p CRT\n"
                           "DOOR STATUS: %4")
                           .arg(room->name())
                           .arg(QRandomGenerator::global()->bounded(1, 10))
                           .arg(QRandomGenerator::global()->bounded(2, 5))
                           .arg(room->isLocked() ? "LOCKED" : "UNLOCKED");
    m_telemetryLabel->setText(info);
}

void PerimeterGridView::setupScene() {
    m_scene->setSceneRect(-400, -300, 800, 600);

    // Create Departments
    createDepartment("Macrodata Refinement", QRectF(-350, -250, 300, 200), QColor("#002200"));
    createDepartment("Optics & Design", QRectF(50, -250, 300, 200), QColor("#001122"));
    createDepartment("Break Room", QRectF(-350, 50, 300, 200), QColor("#220000"));
    createDepartment("Management", QRectF(50, 50, 300, 200), QColor("#222200"));
    createDepartment("Testing Floor Elevator", QRectF(-150, -300, 300, 40), QColor("#111111"));
    createDepartment("Executive Suite", QRectF(-350, -300, 180, 40), QColor("#330033"));
    createDepartment("Goat Room", QRectF(170, -300, 180, 40), QColor("#333300"));

    // Center Elevator
    m_elevator = new ElevatorItem();
    m_elevator->setPos(0, 0);
    m_scene->addItem(m_elevator);

    // Add Motion Sensors randomly within departments
    for (int i = 0; i < 20; ++i) {
        auto* sensor = new MotionSensorItem();
        
        // Randomly place but avoid center elevator
        qreal x, y;
        do {
            x = QRandomGenerator::global()->bounded(700.0) - 350.0;
            y = QRandomGenerator::global()->bounded(500.0) - 250.0;
        } while (qAbs(x) < 80 && qAbs(y) < 80);

        sensor->setPos(x, y);
        m_scene->addItem(sensor);
        m_sensors.append(sensor);
    }
}

void PerimeterGridView::createDepartment(const QString& name, const QRectF& rect, const QColor& color) {
    auto* room = new RoomItem(name, rect, color);
    connect(room, &RoomItem::clicked, this, &PerimeterGridView::onRoomClicked);
    m_scene->addItem(room);
    m_rooms.append(room);
}

void PerimeterGridView::randomizeSensors() {
    int activeCount = QRandomGenerator::global()->bounded(1, 5);
    for (int i = 0; i < activeCount; ++i) {
        int index = QRandomGenerator::global()->bounded(m_sensors.size());
        m_sensors[index]->triggerPing();
    }

    // Occasional elevator toggle
    if (QRandomGenerator::global()->bounded(100) < 10) {
        m_elevator->toggleFloor();
    }
}

void PerimeterGridView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);

    m_otcButton->setGeometry(width() / 2 - 150, 20, 300, 40);
    m_telemetryPanel->setGeometry(width() - 320, 20, 300, 200);
}

void PerimeterGridView::drawBackground(QPainter* painter, const QRectF& rect) {
    QGraphicsView::drawBackground(painter, rect);

    // Draw grid lines
    painter->setPen(QPen(QColor("#00FF41").darker(300), 1, Qt::DotLine));
    
    int gridStep = 50;
    qreal left = int(rect.left()) - (int(rect.left()) % gridStep);
    qreal top = int(rect.top()) - (int(rect.top()) % gridStep);

    QVarLengthArray<QLineF, 100> lines;
    for (qreal x = left; x < rect.right(); x += gridStep)
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (qreal y = top; y < rect.bottom(); y += gridStep)
        lines.append(QLineF(rect.left(), y, rect.right(), y));

    painter->drawLines(lines.data(), lines.size());
}

void PerimeterGridView::drawForeground(QPainter* painter, const QRectF& rect) {
    QGraphicsView::drawForeground(painter, rect);
    
    if (m_otcActive && m_otcFlash) {
        painter->fillRect(rect, QColor(255, 0, 0, 80));
    }
}

} // namespace severance::gui::perimeter_grid
