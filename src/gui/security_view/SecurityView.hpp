#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QTimer>

namespace severance::gui::security_view {

class KiersProtectionWidget : public QWidget {
  Q_OBJECT
public:
  explicit KiersProtectionWidget(QWidget* parent = nullptr);
protected:
  void paintEvent(QPaintEvent* event) override;
private slots:
  void updateAnim();
private:
  int m_rotation{0};
  QTimer* m_animTimer;
};

class SecurityDashboardWidget : public QWidget {
  Q_OBJECT
public:
  explicit SecurityDashboardWidget(QWidget* parent = nullptr);
};

class SecurityView : public QWidget {
  Q_OBJECT

public:
  explicit SecurityView(QWidget* parent = nullptr);
  ~SecurityView() override;

private slots:
  void updateTable();

private:
  void setupUI();

  QTableWidget* m_ChipTable{nullptr};
  QTimer* m_UiTimer{nullptr};
};

} // namespace severance::gui::security_view
