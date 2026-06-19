/**
 * @file sev_platform.h
 * @brief Severance Platform Utilities — Pure C API
 *
 * Thin C-linkage wrappers for common platform queries.
 * These are useful from both C and C++ code and establish
 * the pattern for the C platform layer.
 *
 * Implementations are platform-specific:
 *   - Windows: sev_platform_win32.c
 *   - Linux:   sev_platform_linux.c  (future)
 *   - macOS:   sev_platform_darwin.c (future)
 */

#ifndef SEV_PLATFORM_H
#define SEV_PLATFORM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─── System Info ──────────────────────────────────────────────────── */

/**
 * @brief Returns the system's virtual memory page size in bytes.
 */
uint32_t sev_platform_get_page_size(void);

/**
 * @brief Returns a monotonic tick count in milliseconds.
 *
 * On Windows this wraps GetTickCount64().
 * Useful for coarse elapsed-time measurements.
 */
uint64_t sev_platform_get_tick_count(void);

/**
 * @brief Returns the number of logical processors (cores) available.
 */
uint32_t sev_platform_get_processor_count(void);

/* ─── Error Reporting ──────────────────────────────────────────────── */

/**
 * @brief Formats the last OS error into a human-readable string.
 *
 * @param buffer    Output buffer to write the error message into.
 * @param buf_size  Size of the buffer in bytes.
 * @return          Number of characters written (excluding null terminator),
 *                  or 0 if no error or buffer too small.
 */
uint32_t sev_platform_last_error_string(char* buffer, uint32_t buf_size);

/* ─── Process Info ─────────────────────────────────────────────────── */

/**
 * @brief Returns the current process ID.
 */
uint32_t sev_platform_get_current_pid(void);

/**
 * @brief Returns the current process's working set size in bytes.
 */
uint64_t sev_platform_get_current_memory_usage(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SEV_PLATFORM_H */
