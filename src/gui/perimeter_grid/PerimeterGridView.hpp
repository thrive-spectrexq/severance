#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsObject>

namespace severance::gui::perimeter_grid {

class ElevatorItem : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(qreal floorProgress READ floorProgress WRITE setFloorProgress)

public:
    explicit ElevatorItem(QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    qreal floorProgress() const { return m_floorProgress; }
    void setFloorProgress(qreal progress);

    void toggleFloor();

private:
    qreal m_floorProgress{0.0}; // 0.0 = Severed, 1.0 = Non-Severed
    bool m_isSevered{true};
    QPropertyAnimation* m_anim;
};

class MotionSensorItem : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(qreal pingRadius READ pingRadius WRITE setPingRadius)
    Q_PROPERTY(qreal pingOpacity READ pingOpacity WRITE setPingOpacity)

public:
    explicit MotionSensorItem(QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void triggerPing();

    qreal pingRadius() const { return m_pingRadius; }
    void setPingRadius(qreal r) { m_pingRadius = r; update(); }

    qreal pingOpacity() const { return m_pingOpacity; }
    void setPingOpacity(qreal o) { m_pingOpacity = o; update(); }

private:
    qreal m_pingRadius{0.0};
    qreal m_pingOpacity{0.0};
    QPropertyAnimation* m_radiusAnim;
    QPropertyAnimation* m_opacityAnim;
};


class PerimeterGridView : public QGraphicsView {
    Q_OBJECT

public:
    explicit PerimeterGridView(QWidget* parent = nullptr);
    ~PerimeterGridView() override;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private slots:
    void randomizeSensors();

private:
    void setupScene();
    void createDepartment(const QString& name, const QRectF& rect, const QColor& color);

    QGraphicsScene* m_scene;
    ElevatorItem* m_elevator;
    QList<MotionSensorItem*> m_sensors;
    QTimer* m_sensorTimer;
};

} // namespace severance::gui::perimeter_grid
