#pragma once

#include <QWidget>
#include <QColor>

namespace severance::gui::widgets {

class StatusIndicator : public QWidget {
  Q_OBJECT

public:
  enum class Status {
    Offline,
    Online,
    Warning,
    Error
  };

  explicit StatusIndicator(QWidget *parent = nullptr);
  ~StatusIndicator() override = default;

  void setStatus(Status status);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  Status m_Status{Status::Offline};
  QColor getColorForStatus(Status status) const;
};

} // namespace severance::gui::widgets
