#pragma once

#include <QWidget>
#include <vector>

namespace severance::gui::graphs {

class ResourceGraph : public QWidget {
  Q_OBJECT

public:
  explicit ResourceGraph(QWidget *parent = nullptr);
  ~ResourceGraph() override = default;

  void addDataPoint(double value);
  void clear();

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  std::vector<double> m_Data;
  size_t m_MaxPoints{100};
};

} // namespace severance::gui::graphs
