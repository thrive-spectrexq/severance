#include "severance/plugins/severance_plugin_api.h"
#include <thread>
#include <chrono>
#include <atomic>

// C-ABI Plugin implementation of the behavioral heuristics
// This plugin demonstrates triggering heuristics dynamically

static const SevPluginAPI* g_Api = nullptr;
static std::atomic<bool> g_Running{false};
static std::thread g_Worker;

void SecurityWorkerThread() {
    // Wait a few seconds to let the application start
    std::this_thread::sleep_for(std::chrono::seconds(5));

    if (g_Running && g_Api && g_Api->emit_heuristic_alert) {
        g_Api->emit_heuristic_alert(
            "Behavioral Heuristic Triggered", 
            "The security_plugin detected an anomalous sequence of API calls typically associated with Process Hollowing.",
            2 // Warning
        );
        
        g_Api->log_info("Security heuristic alert emitted.");
    }
}

extern "C" {

SEV_C_PLUGIN_EXPORT SevPluginHandle* sev_plugin_create() {
    return reinterpret_cast<SevPluginHandle*>(1); // Dummy handle
}

SEV_C_PLUGIN_EXPORT void sev_plugin_destroy(SevPluginHandle* handle) {
    (void)handle;
}

SEV_C_PLUGIN_EXPORT SevPluginInfo sev_plugin_get_info(SevPluginHandle* handle) {
    (void)handle;
    SevPluginInfo info;
    info.name = "Security Heuristics Plugin";
    info.version = "1.0.0";
    info.description = "Provides advanced behavioral heuristics.";
    info.author = "Severance AI";
    return info;
}

SEV_C_PLUGIN_EXPORT int sev_plugin_initialize(SevPluginHandle* handle, const SevPluginAPI* api) {
    (void)handle;
    g_Api = api;
    
    if (g_Api && g_Api->log_info) {
        g_Api->log_info("Security Heuristics Plugin initializing...");
    }

    g_Running = true;
    g_Worker = std::thread(SecurityWorkerThread);

    return 1;
}

SEV_C_PLUGIN_EXPORT void sev_plugin_shutdown(SevPluginHandle* handle) {
    (void)handle;
    g_Running = false;
    
    if (g_Worker.joinable()) {
        g_Worker.join();
    }
    
    if (g_Api && g_Api->log_info) {
        g_Api->log_info("Security Heuristics Plugin shut down.");
    }
}

} // extern "C"
