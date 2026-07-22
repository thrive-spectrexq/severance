#include "TimelineHistogram.hpp"
#include <QPainter>
#include <cstdlib>

namespace severance::gui::timeline {

TimelineHistogram::TimelineHistogram(QWidget* parent) : QWidget(parent) {
    setFixedHeight(80);
}

void TimelineHistogram::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#0A0F14"));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#00E5FF"));
    for(int i = 0; i < width(); i += 10) {
        int h = std::rand() % (height() - 10) + 5;
        painter.drawRect(i, height() - h, 8, h);
    }
}

} // namespace severance::gui::timeline
