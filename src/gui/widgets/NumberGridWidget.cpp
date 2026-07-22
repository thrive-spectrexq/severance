#include "NumberGridWidget.hpp"
#include "gui/theme/Theme.hpp"
#include "core/store/EventStore.hpp"
#include "core/events/MacrodataRefinedEvent.hpp"
#include <QPainter>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QQueue>
#include <cmath>
#include <random>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>

namespace severance::gui::widgets {

NumberGridWidget::NumberGridWidget(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    m_Font = QFont("Courier New", 14, QFont::Bold);

    generateGrid();

    // Reset bin counts and load past refined count, distributed evenly
    m_BinCounts.fill(0);
    int pastRefinedCount = 0;
    auto recentEvents = core::store::EventStore::GetInstance().GetRecentEvents(10000);
    for (const auto& ev : recentEvents) {
        if (ev.eventType == static_cast<int>(core::events::EventType::MacrodataRefined)) {
            pastRefinedCount++;
        }
    }
    for (int i = 0; i < pastRefinedCount; ++i) {
        m_BinCounts[i % 5]++;
    }

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
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 9);
    
    m_Grid.assign(GridRows, std::vector<NumberCell>(GridCols));
    for (int y = 0; y < GridRows; ++y) {
        for (int x = 0; x < GridCols; ++x) {
            m_Grid[y][x].value = dist(rng);
            double n = m_Noise.noise(x * 0.1, y * 0.1);
            if (n > 0.15) {
                m_Grid[y][x].isBad = true;
            }
        }
    }
    
    identifyGroups();
    
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
    
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, keys.size() - 1);
    m_ActiveGroupId = keys[dist(rng)];
}

void NumberGridWidget::refineGroup(int groupId) {
    if (m_Groups.find(groupId) == m_Groups.end()) return;
    
    int groupSize = m_Groups[groupId].size();
    int targetBin = std::rand() % 5;
    
    for (const auto& p : m_Groups[groupId]) {
        auto& cell = m_Grid[p.y()][p.x()];
        cell.isAnimating = true;
        cell.targetBin = targetBin;
        // Calculate initial screen pos
        cell.animPos = QPointF(p.x() * CellSize - m_CameraPos.x(), p.y() * CellSize - m_CameraPos.y()) * m_Zoom;
        cell.trail.clear();
        cell.trail.push_back(cell.animPos);
    }

    m_Groups.erase(groupId);
    if (m_ActiveGroupId == groupId) {
        m_ActiveGroupId = -1;
        m_HoveredGroupId = -1;
    }

    std::string binName = m_BinFullNames[targetBin].toStdString();
    auto event = std::make_shared<core::events::MacrodataRefinedEvent>(binName, groupSize);
    core::store::EventStore::GetInstance().RecordEvent(event);

    m_BinCounts[targetBin]++;

    int totalRefined = 0;
    for (int count : m_BinCounts) totalRefined += count;
    int targetTotal = std::max(1, m_TotalBadGroups);
    if (totalRefined >= targetTotal) {
        m_QuotaCompleted = true;
    }
}

void NumberGridWidget::switchFile(int index) {
    m_CurrentFileIndex = (index + m_MdrFiles.size()) % m_MdrFiles.size();
    m_BinCounts.fill(0);
    m_QuotaCompleted = false;
    generateGrid();
    update();
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
    
    if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_F) {
        showFileCabinet();
        return;
    }
    
    switch (event->key()) {
        case Qt::Key_Left:  m_CameraPos.rx() -= panSpeed; break;
        case Qt::Key_Right: m_CameraPos.rx() += panSpeed; break;
        case Qt::Key_Up:    m_CameraPos.ry() -= panSpeed; break;
        case Qt::Key_Down:  m_CameraPos.ry() += panSpeed; break;
        case Qt::Key_Tab:   switchFile(m_CurrentFileIndex + 1); break;
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
    QPoint currentPos = event->pos();
    
    if (m_IsDragging) {
        QPointF diff = (currentPos - m_LastMousePos) / m_Zoom;
        m_CameraPos -= diff;
        m_LastMousePos = currentPos;
        update();
        return;
    }
    
    m_MousePos = currentPos;
    QPoint gridPos = mapToGrid(m_MousePos);
    
    m_HoveredGroupId = -1;
    if (gridPos.x() >= 0 && gridPos.x() < GridCols && gridPos.y() >= 0 && gridPos.y() < GridRows) {
        int gid = m_Grid[gridPos.y()][gridPos.x()].groupId;
        if (gid != -1 && m_Groups.count(gid)) {
            m_HoveredGroupId = gid;
            // Also make hovered scary group active if none is currently active
            if (m_ActiveGroupId == -1) {
                m_ActiveGroupId = gid;
            }
        }
    }
    update();
}

