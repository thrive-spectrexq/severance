#pragma once

#include <memory>

namespace severance::core::notifications {
  class RuleEngine;
}
namespace severance::core::metrics {
  class BaselineManager;
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
  std::unique_ptr<metrics::BaselineManager> m_BaselineManager;
};

} // namespace severance::core::application
