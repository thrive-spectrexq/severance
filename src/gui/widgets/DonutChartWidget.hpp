#pragma once

#include <QWidget>
#include <QString>
#include <QColor>
#include <vector>

namespace severance::gui::widgets {

class DonutChartWidget : public QWidget {
  Q_OBJECT

public:
  struct DataSegment {
    QString label;
    float value;
    QColor color;
  };

  explicit DonutChartWidget(QWidget* parent = nullptr);
  ~DonutChartWidget() override = default;

  void setSegments(const std::vector<DataSegment>& segments);
  void setCenterText(const QString& text);
  void setDonutThickness(int thickness);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  std::vector<DataSegment> m_Segments;
  QString m_CenterText;
  int m_Thickness{24};
  float m_TotalValue{0.0f};
};

} // namespace severance::gui::widgets
