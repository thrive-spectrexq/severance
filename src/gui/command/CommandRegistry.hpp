#pragma once

#include "gui/search/SearchOverlay.hpp"
#include <QString>
#include <functional>
#include <unordered_map>

namespace severance::gui::command {

struct Command {
  QString id;
  QString title;
  QString description;
  QString shortcut;
  std::function<void()> action;
};

class CommandRegistry {
public:
  static CommandRegistry& GetInstance() {
    static CommandRegistry instance;
    return instance;
  }

  void registerCommand(const Command& cmd) {
    m_Commands[cmd.id] = cmd;
  }

  void executeCommand(const QString& id) {
    auto it = m_Commands.find(id);
    if (it != m_Commands.end() && it->second.action) {
      it->second.action();
    }
  }

  const std::unordered_map<QString, Command>& getCommands() const {
    return m_Commands;
  }

private:
  CommandRegistry() = default;
  ~CommandRegistry() = default;
  std::unordered_map<QString, Command> m_Commands;
};

class CommandSearchProvider : public search::SearchProvider {
public:
  std::vector<search::SearchResult> search(const QString& query) override {
    std::vector<search::SearchResult> results;
    QString lowerQuery = query.toLower();

    for (const auto& [id, cmd] : CommandRegistry::GetInstance().getCommands()) {
      QString lowerTitle = cmd.title.toLower();
      if (lowerTitle.contains(lowerQuery)) {
        search::SearchResult res;
        res.title = cmd.title;
        res.subtitle = cmd.description;
        if (!cmd.shortcut.isEmpty()) {
          res.subtitle += " (" + cmd.shortcut + ")";
        }
        res.category = "Directive";
        
        // Exact prefix match gets higher score
        if (lowerTitle.startsWith(lowerQuery)) res.score = 100.0;
        else res.score = 50.0;
        
        res.onSelected = cmd.action;
        results.push_back(res);
      }
    }
    return results;
  }
};

} // namespace severance::gui::command
