#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QTimer>

namespace severance::gui::security_view {

class KiersProtectionWidget : public QWidget {
  Q_OBJECT
public:
  explicit KiersProtectionWidget(QWidget* parent = nullptr);
  void triggerBoost();
protected:
  void paintEvent(QPaintEvent* event) override;
private slots:
  void updateAnim();
private:
  int m_rotation{0};
  int m_speed{2};
  QTimer* m_animTimer;
};

class SecurityDashboardWidget : public QWidget {
  Q_OBJECT
public:
  explicit SecurityDashboardWidget(QWidget* parent = nullptr);
  KiersProtectionWidget* protectionWidget() const { return m_ProtectionWidget; }
private:
  KiersProtectionWidget* m_ProtectionWidget{nullptr};
};

class SecurityView : public QWidget {
  Q_OBJECT

public:
  explicit SecurityView(QWidget* parent = nullptr);
  ~SecurityView() override;

private slots:
  void updateTable();
  void onRecalibrateClicked();
  void onCellDoubleClicked(int row, int column);

private:
  void setupUI();

  SecurityDashboardWidget* m_DashboardWidget{nullptr};
  QTableWidget* m_ChipTable{nullptr};
  QPushButton* m_RecalibrateBtn{nullptr};
  QTimer* m_UiTimer{nullptr};
};

} // namespace severance::gui::security_view