void NumberGridWidget::mousePressEvent(QMouseEvent* event) {
    resetIdleTimer();
    
    if (m_MdeMode) {
        if (m_BtnDefiantJazz.contains(event->pos())) m_CurrentSong = 0;
        else if (m_BtnKierHymn.contains(event->pos())) m_CurrentSong = 1;
        else if (m_BtnLumonLounge.contains(event->pos())) m_CurrentSong = 2;
        else if (m_BtnWaffle.contains(event->pos())) m_WaffleAccepted = true;
        update();
        return;
    }
    
    if (event->pos().y() <= 36) {
        showFileCabinet();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (m_HoveredGroupId != -1) {
            refineGroup(m_HoveredGroupId);
        } else {
            m_IsDragging = true;
            m_LastMousePos = event->pos();
        }
    } else if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
        m_IsDragging = true;
        m_LastMousePos = event->pos();
    }
}

void NumberGridWidget::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    m_IsDragging = false;
}

void NumberGridWidget::leaveEvent(QEvent* event) {
    m_MousePos = QPoint(-1, -1);
    m_HoveredGroupId = -1;
    m_IsDragging = false;
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
    
    // Update bin geometries
    double binWidth = 100;
    double binHeight = 60;
    double spacing = 40;
    double totalWidth = 5 * binWidth + 4 * spacing;
    double startX = (width() - totalWidth) / 2.0;
    
    for (int i = 0; i < 5; ++i) {
        m_BinRects[i] = QRectF(startX + i * (binWidth + spacing), height() - 120, binWidth, binHeight);
    }
    
    // Update animating cells
    for (int y = 0; y < GridRows; ++y) {
        for (int x = 0; x < GridCols; ++x) {
            auto& cell = m_Grid[y][x];
            if (cell.isAnimating) {
                cell.trail.push_back(cell.animPos);
                if (cell.trail.size() > 15) {
                    cell.trail.erase(cell.trail.begin());
                }

                // Move towards assigned bin
                QRectF targetRect = m_BinRects[cell.targetBin];
                QPointF diff = targetRect.center() - cell.animPos;
                double dist = std::hypot(diff.x(), diff.y());
                if (dist < 20.0) {
                    cell.isAnimating = false;
                    cell.isRefined = true;
                    cell.isBad = false;
                    cell.groupId = -1;
                    cell.value = std::rand() % 10;
                    cell.trail.clear();
                    m_BinCounts[cell.targetBin]++;
                } else {
                    QPointF dir = diff / dist;
                    QPointF perp(-dir.y(), dir.x());
                    double vortex = std::sin(m_Time * 10.0 + cell.animPos.x() * 0.1) * 5.0;
                    cell.animPos += (diff * 0.15) + (perp * vortex); // Ease in + vortex
                }
            }
        }
    }
    
    update();
}

void NumberGridWidget::drawLumonLogo(QPainter& painter) {
    painter.fillRect(rect(), QColor(1, 4, 9, 230)); // Very dark background overlay
    
    double glow = (std::sin(m_Time * 2.0) + 1.0) / 2.0;
    int alpha = static_cast<int>(50 + glow * 50);
    
    painter.setPen(QColor(0, 229, 255, alpha * 2)); // Cyan glow
    painter.setFont(QFont("Arial", 48, QFont::Bold));
    painter.drawText(rect(), Qt::AlignCenter, "L U M O N");
    
    painter.setFont(QFont("Arial", 16));
    painter.drawText(QRect(0, height()/2 + 40, width(), 50), Qt::AlignCenter, "The work is mysterious and important.");
}

void NumberGridWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#010409")); // Pure black/near-black

    if (m_IdleMode) {
        drawLumonLogo(painter);
        
        // CRT scanlines even in idle mode
        painter.setPen(QColor(0, 0, 0, 100));
        for (int y = 0; y < height(); y += 4) {
            painter.drawLine(0, y, width(), y);
        }
        return;
    }

    painter.save();
    
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
            QColor textColor = QColor("#1A5C4A"); // Dim teal default
            QColor haloColor = Qt::transparent;
            
            if (cell.isRefined) {
                textColor = QColor("#1A5C4A"); // Back to default
            } else if (cell.isBad) {
                // Pulse red
                double redPulse = (std::sin(m_Time * 4.0 + n * 10.0) + 1.0) / 2.0;
                int rVal = static_cast<int>(139 + redPulse * 116);
                textColor = QColor(rVal, 0, 0); // Scary red

                if (cell.groupId == m_ActiveGroupId) {
                    jitterAmt = 6.0;
                    if (m_HoveredGroupId == m_ActiveGroupId) {
                        jitterAmt = 15.0; // Super active
                        textColor = QColor("#00E5FF"); // Bright cyan
                        haloColor = QColor(0, 229, 255, 100);
                        
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
            
            if (haloColor != Qt::transparent) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(haloColor);
                painter.drawEllipse(QRectF(-CellSize / 2.0, -CellSize / 2.0, CellSize, CellSize));
            }
            
            painter.setPen(textColor);
            painter.drawText(QRectF(-CellSize / 2.0, -CellSize / 2.0, CellSize, CellSize), Qt::AlignCenter, QString::number(cell.value));
            painter.restore();
        }
    }
    
    drawAuras(painter, minX, minY, maxX, maxY);
    
    painter.restore();
    
    // Draw 5 Bins and Progress Bars
    int targetPerBin = std::max(1, m_TotalBadGroups / 5);
    QColor binColors[5] = { QColor("#00E5FF"), QColor("#00BFA5"), QColor("#1DE9B6"), QColor("#64FFDA"), QColor("#A7FFEB") };
    
    int totalRefined = 0;
    for (int i = 0; i < 5; ++i) {
        totalRefined += m_BinCounts[i];
        QRectF r = m_BinRects[i];
        
        // Bin background
        painter.setPen(QColor("#1A5C4A"));
        painter.setBrush(QColor(0, 0, 0, 150));
        painter.drawRect(r);
        
        // Bin text
        painter.setPen(QColor("#00E5FF"));
        painter.setFont(QFont("Arial", 16, QFont::Bold));
        painter.drawText(r, Qt::AlignCenter, m_BinNames[i]);
        
        // Progress background
        QRectF pbg(r.x(), r.bottom() + 10, r.width(), 10);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#161B22"));
        painter.drawRect(pbg);
        
        // Progress fill
        double progress = std::min(1.0, static_cast<double>(m_BinCounts[i]) / targetPerBin);
        QRectF pfg(r.x(), pbg.y(), r.width() * progress, 10);
        painter.setBrush(binColors[i]);
        painter.drawRect(pfg);
        
        // Progress text
        painter.setPen(QColor("#1A5C4A"));
        painter.setFont(QFont("Arial", 8));
        painter.drawText(QRectF(r.x(), pbg.bottom() + 5, r.width(), 20), Qt::AlignCenter, QString::number(std::round(progress * 100)) + "%");
    }

    // Draw Animating Cells
    painter.setFont(m_Font);
    for (int y = 0; y < GridRows; ++y) {
        for (int x = 0; x < GridCols; ++x) {
            const auto& cell = m_Grid[y][x];
            if (cell.isAnimating) {
                if (cell.trail.size() > 1) {
                    painter.setPen(QPen(QColor(0, 229, 255, 150), 3.0, Qt::SolidLine, Qt::RoundCap));
                    for (size_t i = 1; i < cell.trail.size(); ++i) {
                        painter.drawLine(cell.trail[i-1] + QPointF(CellSize/2.0*m_Zoom, CellSize/2.0*m_Zoom), 
                                         cell.trail[i] + QPointF(CellSize/2.0*m_Zoom, CellSize/2.0*m_Zoom));
                    }
                }
                painter.setPen(QColor("#00E5FF"));
                painter.drawText(QRectF(cell.animPos.x(), cell.animPos.y(), CellSize * m_Zoom, CellSize * m_Zoom),
                                 Qt::AlignCenter, QString::number(cell.value));
            }
        }
    }

    // Top Lumon Header Bar
    int totalTarget = std::max(1, m_TotalBadGroups);
    int overallProgress = std::min(100, static_cast<int>(std::round(100.0 * totalRefined / totalTarget)));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#061018"));
    painter.drawRect(0, 0, width(), 36);
    painter.setPen(QColor("#1A5C4A"));
    painter.drawLine(0, 36, width(), 36);

    painter.setFont(QFont("Courier New", 12, QFont::Bold));
    painter.setPen(QColor("#00E5FF"));
    QString headerText = QString("FILE: %1 [0%2/06]   |   QUOTA: %3%   |   PRESS [TAB] TO SWITCH FILE")
                         .arg(m_MdrFiles[m_CurrentFileIndex])
                         .arg(m_CurrentFileIndex + 1)
                         .arg(overallProgress);
    painter.drawText(QRect(16, 0, width() - 32, 36), Qt::AlignVCenter | Qt::AlignLeft, headerText);

    // Quota Completion Celebration Banner
    if (m_QuotaCompleted || overallProgress >= 100) {
        m_MdeMode = true;
        drawMDEOverlay(painter);
    }
    
    // CRT scanline overlay
    painter.setPen(QColor(0, 0, 0, 100)); // Thin semi-transparent lines
    for (int y = 0; y < height(); y += 4) {
        painter.drawLine(0, y, width(), y);
    }
}

