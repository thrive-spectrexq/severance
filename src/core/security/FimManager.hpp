#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <memory>

namespace severance::core::security {

struct FimEvent {
  enum class Action {
    Added,
    Modified,
    Removed,
    Renamed
  };
  
  Action action;
  std::string filePath;
  std::string oldFilePath; // For Renamed
  std::string fileHash;    // SHA-256 for Added/Modified
};

class FimManager {
public:
  using EventCallback = std::function<void(const FimEvent&)>;

  static FimManager& GetInstance() {
    static FimManager instance;
    return instance;
  }

  bool StartWatching(const std::string& directoryPath);
  void StopWatching(const std::string& directoryPath);
  void StopAll();

  void RegisterCallback(EventCallback cb);

private:
  FimManager();
  ~FimManager();

  FimManager(const FimManager&) = delete;
  FimManager& operator=(const FimManager&) = delete;

  std::string ComputeFileHash(const std::string& filePath);
  void WatchThread(std::string directoryPath);

  struct WatchContext {
    std::string directory;
    std::thread workerThread;
    std::atomic<bool> isRunning{false};
    void* hDirectory{nullptr}; // HANDLE equivalent
  };

  std::vector<EventCallback> m_Callbacks;
  std::unordered_map<std::string, std::unique_ptr<WatchContext>> m_Watchers;
  std::mutex m_Mutex;
};

} // namespace severance::core::security
