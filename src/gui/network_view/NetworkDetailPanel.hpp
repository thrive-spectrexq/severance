#pragma once

#include <QWidget>
#include <QString>

namespace severance::gui::network_view {

class NetworkDetailPanel : public QWidget {
  Q_OBJECT

public:
  explicit NetworkDetailPanel(QWidget* parent = nullptr);
  ~NetworkDetailPanel() override = default;

  void Clear();
  void LoadConnection(uint32_t pid, const QString& procName, const QString& localIp, uint16_t localPort, const QString& remoteIp, uint16_t remotePort, const QString& state);

private:
  void setupUI();
};

} // namespace severance::gui::network_view
