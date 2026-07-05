#pragma once

#include <QWidget>
#include <QTimer>
#include <QPointF>
#include <QRectF>
#include <QFont>
#include <vector>
#include <unordered_map>
#include "utils/PerlinNoise.hpp"

namespace severance::gui::widgets {

struct NumberCell {
    int value = 0;
    bool isBad = false;
    int groupId = -1;
    bool isRefined = false;
    QPointF currentOffset{0, 0};
    double scale = 1.0;
    
    // For bin animation
    bool isAnimating = false;
    QPointF animPos{0, 0};
};

class NumberGridWidget : public QWidget {
    Q_OBJECT
public:
    explicit NumberGridWidget(QWidget* parent = nullptr);
    ~NumberGridWidget() override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onTimerUpdate();

private:
    void generateGrid();
    void identifyGroups();
    void activateRandomGroup();
    void refineGroup(int groupId);
    void drawLumonLogo(QPainter& painter);
    void resetIdleTimer();
    QPoint mapToGrid(const QPoint& pos) const;

    static constexpr int GridCols = 60;
    static constexpr int GridRows = 40;
    static constexpr int CellSize = 35;

    std::vector<std::vector<NumberCell>> m_Grid;
    int m_NextGroupId = 1;
    std::unordered_map<int, std::vector<QPoint>> m_Groups; // groupId -> cell coords
    
    int m_ActiveGroupId = -1;
    int m_HoveredGroupId = -1;
    
    // Viewport
    QPointF m_CameraPos{0, 0};
    double m_Zoom{1.0};
    
    // Noise and animation
    utils::PerlinNoise m_Noise;
    double m_Time{0.0};
    QTimer* m_Timer{nullptr};
    QPoint m_MousePos{-1, -1};
    
    // Idle mode
    int m_IdleCounter = 0;
    bool m_IdleMode = false;
    static constexpr int IdleTimeoutTicks = 300; // e.g. 10 seconds at 30fps (approx 33ms timer)

    // Progress
    int m_RefinedCount = 0;
    int m_TotalBadGroups = 0;
    
    // Bin geometry
    QRectF m_BinRect;
    
    // Font
    QFont m_Font;
};

} // namespace severance::gui::widgets