void NumberGridWidget::showFileCabinet() {
    QDialog dlg(this);
    dlg.setWindowTitle("Lumon File Cabinet");
    dlg.resize(400, 300);
    dlg.setStyleSheet("QDialog { background-color: #010409; color: #00E5FF; font-family: 'Courier New'; }");
    
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    
    QLabel* title = new QLabel("MACRODATA REFINEMENT FILES");
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #00E5FF;");
    layout->addWidget(title);
    
    QListWidget* list = new QListWidget();
    list->setStyleSheet("QListWidget { background-color: #061018; color: #39FF14; border: 1px solid #1A5C4A; } "
                        "QListWidget::item:selected { background-color: #1A5C4A; }");
    layout->addWidget(list);
    
    for (int i = 0; i < (int)m_MdrFiles.size(); ++i) {
        int target = std::max(1, m_TotalBadGroups);
        int refined = 0;
        if (i == m_CurrentFileIndex) {
            for (int c : m_BinCounts) refined += c;
        } else {
            refined = (i * 10) % target; 
        }
        int pct = std::min(100, (int)std::round(100.0 * refined / target));
        QString status = (pct >= 100) ? "100% REFINED" : "IN PROGRESS";
        list->addItem(QString("%1 ... %2% - %3").arg(m_MdrFiles[i]).arg(pct).arg(status));
    }
    
    list->setCurrentRow(m_CurrentFileIndex);
    
    QPushButton* btn = new QPushButton("ACCESS FILE");
    btn->setStyleSheet("QPushButton { background-color: #1A5C4A; color: #00E5FF; padding: 10px; font-weight: bold; }");
    layout->addWidget(btn);
    
    connect(btn, &QPushButton::clicked, [&]() {
        if (list->currentRow() >= 0 && list->currentRow() != m_CurrentFileIndex) {
            switchFile(list->currentRow());
        }
        dlg.accept();
    });
    
    dlg.exec();
}

