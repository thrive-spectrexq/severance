#include <string>

namespace severance::core::plugins {

class PluginLoader {
public:
  static void *LoadLibrary(const std::string &path) {
    // Stub for dlopen / LoadLibrary
    (void)path;
    return nullptr;
  }

  static void UnloadLibrary(void *handle) {
    // Stub for dlclose / FreeLibrary
    (void)handle;
  }
};

} // namespace severance::core::plugins
