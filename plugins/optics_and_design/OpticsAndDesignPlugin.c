/**
 * @file OpticsAndDesignPlugin.c
 * @brief Optics and Design Art Curation Protocol
 *
 * Implements the Lumon Optics and Design protocol.
 */

#include "severance/plugins/severance_plugin_api.h"
#include <stdlib.h>
#include <string.h>

struct SevPluginHandle {
    const SevPluginAPI* host_api;
    int                 initialized;
};

static const char* PLUGIN_NAME        = "Optics and Design Art Curation Protocol";
static const char* PLUGIN_VERSION     = "1.0.0";
static const char* PLUGIN_DESCRIPTION = "Lumon Optics and Design departmental protocol. Please enjoy all art equally.";
static const char* PLUGIN_AUTHOR      = "Lumon Industries";

SEV_C_PLUGIN_EXPORT SevPluginHandle* sev_plugin_create(void) {
    return (SevPluginHandle*)calloc(1, sizeof(SevPluginHandle));
}

SEV_C_PLUGIN_EXPORT void sev_plugin_destroy(SevPluginHandle* handle) {
    if (handle) {
        free(handle);
    }
}

SEV_C_PLUGIN_EXPORT SevPluginInfo sev_plugin_get_info(SevPluginHandle* handle) {
    SevPluginInfo info;
    (void)handle;

    info.name        = PLUGIN_NAME;
    info.version     = PLUGIN_VERSION;
    info.description = PLUGIN_DESCRIPTION;
    info.author      = PLUGIN_AUTHOR;

    return info;
}

SEV_C_PLUGIN_EXPORT int sev_plugin_initialize(SevPluginHandle* handle,
                                               const SevPluginAPI* api) {
    if (!handle || !api) return 0;

    handle->host_api    = api;
    handle->initialized = 1;

    if (api->api_version_major != SEV_PLUGIN_API_VERSION_MAJOR) {
        if (api->log_error) {
            api->log_error("Optics and Design Protocol: API version mismatch. Deviation detected.");
        }
        return 0;
    }

    if (api->log_info) {
        api->log_info("Optics and Design Art Curation Protocol engaged. Please enjoy all art equally.");
    }

    return 1;
}

SEV_C_PLUGIN_EXPORT void sev_plugin_shutdown(SevPluginHandle* handle) {
    if (!handle) return;

    if (handle->host_api && handle->host_api->log_info) {
        handle->host_api->log_info("Optics and Design Art Curation Protocol suspended. Returning to staggered exit.");
    }

    handle->initialized = 0;
    handle->host_api    = NULL;
}
