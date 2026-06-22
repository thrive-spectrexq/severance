#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QTimer>
#include <QMutex>
#include <vector>

#include "core/security/FimManager.hpp"
#include "core/security/EventLogManager.hpp"

namespace severance::gui::security_view {

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
  QTimer* m_UiTimer{nullptr};
};

} // namespace severance::gui::security_view
