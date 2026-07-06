#include "PerimeterGridView.hpp"

#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QRandomGenerator>

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

    m_sensorTimer = new QTimer(this);
    connect(m_sensorTimer, &QTimer::timeout, this, &PerimeterGridView::randomizeSensors);
    m_sensorTimer->start(1500);
}

PerimeterGridView::~PerimeterGridView() = default;

void PerimeterGridView::setupScene() {
    m_scene->setSceneRect(-400, -300, 800, 600);

    // Create Departments
    createDepartment("Macrodata Refinement", QRectF(-350, -250, 300, 200), QColor("#002200"));
    createDepartment("Optics & Design", QRectF(50, -250, 300, 200), QColor("#001122"));
    createDepartment("Break Room", QRectF(-350, 50, 300, 200), QColor("#220000"));
    createDepartment("Management", QRectF(50, 50, 300, 200), QColor("#222200"));

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
    auto* rectItem = new QGraphicsRectItem(rect);
    rectItem->setPen(QPen(QColor("#00FF41").darker(200), 1));
    rectItem->setBrush(color);
    m_scene->addItem(rectItem);

    auto* textItem = new QGraphicsTextItem(name);
    textItem->setDefaultTextColor(QColor("#00FF41"));
    QFont font("Courier", 12, QFont::Bold);
    textItem->setFont(font);
    
    // Center text
    QRectF tr = textItem->boundingRect();
    textItem->setPos(rect.center().x() - tr.width() / 2, rect.center().y() - tr.height() / 2);
    m_scene->addItem(textItem);
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
