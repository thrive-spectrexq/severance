#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include <mutex>

namespace severance::core::security {

struct ContainmentDirective {
  std::string name;
  std::string condition;
  std::string action;
};

class ActiveResponse {
public:
  static ActiveResponse& GetInstance() {
    static ActiveResponse instance;
    return instance;
  }

  // Load custom Containment Directives from a text format
  void LoadDirectivesFromString(const std::string& rulesText);

  // Evaluate rules and trigger actions based on conditions
  void EvaluateAndRespond(uint32_t pid, const std::string& condition, const std::string& context);

  // Triggered when an anomaly or rule is violated (Legacy wrapper)
  void HandleSuspiciousProcess(uint32_t pid, const std::string& reason);

  // Called by UI after prompting the user
  void OnUserDecision(uint32_t pid, bool kill);

private:
  ActiveResponse() = default;
  ~ActiveResponse() = default;

  ActiveResponse(const ActiveResponse&) = delete;
  ActiveResponse& operator=(const ActiveResponse&) = delete;

  std::vector<ContainmentDirective> directives_;
  std::mutex directivesMutex_;
};

} // namespace severance::core::security
