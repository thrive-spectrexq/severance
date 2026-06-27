#include "FimManager.hpp"
#include "logging/Logger.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#include <QCryptographicHash>
#include <QFile>
#include <filesystem>
#include <iostream>

namespace severance::core::security {

FimManager::FimManager() {}

FimManager::~FimManager() {
  StopAll();
}

void FimManager::RegisterCallback(EventCallback cb) {
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Callbacks.push_back(cb);
}

std::string FimManager::ComputeFileHash(const std::string& filePath) {
  QFile file(QString::fromStdString(filePath));
  if (!file.open(QIODevice::ReadOnly)) {
    return "";
  }
  
  QCryptographicHash hash(QCryptographicHash::Sha256);
  if (hash.addData(&file)) {
    return hash.result().toHex().toStdString();
  }
  return "";
}

bool FimManager::StartWatching(const std::string& directoryPath) {
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (m_Watchers.find(directoryPath) != m_Watchers.end()) {
    return true; // already watching
  }

#ifdef _WIN32
  // Create a context
  auto ctx = std::make_unique<WatchContext>();
  ctx->directory = directoryPath;
  ctx->isRunning = true;

  // We start the thread. The thread will handle the Win32 specifics.
  ctx->workerThread = std::thread(&FimManager::WatchThread, this, directoryPath);
  
  m_Watchers[directoryPath] = std::move(ctx);
  SEV_CORE_INFO("FIM started watching: {}", directoryPath);
  return true;
#else
  SEV_CORE_WARN("FIM is only supported on Windows currently");
  return false;
#endif
}

void FimManager::StopWatching(const std::string& directoryPath) {
  std::unique_ptr<WatchContext> ctxToJoin;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Watchers.find(directoryPath);
    if (it != m_Watchers.end()) {
      it->second->isRunning = false;
      
  #ifdef _WIN32
      if (it->second->hDirectory && it->second->hDirectory != INVALID_HANDLE_VALUE) {
          CancelIoEx(it->second->hDirectory, nullptr);
      }
  #endif
      ctxToJoin = std::move(it->second);
      m_Watchers.erase(it);
    }
  }

  if (ctxToJoin && ctxToJoin->workerThread.joinable()) {
    ctxToJoin->workerThread.join();
  }
}

void FimManager::StopAll() {
  std::vector<std::string> dirs;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    for (const auto& kv : m_Watchers) {
      dirs.push_back(kv.first);
    }
  }
  for (const auto& d : dirs) {
    StopWatching(d);
  }
}

void FimManager::WatchThread(std::string directoryPath) {
#ifdef _WIN32
  HANDLE hDir = CreateFileA(
      directoryPath.c_str(),
      FILE_LIST_DIRECTORY,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      nullptr,
      OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
      nullptr
  );

  if (hDir == INVALID_HANDLE_VALUE) {
    SEV_CORE_ERROR("FimManager: Failed to open directory {}", directoryPath);
    return;
  }

  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Watchers.find(directoryPath);
    if (it != m_Watchers.end()) {
      it->second->hDirectory = hDir;
    }
  }

  uint8_t buffer[1024 * 64];
  OVERLAPPED overlapped = {};
  overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

  while (true) {
    {
      std::lock_guard<std::mutex> lock(m_Mutex);
      auto it = m_Watchers.find(directoryPath);
      if (it == m_Watchers.end() || !it->second->isRunning) {
        break;
      }
    }

    DWORD bytesReturned = 0;
    BOOL result = ReadDirectoryChangesW(
        hDir,
        buffer,
        sizeof(buffer),
        TRUE, // recursive
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SECURITY,
        &bytesReturned,
        &overlapped,
        nullptr
    );

    if (!result) {
      SEV_CORE_ERROR("FimManager: ReadDirectoryChangesW failed for {}", directoryPath);
      break;
    }

    // Wait for the result
    DWORD waitStatus = WaitForSingleObject(overlapped.hEvent, 1000); // 1 sec timeout
    
    // Check if we should exit
    bool shouldExit = false;
    {
      std::lock_guard<std::mutex> lock(m_Mutex);
      auto it = m_Watchers.find(directoryPath);
      if (it == m_Watchers.end() || !it->second->isRunning) {
        shouldExit = true;
      }
    }
    
    if (shouldExit) {
      CancelIoEx(hDir, nullptr);
      break;
    }

    if (waitStatus == WAIT_TIMEOUT) {
      continue;
    }

    if (waitStatus == WAIT_OBJECT_0) {
      if (!GetOverlappedResult(hDir, &overlapped, &bytesReturned, FALSE)) {
        break;
      }

      if (bytesReturned == 0) {
        // Buffer overflow - too many changes at once
        SEV_CORE_WARN("FimManager: Buffer overflow, some events may be lost for {}", directoryPath);
        continue;
      }

      FILE_NOTIFY_INFORMATION* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
      std::string oldName;

      while (true) {
        int nameLen = WideCharToMultiByte(CP_UTF8, 0, fni->FileName, fni->FileNameLength / sizeof(WCHAR), nullptr, 0, nullptr, nullptr);
        std::string fileName;
        if (nameLen > 0) {
          fileName.resize(nameLen);
          WideCharToMultiByte(CP_UTF8, 0, fni->FileName, fni->FileNameLength / sizeof(WCHAR), fileName.data(), nameLen, nullptr, nullptr);
        }
        
        std::string fullPath = directoryPath + "\\" + fileName;

        FimEvent ev;
        ev.filePath = fullPath;
        bool triggerEvent = false;

        switch (fni->Action) {
          case FILE_ACTION_ADDED:
            ev.action = FimEvent::Action::Added;
            ev.fileHash = ComputeFileHash(fullPath);
            triggerEvent = true;
            break;
          case FILE_ACTION_REMOVED:
            ev.action = FimEvent::Action::Removed;
            triggerEvent = true;
            break;
          case FILE_ACTION_MODIFIED:
            ev.action = FimEvent::Action::Modified;
            ev.fileHash = ComputeFileHash(fullPath);
            triggerEvent = true;
            break;
          case FILE_ACTION_RENAMED_OLD_NAME:
            oldName = fullPath;
            break;
          case FILE_ACTION_RENAMED_NEW_NAME:
            ev.action = FimEvent::Action::Renamed;
            ev.oldFilePath = oldName;
            triggerEvent = true;
            break;
        }

        if (triggerEvent) {
          std::lock_guard<std::mutex> lock(m_Mutex);
          for (auto& cb : m_Callbacks) {
            cb(ev);
          }
        }

        if (fni->NextEntryOffset == 0) break;
        fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
            reinterpret_cast<uint8_t*>(fni) + fni->NextEntryOffset);
      }
    }
    ResetEvent(overlapped.hEvent);
  }

  CloseHandle(overlapped.hEvent);
  if (hDir && hDir != INVALID_HANDLE_VALUE) {
    CloseHandle(hDir);
  }
#endif
}

} // namespace severance::core::security
