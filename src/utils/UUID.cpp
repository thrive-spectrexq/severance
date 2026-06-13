#include "UUID.hpp"
#include <iomanip>
#include <random>
#include <sstream>

namespace severance::utils {

std::string UUID::Generate() {
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    thread_local std::uniform_int_distribution<> dis(0, 15);
    thread_local std::uniform_int_distribution<> dis2(8, 11);

  std::stringstream ss;
  ss << std::hex;
  for (int i = 0; i < 8; ++i)
    ss << dis(gen);
  ss << "-";
  for (int i = 0; i < 4; ++i)
    ss << dis(gen);
  ss << "-4";
  for (int i = 0; i < 3; ++i)
    ss << dis(gen);
  ss << "-";
  ss << dis2(gen);
  for (int i = 0; i < 3; ++i)
    ss << dis(gen);
  ss << "-";
  for (int i = 0; i < 12; ++i)
    ss << dis(gen);

  return ss.str();
}

} // namespace severance::utils
