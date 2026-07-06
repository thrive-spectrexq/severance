#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QTimer>
#include <QMutex>
#include <vector>

#include "core/security/FimManager.hpp"
#include "core/security/EventLogManager.hpp"
#include "core/security/ActiveResponse.hpp"
#include "core/correlation/CorrelationEngine.hpp"

namespace severance::gui::security_view {

class KiersProtectionWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(int rotation READ rotation WRITE setRotation)
public:
  explicit KiersProtectionWidget(QWidget* parent = nullptr);
  int rotation() const { return m_rotation; }
  void setRotation(int r) { m_rotation = r; update(); }

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  int m_rotation{0};
};

class SecurityDashboardWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(int codeDetectorValue READ codeDetectorValue WRITE setCodeDetectorValue)
  Q_PROPERTY(int firewallValue READ firewallValue WRITE setFirewallValue)
public:
  explicit SecurityDashboardWidget(QWidget* parent = nullptr);

  int codeDetectorValue() const { return m_codeDetectorValue; }
  void setCodeDetectorValue(int v) { m_codeDetectorValue = v; update(); }

  int firewallValue() const { return m_firewallValue; }
  void setFirewallValue(int v) { m_firewallValue = v; update(); }

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  int m_codeDetectorValue{0};
  int m_firewallValue{100};
  KiersProtectionWidget* m_shieldWidget;
  QTimer* m_animTimer;
};

class SecurityView : public QWidget {
  Q_OBJECT

public:
  explicit SecurityView(QWidget* parent = nullptr);
  ~SecurityView() override;

private slots:
  void updateTables();

private:
  void setupUI();
  void onFimEvent(const core::security::FimEvent& ev);
  void onLogEvent(const core::security::WindowsEvent& ev);

  // Thread-safe queues for incoming events
  std::vector<core::security::FimEvent> m_PendingFimEvents;
  std::vector<core::security::WindowsEvent> m_PendingLogEvents;
  QMutex m_QueueMutex;

  QTableWidget* m_FimTable{nullptr};
  QTableWidget* m_LogTable{nullptr};
  QTableWidget* m_AnomaliesTable{nullptr};
  QTableWidget* m_DirectivesTable{nullptr};
  QTimer* m_UiTimer{nullptr};
  SecurityDashboardWidget* m_Dashboard{nullptr};
};

} // namespace severance::gui::security_view
