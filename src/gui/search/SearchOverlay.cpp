#include "SearchOverlay.hpp"
#include "core/game/GameEngine.hpp"
#include <QVBoxLayout>
#include <QPainter>
#include <QKeyEvent>
#include <algorithm>

namespace severance::gui::search {

SearchOverlay::SearchOverlay(QWidget *parent) : QWidget(parent) {
  setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  hide();

  setupUI();
  
  if (parent) {
    parent->installEventFilter(this);
  }
}

SearchOverlay::~SearchOverlay() = default;

void SearchOverlay::setupUI() {
  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 80, 0, 0);
  mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

  auto container = new QWidget(this);
  container->setFixedWidth(600);
  container->setStyleSheet(R"(
    QWidget {
      background-color: #161B22;
      border: 1px solid #30363D;
      border-radius: 8px;
    }
  )");

  auto containerLayout = new QVBoxLayout(container);
  containerLayout->setContentsMargins(8, 8, 8, 8);
  containerLayout->setSpacing(8);

  m_SearchInput = new QLineEdit(container);
  m_SearchInput->setPlaceholderText("Search procedures, documents, grid connections... (Ctrl+K to focus)");
  m_SearchInput->setStyleSheet(R"(
    QLineEdit {
      background-color: #0D1117;
      border: 1px solid #30363D;
      border-radius: 6px;
      padding: 12px 16px;
      font-size: 16px;
      color: #E6EDF3;
    }
    QLineEdit:focus {
      border-color: #58A6FF;
    }
  )");
  connect(m_SearchInput, &QLineEdit::textChanged, this, &SearchOverlay::onSearchTextChanged);
  containerLayout->addWidget(m_SearchInput);

  m_ResultsList = new QListWidget(container);
  m_ResultsList->setStyleSheet(R"(
    QListWidget {
      background-color: transparent;
      border: none;
      outline: none;
    }
    QListWidget::item {
      padding: 12px;
      border-radius: 6px;
      color: #E6EDF3;
    }
    QListWidget::item:selected {
      background-color: #1F3A5F;
    }
  )");
  m_ResultsList->hide(); // Initially hidden until there are results
  connect(m_ResultsList, &QListWidget::itemActivated, this, &SearchOverlay::onResultActivated);
  containerLayout->addWidget(m_ResultsList);

  mainLayout->addWidget(container);
}

void SearchOverlay::registerProvider(std::shared_ptr<SearchProvider> provider) {
  if (provider) {
    m_Providers.push_back(provider);
  }
}

void SearchOverlay::showOverlay() {
  if (parentWidget()) {
    resize(parentWidget()->size());
  }
  show();
  raise();
  m_SearchInput->setFocus();
  m_SearchInput->selectAll();
}

void SearchOverlay::hideOverlay() {
  hide();
  m_SearchInput->clear();
  m_ResultsList->clear();
  m_ResultsList->hide();
}

void SearchOverlay::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  if (parentWidget()) {
    resize(parentWidget()->size());
  }
}

void SearchOverlay::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  painter.fillRect(rect(), QColor(13, 17, 23, 180)); // Semi-transparent dark background
}

bool SearchOverlay::eventFilter(QObject* obj, QEvent* event) {
  if (event->type() == QEvent::Resize && obj == parentWidget()) {
    resize(parentWidget()->size());
  } else if (event->type() == QEvent::KeyPress && isVisible()) {
    auto* keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      hideOverlay();
      return true;
    } else if (keyEvent->key() == Qt::Key_Down) {
      m_ResultsList->setFocus();
      if (m_ResultsList->currentRow() < 0 && m_ResultsList->count() > 0) {
        m_ResultsList->setCurrentRow(0);
      }
      return true;
    }
  }
  return QWidget::eventFilter(obj, event);
}

void SearchOverlay::onSearchTextChanged(const QString& text) {
  performSearch(text);
}

void SearchOverlay::performSearch(const QString& query) {
  m_ResultsList->clear();
  m_CurrentResults.clear();

  QString lowerQ = query.toLower();
  if (lowerQ.contains("otc") || lowerQ.contains("overtime") || lowerQ.contains("lexington")) {
    core::game::GameEngine::GetInstance().CollectKeycard(1, "Classified Document Archive (" + query + ")");
  }

  if (query.trimmed().isEmpty()) {
    m_ResultsList->hide();
    return;
  }

  for (auto& provider : m_Providers) {
    auto results = provider->search(query);
    m_CurrentResults.insert(m_CurrentResults.end(), results.begin(), results.end());
  }

  // Sort by score descending
  std::sort(m_CurrentResults.begin(), m_CurrentResults.end(),
    [](const SearchResult& a, const SearchResult& b) {
      return a.score > b.score;
    });

  // Limit to top 10
  if (m_CurrentResults.size() > 10) {
    m_CurrentResults.resize(10);
  }

  for (size_t i = 0; i < m_CurrentResults.size(); ++i) {
    const auto& res = m_CurrentResults[i];
    auto* item = new QListWidgetItem(res.title + " - " + res.subtitle, m_ResultsList);
    item->setData(Qt::UserRole, static_cast<unsigned int>(i));
  }

  m_ResultsList->setVisible(!m_CurrentResults.empty());
  if (!m_CurrentResults.empty()) {
    m_ResultsList->setCurrentRow(0);
  }
}

void SearchOverlay::onResultActivated(QListWidgetItem* item) {
  if (!item) return;
  bool ok;
  size_t idx = item->data(Qt::UserRole).toUInt(&ok);
  if (ok && idx < m_CurrentResults.size()) {
    if (m_CurrentResults[idx].onSelected) {
      m_CurrentResults[idx].onSelected();
    }
  }
  hideOverlay();
}

} // namespace severance::gui::search
