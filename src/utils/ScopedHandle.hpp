#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

namespace severance::utils {

#ifdef _WIN32

/**
 * @brief RAII wrapper for Windows HANDLE to ensure strict leak prevention.
 * Uses `[[nodiscard]]` and prevents copy-semantics to avoid double-closure.
 */
class ScopedHandle {
public:
    ScopedHandle(HANDLE handle = nullptr) noexcept : m_handle(handle) {}

    // Disable copy
    ScopedHandle(const ScopedHandle&) = delete;
    ScopedHandle& operator=(const ScopedHandle&) = delete;

    // Enable move
    ScopedHandle(ScopedHandle&& other) noexcept : m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }
    
    ScopedHandle& operator=(ScopedHandle&& other) noexcept {
        if (this != &other) {
            Close();
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    ~ScopedHandle() {
        Close();
    }

    [[nodiscard]] HANDLE Get() const noexcept { return m_handle; }
    [[nodiscard]] bool IsValid() const noexcept { return m_handle != nullptr && m_handle != INVALID_HANDLE_VALUE; }
    
    // Auto-cast to HANDLE
    operator HANDLE() const noexcept { return m_handle; }

    void Reset(HANDLE handle = nullptr) noexcept {
        Close();
        m_handle = handle;
    }

    // Used for out-parameters in Win32 APIs: &handle.GetAddressOf()
    HANDLE* GetAddressOf() noexcept { return &m_handle; }

private:
    void Close() noexcept {
        if (IsValid()) {
            CloseHandle(m_handle);
            m_handle = nullptr;
        }
    }

    HANDLE m_handle;
};

#endif

} // namespace severance::utils
