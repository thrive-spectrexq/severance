#pragma once

#include <QWidget>
#include <QTimer>
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

private slots:
  void updateData();

private:
  std::vector<double> m_Data;
  size_t m_MaxPoints{100};
  QTimer* m_Timer{nullptr};
};

} // namespace severance::gui::graphs
