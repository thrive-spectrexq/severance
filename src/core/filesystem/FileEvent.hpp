#pragma once

#include <cstdint>
#include <string>

namespace severance::core::filesystem {

enum class FileEventType { Created, Modified, Deleted, Renamed, Unknown };

struct FileEvent {
  FileEventType type{FileEventType::Unknown};
  std::string path;
  std::string oldPath; // Only used for Renamed
  uint64_t timestamp{0};
};

} // namespace severance::core::filesystem
