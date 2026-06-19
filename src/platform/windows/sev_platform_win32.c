/**
 * @file sev_platform_win32.c
 * @brief Win32 implementation of sev_platform.h
 *
 * Pure C — compiled as C17. Uses Win32 APIs directly.
 */

#ifdef _WIN32

#include "severance/platform/sev_platform.h"

/* Lean Windows headers */
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <psapi.h>

/* ─── System Info ──────────────────────────────────────────────────── */

uint32_t sev_platform_get_page_size(void) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (uint32_t)si.dwPageSize;
}

uint64_t sev_platform_get_tick_count(void) {
    return GetTickCount64();
}

uint32_t sev_platform_get_processor_count(void) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (uint32_t)si.dwNumberOfProcessors;
}

/* ─── Error Reporting ──────────────────────────────────────────────── */

uint32_t sev_platform_last_error_string(char* buffer, uint32_t buf_size) {
    if (!buffer || buf_size == 0) {
        return 0;
    }

    DWORD error_code = GetLastError();
    if (error_code == 0) {
        buffer[0] = '\0';
        return 0;
    }

    DWORD chars_written = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buffer,
        (DWORD)buf_size,
        NULL
    );

    /* Strip trailing newline if present */
    while (chars_written > 0 &&
           (buffer[chars_written - 1] == '\n' ||
            buffer[chars_written - 1] == '\r')) {
        buffer[--chars_written] = '\0';
    }

    return (uint32_t)chars_written;
}

/* ─── Process Info ─────────────────────────────────────────────────── */

uint32_t sev_platform_get_current_pid(void) {
    return (uint32_t)GetCurrentProcessId();
}

uint64_t sev_platform_get_current_memory_usage(void) {
    PROCESS_MEMORY_COUNTERS pmc;
    pmc.cb = sizeof(pmc);

    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return (uint64_t)pmc.WorkingSetSize;
    }
    return 0;
}

#endif /* _WIN32 */
