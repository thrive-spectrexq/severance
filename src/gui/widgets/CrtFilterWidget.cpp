#include "CrtFilterWidget.hpp"
#include <QPaintEvent>
#include <QRandomGenerator>
#include <cmath>

namespace severance::gui::widgets {

CrtFilterWidget::CrtFilterWidget(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    m_TickTimer = new QTimer(this);
    connect(m_TickTimer, &QTimer::timeout, this, &CrtFilterWidget::onFrameTick);
    m_TickTimer->start(16); // ~60 FPS
}

void CrtFilterWidget::setEnabledFilter(bool enabled) {
    m_Enabled = enabled;
    update();
}

void CrtFilterWidget::onFrameTick() {
    if (!m_Enabled) return;

    m_Time += 0.016;
    m_BeamY += 4.0f;
    if (m_BeamY > height()) {
        m_BeamY = 0.0f;
    }

    // Micro flicker alpha between 15 and 28
    m_FlickerAlpha = 18 + (QRandomGenerator::global()->bounded(0, 10));
    update();
}

void CrtFilterWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
}

void CrtFilterWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    if (!m_Enabled) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    int w = width();
    int h = height();

    // 1. Scanlines overlay
    painter.setPen(QColor(0, 0, 0, 40));
    for (int y = 0; y < h; y += 4) {
        painter.drawLine(0, y, w, y);
    }

    // 2. Refresh Beam Sweep (moving phosphor scan line)
    if (m_BeamY > 0 && m_BeamY < h) {
        QLinearGradient beamGrad(0, m_BeamY - 15, 0, m_BeamY + 15);
        beamGrad.setColorAt(0.0, QColor(0, 229, 255, 0));
        beamGrad.setColorAt(0.5, QColor(0, 229, 255, 25));
        beamGrad.setColorAt(1.0, QColor(0, 229, 255, 0));
        painter.fillRect(QRectF(0, m_BeamY - 15, w, 30), beamGrad);
    }

    // 3. Curved CRT Glass Vignette (Dark corner rim)
    QRadialGradient vignette(w / 2.0, h / 2.0, std::hypot(w / 2.0, h / 2.0));
    vignette.setColorAt(0.0, QColor(0, 0, 0, 0));
    vignette.setColorAt(0.7, QColor(0, 0, 0, 30));
    vignette.setColorAt(0.9, QColor(0, 0, 0, 110));
    vignette.setColorAt(1.0, QColor(0, 0, 0, 190));
    painter.fillRect(rect(), vignette);

    // 4. Subtle overall phosphor screen flicker
    painter.fillRect(rect(), QColor(0, 255, 180, m_FlickerAlpha / 4));
}

} // namespace severance::gui::widgets
