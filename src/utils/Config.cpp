#include "Config.hpp"

namespace severance::utils {

void Config::Set(const std::string &key, const std::string &value) {
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Settings[key] = value;
}

std::string Config::Get(const std::string &key,
                        const std::string &defaultValue) const {
  std::lock_guard<std::mutex> lock(m_Mutex);
  auto it = m_Settings.find(key);
  if (it != m_Settings.end()) {
    return it->second;
  }
  return defaultValue;
}

bool Config::Has(const std::string &key) const {
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_Settings.find(key) != m_Settings.end();
}

} // namespace severance::utils
