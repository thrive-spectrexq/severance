/**
 * @file SampleCPlugin.c
 * @brief Example Severance plugin written in pure C
 *
 * Demonstrates how to implement all required plugin exports
 * using only the C API defined in severance_plugin_api.h.
 *
 * This plugin serves as a reference implementation and template
 * for developers writing Severance plugins in C.
 */

#include "severance/plugins/severance_plugin_api.h"
#include <stdlib.h>
#include <string.h>

/* ─── Plugin Instance ──────────────────────────────────────────────── */

/**
 * @brief Internal state for this plugin instance.
 *
 * Define SevPluginHandle as the concrete struct — the host
 * only ever sees an opaque pointer to this.
 */
struct SevPluginHandle {
    const SevPluginAPI* host_api;
    int                 initialized;
    int                 event_count;
};

/* ─── Static Metadata ──────────────────────────────────────────────── */

static const char* PLUGIN_NAME        = "Sample C Observer";
static const char* PLUGIN_VERSION     = "1.0.0";
static const char* PLUGIN_DESCRIPTION = "Reference C plugin demonstrating the Severance C Plugin SDK";
static const char* PLUGIN_AUTHOR      = "Severance Contributors";

/* ─── Exported Lifecycle Functions ─────────────────────────────────── */

SEV_C_PLUGIN_EXPORT SevPluginHandle* sev_plugin_create(void) {
    SevPluginHandle* handle = (SevPluginHandle*)calloc(1, sizeof(SevPluginHandle));
    return handle;
}

SEV_C_PLUGIN_EXPORT void sev_plugin_destroy(SevPluginHandle* handle) {
    if (handle) {
        free(handle);
    }
}

SEV_C_PLUGIN_EXPORT SevPluginInfo sev_plugin_get_info(SevPluginHandle* handle) {
    SevPluginInfo info;
    (void)handle; /* Info is static, doesn't depend on instance */

    info.name        = PLUGIN_NAME;
    info.version     = PLUGIN_VERSION;
    info.description = PLUGIN_DESCRIPTION;
    info.author      = PLUGIN_AUTHOR;

    return info;
}

SEV_C_PLUGIN_EXPORT int sev_plugin_initialize(SevPluginHandle* handle,
                                               const SevPluginAPI* api) {
    if (!handle || !api) {
        return 0; /* failure */
    }

    handle->host_api    = api;
    handle->initialized = 1;
    handle->event_count = 0;

    /* Verify API version compatibility */
    if (api->api_version_major != SEV_PLUGIN_API_VERSION_MAJOR) {
        if (api->log_error) {
            api->log_error("Sample C Plugin: API version mismatch!");
        }
        return 0;
    }

    if (api->log_info) {
        api->log_info("Sample C Plugin initialized successfully (C ABI)");
    }

    return 1; /* success */
}

SEV_C_PLUGIN_EXPORT void sev_plugin_shutdown(SevPluginHandle* handle) {
    if (!handle) {
        return;
    }

    if (handle->host_api && handle->host_api->log_info) {
        handle->host_api->log_info("Sample C Plugin shutting down (C ABI)");
    }

    handle->initialized = 0;
    handle->host_api    = NULL;
}
