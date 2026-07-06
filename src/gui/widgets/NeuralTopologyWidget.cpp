#include "NeuralTopologyWidget.hpp"
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QWheelEvent>
#include <QDebug>
#include <QtMath>
#include <QGraphicsDropShadowEffect>

namespace severance::gui::widgets {

// --- NeuralNode ---

NeuralNode::NeuralNode(const QString& id, const QColor& color, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_id(id), m_color(color) {
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable);
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setZValue(2);
}

QRectF NeuralNode::boundingRect() const {
    qreal d = 20.0 + m_glowRadius;
    return QRectF(-d, -d, d * 2, d * 2);
}

void NeuralNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    // Glow
    if (m_glowRadius > 0) {
        QRadialGradient gradient(0, 0, 15 + m_glowRadius);
        QColor glowColor = m_color;
        glowColor.setAlpha(100);
        gradient.setColorAt(0, glowColor);
        gradient.setColorAt(1, Qt::transparent);
        painter->setBrush(gradient);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), 15 + m_glowRadius, 15 + m_glowRadius);
    }

    // Core
    painter->setBrush(m_color);
    QPen pen(m_hovered ? Qt::white : Qt::black, 2);
    painter->setPen(pen);
    painter->drawEllipse(QPointF(0, 0), 10, 10);
    
    // Pulse ring
    painter->setBrush(Qt::NoBrush);
    QPen ringPen(m_color, 1);
    painter->setPen(ringPen);
    painter->drawEllipse(QPointF(0, 0), 14, 14);

    // Label
    painter->setPen(Qt::white);
    QFont f = painter->font();
    f.setPointSize(8);
    painter->setFont(f);
    painter->drawText(QRectF(-40, 18, 80, 20), Qt::AlignCenter, m_id);
}

void NeuralNode::triggerPulse() {
    QPropertyAnimation* anim = new QPropertyAnimation(this, "glowRadius", this);
    anim->setDuration(600);
    anim->setStartValue(0.0);
    anim->setKeyValueAt(0.5, 15.0);
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InOutSine);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void NeuralNode::setGlowRadius(qreal r) {
    if (!qFuzzyCompare(m_glowRadius, r)) {
        m_glowRadius = r;
        update();
    }
}

void NeuralNode::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    m_hovered = true;
    update();
    QGraphicsObject::hoverEnterEvent(event);
}

void NeuralNode::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    m_hovered = false;
    update();
    QGraphicsObject::hoverLeaveEvent(event);
}

QVariant NeuralNode::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged) {
        // notify links
        for (auto* item : scene()->items()) {
            if (auto* link = qobject_cast<SynapticLink*>(item->toGraphicsObject())) {
                link->updatePosition();
            }
        }
    }
    return QGraphicsObject::itemChange(change, value);
}

// --- SynapticLink ---

SynapticLink::SynapticLink(NeuralNode* source, NeuralNode* target, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_source(source), m_target(target), m_color(QColor("#0088aa")) {
    setZValue(1);
    updatePosition();
}

QRectF SynapticLink::boundingRect() const {
    if (!m_source || !m_target) return QRectF();
    return QRectF(m_sourcePoint, QSizeF(m_targetPoint.x() - m_sourcePoint.x(),
                                      m_targetPoint.y() - m_sourcePoint.y())).normalized().adjusted(-10, -10, 10, 10);
}

void SynapticLink::updatePosition() {
    if (!m_source || !m_target) return;
    prepareGeometryChange();
    m_sourcePoint = m_source->scenePos();
    m_targetPoint = m_target->scenePos();
    update();
}

void SynapticLink::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    if (!m_source || !m_target) return;

    painter->setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.moveTo(m_sourcePoint);
    path.lineTo(m_targetPoint);

    painter->setPen(QPen(m_color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawPath(path);

    if (m_pulsePos >= 0.0 && m_pulsePos <= 1.0) {
        QPointF pulsePoint(
            m_sourcePoint.x() + (m_targetPoint.x() - m_sourcePoint.x()) * m_pulsePos,
            m_sourcePoint.y() + (m_targetPoint.y() - m_sourcePoint.y()) * m_pulsePos
        );
        painter->setBrush(QColor("#00FFcc"));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(pulsePoint, 4, 4);
    }
}

void SynapticLink::setPulsePos(qreal p) {
    if (!qFuzzyCompare(m_pulsePos, p)) {
        m_pulsePos = p;
        update();
    }
}

void SynapticLink::triggerPulse() {
    QPropertyAnimation* anim = new QPropertyAnimation(this, "pulsePos", this);
    anim->setDuration(800);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
    
    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        m_pulsePos = -1.0;
        update();
        if (m_target) m_target->triggerPulse();
    });
}

