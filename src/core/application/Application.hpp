#pragma once

#include <memory>

namespace severance::core::notifications {
  class RuleEngine;
}

namespace severance::core::application {

class Application {
public:
  Application();
  ~Application();

  void Run();

private:
  bool m_Running;
  std::unique_ptr<notifications::RuleEngine> m_RuleEngine;
};

} // namespace severance::core::application
