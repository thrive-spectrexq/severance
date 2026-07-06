#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QPropertyAnimation>
#include <QTimer>
#include <vector>
#include <memory>

class NeuralNode : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(qreal glowRadius READ glowRadius WRITE setGlowRadius)
    Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
    explicit NeuralNode(const QString& id, const QColor& color, QGraphicsItem* parent = nullptr);
    ~NeuralNode() override = default;

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    QString id() const { return m_id; }
    void triggerPulse();

    qreal glowRadius() const { return m_glowRadius; }
    void setGlowRadius(qreal r);

    void setStatus(bool active) { m_active = active; update(); }
    bool isActive() const { return m_active; }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QString m_id;
    QColor m_color;
    qreal m_glowRadius{0.0};
    bool m_active{true};
    bool m_hovered{false};
};

class SynapticLink : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(qreal pulsePos READ pulsePos WRITE setPulsePos)

public:
    SynapticLink(NeuralNode* source, NeuralNode* target, QGraphicsItem* parent = nullptr);
    ~SynapticLink() override = default;

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void updatePosition();

    qreal pulsePos() const { return m_pulsePos; }
    void setPulsePos(qreal p);

    void triggerPulse();

private:
    NeuralNode* m_source;
    NeuralNode* m_target;
    QPointF m_sourcePoint;
    QPointF m_targetPoint;
    qreal m_pulsePos{-1.0}; // -1 means no pulse active
    QColor m_color;
};

class NeuralTopologyWidget : public QGraphicsView {
    Q_OBJECT

public:
    explicit NeuralTopologyWidget(QWidget* parent = nullptr);
    ~NeuralTopologyWidget() override;

    void addNode(const QString& id, const QPointF& pos, const QColor& color = QColor("#00FFcc"));
    void addLink(const QString& sourceId, const QString& targetId);
    void triggerRandomActivity();
    void layoutNodesCircular(qreal radius);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QGraphicsScene* m_scene;
    std::vector<NeuralNode*> m_nodes;
    std::vector<SynapticLink*> m_links;
    QTimer* m_activityTimer;
};
