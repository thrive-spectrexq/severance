#pragma once

#include <QWidget>
#include <QString>
#include <QModelIndex>
#include <QPoint>
#include <QLineEdit>
#include <QFrame>
#include <vector>

namespace severance::gui::process_view {

struct InnieProfile {
  QString name;
  QString dept;
  QString id;
  int compliance;
  QString status;
  int defianceIndex;
  QString notes;
  QFrame* cardWidget{nullptr};
};

class ProcessView : public QWidget {
  Q_OBJECT

public:
  explicit ProcessView(QWidget *parent = nullptr);
  ~ProcessView() = default;

signals:
  void analyzeProcessRequested(uint32_t pid, const QString& processName, const QString& context);

private slots:
  void onRefreshTimer();
  void onSearchTextChanged(const QString &text);
  void onProcessContextMenu(const QPoint &pos);
  void onProcessDoubleClicked(const QModelIndex &index);

private:
  void setupUI();
  void showInnieDetails(size_t index);

  QLineEdit* m_SearchInput{nullptr};
  std::vector<InnieProfile> m_Innies;
};

} // namespace severance::gui::process_view
