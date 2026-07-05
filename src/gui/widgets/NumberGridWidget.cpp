#include "NumberGridWidget.hpp"
#include "gui/theme/Theme.hpp"
#include <QPainter>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QQueue>
#include <cmath>
#include <random>

namespace severance::gui::widgets {

NumberGridWidget::NumberGridWidget(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_Font = QFont("Courier New", 14, QFont::Bold);

    generateGrid();

    m_Timer = new QTimer(this);
    connect(m_Timer, &QTimer::timeout, this, &NumberGridWidget::onTimerUpdate);
    m_Timer->start(33); // ~30 FPS
}

NumberGridWidget::~NumberGridWidget() = default;

void NumberGridWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    
    // Center camera
    m_CameraPos = QPointF(GridCols * CellSize / 2.0 - width() / 2.0,
                          GridRows * CellSize / 2.0 - height() / 2.0);
}

void NumberGridWidget::generateGrid() {
    std::mt19random_engine rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 9);
    
    m_Grid.assign(GridRows, std::vector<NumberCell>(GridCols));
    for (int y = 0; y < GridRows; ++y) {
        for (int x = 0; x < GridCols; ++x) {
            m_Grid[y][x].value = dist(rng);
            // Threshold for bad numbers
            double n = m_Noise.noise(x * 0.1, y * 0.1);
            if (n > 0.15) {
                m_Grid[y][x].isBad = true;
            }
        }
    }
    
    identifyGroups();
    
    // Set a random group active if any exist
    if (!m_Groups.empty()) {
        activateRandomGroup();
    }
}

void NumberGridWidget::identifyGroups() {
    m_Groups.clear();
    for (int y = 0; y < GridRows; ++y) {
        for (int x = 0; x < GridCols; ++x) {
            m_Grid[y][x].groupId = -1;
        }
    }
    
    m_NextGroupId = 1;
    m_TotalBadGroups = 0;
    
    // BFS
    int dx[] = {-1, 1, 0, 0, -1, 1, -1, 1};
    int dy[] = {0, 0, -1, 1, -1, -1, 1, 1};
    
    for (int y = 0; y < GridRows; ++y) {
        for (int x = 0; x < GridCols; ++x) {
            if (m_Grid[y][x].isBad && !m_Grid[y][x].isRefined && m_Grid[y][x].groupId == -1) {
                int currentGroup = m_NextGroupId++;
                QQueue<QPoint> q;
                q.enqueue(QPoint(x, y));
                m_Grid[y][x].groupId = currentGroup;
                std::vector<QPoint> groupCells;
                
                while (!q.isEmpty()) {
                    QPoint p = q.dequeue();
                    groupCells.push_back(p);
                    
                    for (int i = 0; i < 8; ++i) {
                        int nx = p.x() + dx[i];
                        int ny = p.y() + dy[i];
                        if (nx >= 0 && nx < GridCols && ny >= 0 && ny < GridRows) {
                            if (m_Grid[ny][nx].isBad && !m_Grid[ny][nx].isRefined && m_Grid[ny][nx].groupId == -1) {
                                m_Grid[ny][nx].groupId = currentGroup;
                                q.enqueue(QPoint(nx, ny));
                            }
                        }
                    }
                }
                
                // Only count large enough groups
                if (groupCells.size() >= 4) {
                    m_Groups[currentGroup] = groupCells;
                    m_TotalBadGroups++;
                } else {
                    for (auto& p : groupCells) {
                        m_Grid[p.y()][p.x()].groupId = -1;
                        m_Grid[p.y()][p.x()].isBad = false; // Discard tiny groups
                    }
                }
            }
        }
    }
}

void NumberGridWidget::activateRandomGroup() {
    if (m_Groups.empty()) {
        m_ActiveGroupId = -1;
        return;
    }
    
    std::vector<int> keys;
    for (const auto& pair : m_Groups) {
        keys.push_back(pair.first);
    }
    
    std::mt19random_engine rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, keys.size() - 1);
    m_ActiveGroupId = keys[dist(rng)];
}

void NumberGridWidget::refineGroup(int groupId) {
    if (m_Groups.find(groupId) == m_Groups.end()) return;
    
    for (const auto& p : m_Groups[groupId]) {
        auto& cell = m_Grid[p.y()][p.x()];
        cell.isAnimating = true;
        // Calculate initial screen pos
        cell.animPos = QPointF(p.x() * CellSize - m_CameraPos.x(), p.y() * CellSize - m_CameraPos.y()) * m_Zoom;
    }
    m_RefinedCount++;
    m_Groups.erase(groupId);
    if (m_ActiveGroupId == groupId) {
        m_ActiveGroupId = -1;
        m_HoveredGroupId = -1;
    }
}

