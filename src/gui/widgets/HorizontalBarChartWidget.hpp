#pragma once

#include <QWidget>
#include <QString>
#include <QColor>
#include <vector>

namespace severance::gui::widgets {

class HorizontalBarChartWidget : public QWidget {
  Q_OBJECT

public:
  struct DataBar {
    QString label;
    float value;
    QString displayValue;
    QColor color;
  };

  explicit HorizontalBarChartWidget(QWidget* parent = nullptr);
  ~HorizontalBarChartWidget() override = default;

  void setBars(const std::vector<DataBar>& bars);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  std::vector<DataBar> m_Bars;
  float m_MaxValue{0.0f};
};

} // namespace severance::gui::widgets