// --- NeuralTopologyWidget ---

NeuralTopologyWidget::NeuralTopologyWidget(QWidget* parent)
    : QGraphicsView(parent) {
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setBackgroundBrush(QColor("#0A0A0E")); // Lumon dark background
    setFrameShape(QFrame::NoFrame);

    m_activityTimer = new QTimer(this);
    connect(m_activityTimer, &QTimer::timeout, this, &NeuralTopologyWidget::triggerRandomActivity);
    m_activityTimer->start(1500);
}

NeuralTopologyWidget::~NeuralTopologyWidget() {
}

void NeuralTopologyWidget::addNode(const QString& id, const QPointF& pos, const QColor& color) {
    NeuralNode* node = new NeuralNode(id, color);
    node->setPos(pos);
    m_scene->addItem(node);
    m_nodes.push_back(node);
}

void NeuralTopologyWidget::addLink(const QString& sourceId, const QString& targetId) {
    NeuralNode* source = nullptr;
    NeuralNode* target = nullptr;

    for (auto* node : m_nodes) {
        if (node->id() == sourceId) source = node;
        if (node->id() == targetId) target = node;
    }

    if (source && target) {
        SynapticLink* link = new SynapticLink(source, target);
        m_scene->addItem(link);
        m_links.push_back(link);
    }
}

void NeuralTopologyWidget::layoutNodesCircular(qreal radius) {
    if (m_nodes.empty()) return;
    qreal angleStep = 2.0 * M_PI / m_nodes.size();
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        qreal angle = i * angleStep;
        qreal x = radius * qCos(angle);
        qreal y = radius * qSin(angle);
        m_nodes[i]->setPos(x, y);
    }
}

void NeuralTopologyWidget::triggerRandomActivity() {
    if (m_links.empty()) return;
    int index = QRandomGenerator::global()->bounded(static_cast<int>(m_links.size()));
    m_links[index]->triggerPulse();
}

void NeuralTopologyWidget::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    if (m_scene->itemsBoundingRect().isEmpty()) {
        m_scene->setSceneRect(-width() / 2.0, -height() / 2.0, width(), height());
    } else {
        m_scene->setSceneRect(m_scene->itemsBoundingRect().adjusted(-100, -100, 100, 100));
    }
}

void NeuralTopologyWidget::drawBackground(QPainter* painter, const QRectF& rect) {
    // Lumon subtle grid
    QColor gridColor("#1A1A24");
    painter->setPen(QPen(gridColor, 1, Qt::SolidLine));

    qreal gridSize = 40.0;
    qreal left = int(rect.left()) - (int(rect.left()) % int(gridSize));
    qreal top = int(rect.top()) - (int(rect.top()) % int(gridSize));

    for (qreal x = left; x < rect.right(); x += gridSize) {
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    }
    for (qreal y = top; y < rect.bottom(); y += gridSize) {
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }
}

void NeuralTopologyWidget::wheelEvent(QWheelEvent* event) {
    if (event->modifiers() & Qt::ControlModifier) {
        double angle = event->angleDelta().y();
        double factor = qPow(1.0015, angle);
        scale(factor, factor);
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

} // namespace severance::gui::widgets