QPoint NumberGridWidget::mapToGrid(const QPoint& pos) const {
    double wx = pos.x() / m_Zoom + m_CameraPos.x();
    double wy = pos.y() / m_Zoom + m_CameraPos.y();
    
    int cx = static_cast<int>(std::floor(wx / CellSize));
    int cy = static_cast<int>(std::floor(wy / CellSize));
    return QPoint(cx, cy);
}

void NumberGridWidget::resetIdleTimer() {
    m_IdleCounter = 0;
    m_IdleMode = false;
}

void NumberGridWidget::keyPressEvent(QKeyEvent* event) {
    resetIdleTimer();
    const double panSpeed = 30.0 / m_Zoom;
    
    switch (event->key()) {
        case Qt::Key_Left:  m_CameraPos.rx() -= panSpeed; break;
        case Qt::Key_Right: m_CameraPos.rx() += panSpeed; break;
        case Qt::Key_Up:    m_CameraPos.ry() -= panSpeed; break;
        case Qt::Key_Down:  m_CameraPos.ry() += panSpeed; break;
        default: QWidget::keyPressEvent(event); return;
    }
    update();
}

void NumberGridWidget::wheelEvent(QWheelEvent* event) {
    resetIdleTimer();
    double delta = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    
    // Zoom towards mouse pos
    QPointF mousePos = event->position();
    QPointF worldPosBefore = mousePos / m_Zoom + m_CameraPos;
    
    m_Zoom *= delta;
    m_Zoom = std::clamp(m_Zoom, 0.3, 3.0);
    
    QPointF worldPosAfter = mousePos / m_Zoom + m_CameraPos;
    m_CameraPos -= (worldPosAfter - worldPosBefore);
    
    update();
}

void NumberGridWidget::mouseMoveEvent(QMouseEvent* event) {
    resetIdleTimer();
    m_MousePos = event->pos();
    QPoint gridPos = mapToGrid(m_MousePos);
    
    m_HoveredGroupId = -1;
    if (gridPos.x() >= 0 && gridPos.x() < GridCols && gridPos.y() >= 0 && gridPos.y() < GridRows) {
        int gid = m_Grid[gridPos.y()][gridPos.x()].groupId;
        if (gid != -1 && m_Groups.count(gid)) {
            m_HoveredGroupId = gid;
        }
    }
}

void NumberGridWidget::mousePressEvent(QMouseEvent* event) {
    resetIdleTimer();
    if (event->button() == Qt::LeftButton && m_HoveredGroupId != -1 && m_HoveredGroupId == m_ActiveGroupId) {
        refineGroup(m_HoveredGroupId);
    }
}

void NumberGridWidget::leaveEvent(QEvent* event) {
    m_MousePos = QPoint(-1, -1);
    m_HoveredGroupId = -1;
    QWidget::leaveEvent(event);
}

void NumberGridWidget::onTimerUpdate() {
    m_Time += 0.05;
    m_IdleCounter++;
    
    if (m_IdleCounter > IdleTimeoutTicks) {
        m_IdleMode = true;
    }
    
    // Every ~5 seconds, maybe activate a new group
    if (!m_IdleMode && m_ActiveGroupId == -1 && (static_cast<int>(m_Time * 20) % 100 == 0)) {
        activateRandomGroup();
    }
    
    // Update bin geometry
    m_BinRect = QRectF(width() / 2.0 - 60, height() - 100, 120, 80);
    
    // Update animating cells
    for (int y = 0; y < GridRows; ++y) {
        for (int x = 0; x < GridCols; ++x) {
            auto& cell = m_Grid[y][x];
            if (cell.isAnimating) {
                // Move towards bin
                QPointF diff = m_BinRect.center() - cell.animPos;
                double dist = std::hypot(diff.x(), diff.y());
                if (dist < 20.0) {
                    cell.isAnimating = false;
                    cell.isRefined = true;
                    cell.isBad = false;
                    cell.groupId = -1;
                    cell.value = std::rand() % 10;
                } else {
                    cell.animPos += diff * 0.15; // Ease in
                }
            }
        }
    }
    
    update();
}

void NumberGridWidget::drawLumonLogo(QPainter& painter) {
    painter.fillRect(rect(), QColor(13, 17, 23, 200)); // Semi-transparent overlay
    painter.setPen(QColor(theme::Colors::Accent));
    painter.setFont(QFont("Arial", 48, QFont::Bold));
    painter.drawText(rect(), Qt::AlignCenter, "L U M O N");
    
    painter.setFont(QFont("Arial", 16));
    painter.drawText(QRect(0, height()/2 + 40, width(), 50), Qt::AlignCenter, "The work is mysterious and important.");
}

void NumberGridWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#010409")); // Very dark background

    if (m_IdleMode) {
        drawLumonLogo(painter);
        return;
    }

    painter.save();
    
    // Draw Refinement Progress Bar
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#161B22"));
    painter.drawRect(20, 20, 300, 20);
    if (m_TotalBadGroups > 0) {
        double progress = static_cast<double>(m_RefinedCount) / std::max(1, m_RefinedCount + static_cast<int>(m_Groups.size()));
        painter.setBrush(QColor(theme::Colors::Success));
        painter.drawRect(20, 20, static_cast<int>(300 * progress), 20);
    }
    painter.setPen(QColor("#8B949E"));
    painter.setFont(QFont("Arial", 10));
    painter.drawText(20, 15, QString("Refined: %1 / %2").arg(m_RefinedCount).arg(m_RefinedCount + m_Groups.size()));

    // Draw Grid
    painter.translate(-m_CameraPos * m_Zoom);
    painter.scale(m_Zoom, m_Zoom);
    
    painter.setFont(m_Font);

    // Compute visible area
    double vx0 = m_CameraPos.x();
    double vy0 = m_CameraPos.y();
    double vx1 = vx0 + width() / m_Zoom;
    double vy1 = vy0 + height() / m_Zoom;
    
    int minX = std::max(0, static_cast<int>(vx0 / CellSize) - 1);
    int minY = std::max(0, static_cast<int>(vy0 / CellSize) - 1);
    int maxX = std::min(GridCols - 1, static_cast<int>(vx1 / CellSize) + 1);
    int maxY = std::min(GridRows - 1, static_cast<int>(vy1 / CellSize) + 1);

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            auto& cell = m_Grid[y][x];
            if (cell.isAnimating) continue; // Drawn later in screen space
            
            double cx = x * CellSize;
            double cy = y * CellSize;
            
            // Jitter calculation
            double n = m_Noise.noise(x * 0.2, y * 0.2 + m_Time);
            double jitterAmt = 2.0;
            QColor textColor = QColor("#64748B"); // Default
            
            if (cell.isRefined) {
                textColor = QColor(theme::Colors::Success);
            } else if (cell.isBad) {
                textColor = QColor("#F85149"); // Scary red
                if (cell.groupId == m_ActiveGroupId) {
                    jitterAmt = 6.0;
                    if (m_HoveredGroupId == m_ActiveGroupId) {
                        jitterAmt = 15.0; // Super active
                        textColor = QColor("#FFD700"); // Yellow warning
                        // Scale based on mouse distance
                        QPointF worldMouse = m_MousePos / m_Zoom + m_CameraPos;
                        double dist = std::hypot(worldMouse.x() - cx, worldMouse.y() - cy);
                        if (dist < 150) {
                            cell.scale = 1.0 + (150 - dist) / 100.0;
                        } else {
                            cell.scale = 1.0;
                        }
                    } else {
                        cell.scale = 1.0;
                    }
                } else {
                    cell.scale = 1.0;
                }
            } else {
                cell.scale = 1.0;
            }
            
            double ox = std::sin(m_Time * 5.0 + x) * n * jitterAmt;
            double oy = std::cos(m_Time * 4.0 + y) * n * jitterAmt;
            
            painter.save();
            painter.translate(cx + CellSize / 2.0 + ox, cy + CellSize / 2.0 + oy);
            painter.scale(cell.scale, cell.scale);
            painter.setPen(textColor);
            painter.drawText(QRectF(-CellSize / 2.0, -CellSize / 2.0, CellSize, CellSize), Qt::AlignCenter, QString::number(cell.value));
            painter.restore();
        }
    }
    painter.restore();
    
    // Draw Bin
    painter.setPen(QColor(theme::Colors::Border));
    painter.setBrush(QColor(theme::Colors::BgLight));
    painter.drawRect(m_BinRect);
    painter.setPen(QColor(theme::Colors::TextMain));
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(m_BinRect, Qt::AlignCenter, "DROP BOX");

    // Draw Animating Cells
    painter.setFont(m_Font);
    for (int y = 0; y < GridRows; ++y) {
        for (int x = 0; x < GridCols; ++x) {
            const auto& cell = m_Grid[y][x];
            if (cell.isAnimating) {
                painter.setPen(QColor(theme::Colors::Success));
                painter.drawText(QRectF(cell.animPos.x(), cell.animPos.y(), CellSize * m_Zoom, CellSize * m_Zoom),
                                 Qt::AlignCenter, QString::number(cell.value));
            }
        }
    }
}

} // namespace severance::gui::widgets
