#pragma once

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QRadialGradient>

namespace severance::gui::widgets {

class CrtFilterWidget : public QWidget {
    Q_OBJECT

public:
    explicit CrtFilterWidget(QWidget* parent = nullptr);
    ~CrtFilterWidget() override = default;

    void setEnabledFilter(bool enabled);
    bool isFilterEnabled() const { return m_Enabled; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onFrameTick();

private:
    bool m_Enabled{true};
    QTimer* m_TickTimer{nullptr};
    double m_Time{0.0};
    float m_BeamY{0.0f};
    int m_FlickerAlpha{20};
};

} // namespace severance::gui::widgets
