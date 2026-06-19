/**
 * @file severance_plugin_api.h
 * @brief Severance Plugin SDK — Pure C API
 *
 * This header defines the stable C ABI for Severance plugins.
 * Plugins written in C (or any language with C FFI) implement the
 * functions declared here. The host application loads them at runtime
 * and communicates through the SevPluginAPI function-pointer table.
 *
 * No C++ types cross this boundary. All strings are null-terminated
 * const char*. All structs are POD-safe.
 */

#ifndef SEVERANCE_PLUGIN_API_H
#define SEVERANCE_PLUGIN_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─── Export / Import Macros ────────────────────────────────────────── */

#if defined(_WIN32) || defined(_WIN64)
  #define SEV_C_PLUGIN_EXPORT __declspec(dllexport)
#else
  #define SEV_C_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

/* ─── Version Constants ────────────────────────────────────────────── */

#define SEV_PLUGIN_API_VERSION_MAJOR 1
#define SEV_PLUGIN_API_VERSION_MINOR 0

/* ─── Host API (provided by Severance to plugins) ──────────────────── */

/**
 * @brief Function-pointer table provided by the host to each plugin.
 *
 * Plugins call these to interact with the Severance engine:
 * logging, event subscription (future), and host queries (future).
 */
typedef struct SevPluginAPI {
    uint32_t api_version_major;
    uint32_t api_version_minor;

    /* Logging */
    void (*log_info)(const char* message);
    void (*log_warning)(const char* message);
    void (*log_error)(const char* message);

    /* Reserved for future expansion — do not access */
    void (*_reserved1)(void);
    void (*_reserved2)(void);
    void (*_reserved3)(void);
    void (*_reserved4)(void);
} SevPluginAPI;

/* ─── Plugin Metadata ──────────────────────────────────────────────── */

/**
 * @brief Static metadata returned by the plugin to identify itself.
 *
 * The name and version strings must remain valid for the plugin's
 * entire lifetime (typically static string literals).
 */
typedef struct SevPluginInfo {
    const char* name;
    const char* version;
    const char* description;
    const char* author;
} SevPluginInfo;

/* ─── Plugin Handle ────────────────────────────────────────────────── */

/**
 * @brief Opaque handle representing a plugin instance.
 *
 * The plugin allocates and owns this. The host passes it back
 * on every subsequent call. The plugin defines the internal struct.
 */
typedef struct SevPluginHandle SevPluginHandle;

/* ─── Plugin Lifecycle Exports ─────────────────────────────────────── */

/**
 * Every C plugin DLL must export these four functions:
 *
 *   sev_plugin_create      — Allocate and return a new instance
 *   sev_plugin_destroy     — Free the instance
 *   sev_plugin_get_info    — Return static metadata
 *   sev_plugin_initialize  — Start the plugin (receives host API)
 *   sev_plugin_shutdown    — Stop the plugin and release resources
 */

typedef SevPluginHandle* (*SevPluginCreateFunc)(void);
typedef void             (*SevPluginDestroyFunc)(SevPluginHandle* handle);
typedef SevPluginInfo    (*SevPluginGetInfoFunc)(SevPluginHandle* handle);
typedef int              (*SevPluginInitializeFunc)(SevPluginHandle* handle,
                                                     const SevPluginAPI* api);
typedef void             (*SevPluginShutdownFunc)(SevPluginHandle* handle);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SEVERANCE_PLUGIN_API_H */
