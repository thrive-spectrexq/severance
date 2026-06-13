#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

namespace severance::utils {

class Config {
public:
  static Config &GetInstance() {
    static Config instance;
    return instance;
  }

  void Set(const std::string &key, const std::string &value);
  std::string Get(const std::string &key,
                  const std::string &defaultValue = "") const;
  bool Has(const std::string &key) const;

  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

private:
  Config() = default;
  ~Config() = default;

  std::unordered_map<std::string, std::string> m_Settings;
  mutable std::mutex m_Mutex;
};

} // namespace severance::utils
