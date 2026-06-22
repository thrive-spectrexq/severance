#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace severance::core::security {

class ActiveResponse {
public:
  static ActiveResponse& GetInstance() {
    static ActiveResponse instance;
    return instance;
  }

  // Triggered when an anomaly or rule is violated
  void HandleSuspiciousProcess(uint32_t pid, const std::string& reason);

  // Called by UI after prompting the user
  void OnUserDecision(uint32_t pid, bool kill);

private:
  ActiveResponse() = default;
  ~ActiveResponse() = default;

  ActiveResponse(const ActiveResponse&) = delete;
  ActiveResponse& operator=(const ActiveResponse&) = delete;
};

} // namespace severance::core::security
