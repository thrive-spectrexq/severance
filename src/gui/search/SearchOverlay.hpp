#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <vector>
#include <string>

namespace severance::gui::search {

struct SearchResult {
  QString title;
  QString subtitle;
  QString icon;
  QString category;
  double score{0.0};
  std::function<void()> onSelected;
};

class SearchProvider {
public:
  virtual ~SearchProvider() = default;
  virtual std::vector<SearchResult> search(const QString& query) = 0;
};

class SearchOverlay : public QWidget {
  Q_OBJECT

public:
  explicit SearchOverlay(QWidget *parent = nullptr);
  ~SearchOverlay() override;

  void showOverlay();
  void hideOverlay();
  void registerProvider(std::shared_ptr<SearchProvider> provider);

protected:
  void showEvent(QShowEvent* event) override;
  void paintEvent(QPaintEvent* event) override;
  bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
  void onSearchTextChanged(const QString& text);
  void onResultActivated(QListWidgetItem* item);

private:
  void setupUI();
  void performSearch(const QString& query);

  QLineEdit* m_SearchInput{nullptr};
  QListWidget* m_ResultsList{nullptr};
  
  std::vector<std::shared_ptr<SearchProvider>> m_Providers;
  std::vector<SearchResult> m_CurrentResults;
};

} // namespace severance::gui::search
