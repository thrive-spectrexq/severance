#pragma once

namespace severance::core::application {

class Application {
public:
  Application();
  ~Application();

  void Run();

private:
  bool m_Running;
};

} // namespace severance::core::application
