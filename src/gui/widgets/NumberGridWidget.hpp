#pragma once

#include <QWidget>
#include <QTimer>
#include <QPointF>
#include <QRectF>
#include <QFont>
#include <vector>
#include <unordered_map>
#include <array>
#include <QString>
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
    int targetBin = -1;
    std::vector<QPointF> trail;
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
    void mouseReleaseEvent(QMouseEvent* event) override;
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

    void showFileCabinet();
    void drawAuras(QPainter& painter, int minX, int minY, int maxX, int maxY);
    void drawMDEOverlay(QPainter& painter);

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
    QPoint m_LastMousePos{-1, -1};
    bool m_IsDragging = false;
    
    // Idle mode
    int m_IdleCounter = 0;
    bool m_IdleMode = false;
    static constexpr int IdleTimeoutTicks = 300; // e.g. 10 seconds at 30fps (approx 33ms timer)

    // Files & Quotas
    std::vector<QString> m_MdrFiles{"COLDWATER", "TUMWATER", "CULPEPPER", "DILLON", "SIENA", "PACOIMA"};
    int m_CurrentFileIndex = 0;
    bool m_QuotaCompleted = false;
    
    // MDE & Waffle Party
    bool m_MdeMode = false;
    int m_CurrentSong = 0;
    bool m_WaffleAccepted = false;
    QRectF m_BtnDefiantJazz;
    QRectF m_BtnKierHymn;
    QRectF m_BtnLumonLounge;
    QRectF m_BtnWaffle;

    // Progress
    int m_TotalBadGroups = 0;
    std::array<int, 5> m_BinCounts{0, 0, 0, 0, 0};
    
    // Bin geometry
    std::array<QRectF, 5> m_BinRects;
    std::array<QString, 5> m_BinNames{"WO", "FC", "DR", "MA", "SI"};
    std::array<QString, 5> m_BinFullNames{"Woe", "Frolic", "Dread", "Malice", "Siena"};
    
    // Font
    QFont m_Font;
    void switchFile(int index);
};

} // namespace severance::gui::widgets