void NumberGridWidget::drawAuras(QPainter& painter, int minX, int minY, int maxX, int maxY) {
    if (m_HoveredGroupId != -1 && m_HoveredGroupId == m_ActiveGroupId) {
        double minCx = 1e9, minCy = 1e9, maxCx = -1e9, maxCy = -1e9;
        std::vector<QPointF> points;
        
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                if (m_Grid[y][x].groupId == m_ActiveGroupId) {
                    double cx = x * CellSize + CellSize / 2.0;
                    double cy = y * CellSize + CellSize / 2.0;
                    points.push_back(QPointF(cx, cy));
                    minCx = std::min(minCx, cx); minCy = std::min(minCy, cy);
                    maxCx = std::max(maxCx, cx); maxCy = std::max(maxCy, cy);
                    
                    painter.save();
                    painter.translate(cx, cy);
                    QColor auraColors[] = { QColor("#00E5FF"), QColor("#39FF14"), QColor("#FF0055"), QColor("#FF9900") };
                    QColor c = auraColors[(x + y) % 4];
                    c.setAlpha(60);
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(c);
                    painter.drawEllipse(QRectF(-CellSize, -CellSize, CellSize*2, CellSize*2));
                    painter.restore();
                }
            }
        }
        
        if (!points.empty()) {
            double centerCx = (minCx + maxCx) / 2.0;
            QStringList sentiments = {"WOE: 89%", "FROLIC: 4%", "DREAD: 2%", "MALICE: 5%"};
            QColor colors[] = {QColor("#00E5FF"), QColor("#39FF14"), QColor("#FF0055"), QColor("#FF9900")};
            
            painter.save();
            painter.translate(centerCx, minCy - 60);
            painter.setFont(QFont("Courier New", 12, QFont::Bold));
            for (int i = 0; i < 4; ++i) {
                painter.setPen(colors[i]);
                painter.drawText(QRectF(-100, i * 15, 200, 20), Qt::AlignCenter, sentiments[i]);
            }
            painter.restore();
        }
    }
}

void NumberGridWidget::drawMDEOverlay(QPainter& painter) {
    painter.fillRect(rect(), QColor(0, 0, 0, 200));
    
    QColor hues[] = { QColor(0, 255, 0, 50), QColor(0, 255, 255, 50), QColor(255, 0, 255, 50) };
    int hueIdx = static_cast<int>(m_Time * 5.0) % 3;
    painter.fillRect(rect(), hues[hueIdx]);
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 150));
    double danceX = std::sin(m_Time * 3.0) * 50.0;
    painter.drawEllipse(QPointF(width()/2.0 + danceX, height()/2.0 + 50), 40, 80); 
    painter.drawEllipse(QPointF(width()/2.0 + danceX, height()/2.0 - 40), 20, 20); 
    
    painter.setFont(QFont("Courier New", 18, QFont::Bold));
    painter.setPen(QColor("#39FF14"));
    painter.drawText(QRect(0, 50, width(), 40), Qt::AlignCenter, "MUSIC DANCE EXPERIENCE INITIATED");
    
    m_BtnDefiantJazz = QRectF(width()/2 - 250, 150, 150, 40);
    m_BtnKierHymn = QRectF(width()/2 - 75, 150, 150, 40);
    m_BtnLumonLounge = QRectF(width()/2 + 100, 150, 150, 40);
    
    auto drawBtn = [&](QRectF r, QString t, bool active) {
        painter.setBrush(active ? QColor("#39FF14") : QColor(10, 20, 30, 200));
        painter.setPen(active ? QColor("#000000") : QColor("#39FF14"));
        painter.drawRect(r);
        painter.drawText(r, Qt::AlignCenter, t);
    };
    
    drawBtn(m_BtnDefiantJazz, "[ DEFIANT JAZZ ]", m_CurrentSong == 0);
    drawBtn(m_BtnKierHymn, "[ KIER HYMN ]", m_CurrentSong == 1);
    drawBtn(m_BtnLumonLounge, "[ LUMON LOUNGE ]", m_CurrentSong == 2);
    
    m_BtnWaffle = QRectF(width()/2 - 150, height() - 150, 300, 50);
    if (!m_WaffleAccepted) {
        painter.setBrush(QColor("#FF9900"));
        painter.setPen(QColor(0, 0, 0));
        painter.drawRect(m_BtnWaffle);
        painter.drawText(m_BtnWaffle, Qt::AlignCenter, "[ ACCEPT WAFFLE PARTY ]");
    } else {
        painter.setPen(QColor("#FF9900"));
        painter.drawText(QRect(0, height() - 150, width(), 50), Qt::AlignCenter, "PRAISE KIER. ENJOY YOUR WAFFLE PARTY.");
    }
}

} // namespace severance::gui::widgets
