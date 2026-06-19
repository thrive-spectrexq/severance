# Severance

> Deep, interconnected visibility into your running machine — processes, network, filesystem, and their relationships — all in one place.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![C](https://img.shields.io/badge/C-17-blue.svg)
![Qt](https://img.shields.io/badge/Qt-6-green.svg)
![Build](https://img.shields.io/github/actions/workflow/status/thrive-spectrexq/severance/ci.yml)
![Platform](https://img.shields.io/badge/platform-Windows-0078D4.svg)

---

## Overview

Severance is a native desktop application for understanding what your machine is doing — in real time, with clarity and depth.

Most system tools scatter the picture across a dozen utilities. Task Manager shows processes but not what they're touching. ProcMon shows file activity but not the network. Wireshark shows packets but not which process sent them. You end up copy-pasting PIDs between six different windows to answer one question.

Severance brings it all together. Processes, network connections, filesystem events, resource consumption, and sandboxed execution environments — all in one application, all linked through a cross-domain correlation engine. Click a suspicious network connection and immediately see the process that opened it, the files it read before doing so, and the parent that spawned it. No context switching. No lost threads.

Built on C++23 and C17 with Qt 6. The core application uses modern C++23. The plugin SDK and platform abstraction layer expose a stable C17 ABI, making plugins writable in C or any language with C FFI. Designed for developers, security researchers, and power users who want a single tool they can open every morning and leave running all day.

> **Current focus: Windows.** Linux and macOS support are planned for future releases.

---

## Why Severance?

Most monitoring tools are either too narrow (one metric, one view) or too noisy (dashboards that bury signal in decoration). Severance is built on a different premise: **complete separation of concerns, with deep cross-context connection.**

Each module observes one domain cleanly. The correlation engine links events across all of them. The AI insights layer explains what it all means in plain language. You don't need to be a kernel engineer to understand what your machine is doing — you just need Severance open.

### What makes Severance different

| Others | Severance |
|---|---|
| Scattered across 6+ tools | Everything in one window |
| Raw data dumps | AI-powered plain-language explanations |
| Single-domain views | Cross-domain correlation — one click from any event to its full chain |
| Win32-era interfaces | Modern, premium UI designed for daily use |
| Threshold-based alerts | Pattern-based smart notifications |
| Static dashboards | Searchable, filterable, keyboard-driven |

---

## Core Principles

**Modern C++ with a Stable C ABI**
C++23 throughout the core. RAII everywhere. Smart pointers only — no raw ownership. Strong type safety. STL-first design. The plugin SDK and platform layer expose a C17 ABI boundary — no C++ types cross the plugin interface. This means plugins can be written in C, Rust, Zig, or any language with C FFI support.

**Performance**
Low memory footprint. Native execution with no runtime overhead. Multi-threaded event processing. Efficient data structures built for high-frequency system events. Sub-100ms response to every interaction.

**Keyboard-First**
Global search, command palette, and Vim-style optional bindings. Every action reachable without a mouse.

**Extensibility**
Dual-language plugin architecture with both C++ and C SDKs. Event-driven communication via a central event bus. Dynamic module loading at runtime. Clean separation between core and extensions. C plugins use a pure C ABI — no C++ compiler required to build one.

**Intelligence**
Optional local AI integration for anomaly explanation, behavioral summarization, and natural-language system queries — 100% private, nothing leaves your machine.

---

## Features

### System Dashboard

Your machine's health at a glance, before you drill into anything.

- CPU usage per core with trend history
- Memory, swap, and virtual memory breakdown
- GPU utilization, VRAM, and temperature monitoring
- Disk I/O summary per device
- Network activity summary across all interfaces
- Thermal and power draw data via Windows APIs
- Live event ticker for high-priority system activity
- Anomaly indicators — spikes, stalls, and unexpected pattern changes
- Customizable widget layout; arrange the view to suit your workflow

---

### Process Explorer

Deep, real-time inspection of every running process.

- Full process tree with parent-child relationships
- Per-process CPU, memory, disk I/O, GPU, and thread count
- Handle and file descriptor enumeration
- Search and filter by name, PID, user, or binary path
- Sortable columns and expandable detail panels
- Process tagging and notes — annotate processes ("dev server", "known safe", "investigate") persisted across sessions
- Abnormal termination detection and flagging
- Real-time resource consumption heat-mapping

---

### Activity Timeline

A single scrollable view of everything that has happened on your system, across all event types.

- Process creation, suspension, and termination
- File access and mutation events
- Network connection lifecycle — open, active, closed
- Plugin-emitted custom events
- Scrollable and zoomable, with per-event-type filtering
- Click any event to see its full cross-context chain: process → files touched → connections opened
- Session recording and playback for post-mortem analysis
- Exportable event logs in JSON, CSV, or Markdown

---

### Network Monitor

Complete real-time visibility into every connection your machine makes or accepts.

- Live connection list with protocol, state, and duration
- Per-process traffic statistics with historical graphs
- Port usage breakdown and anomaly detection
- DNS resolution log
- Unusual outbound connection alerting
- Connection grouping by process, domain, or port family
- Deep packet inspection hooks via the plugin system

---

### File Activity Monitor

Watch the filesystem as events happen — every write, deletion, rename, and access.

- File creation, modification, deletion, and rename events
- Watched directory configuration with recursive depth control
- Filter by path, extension, process, or event type
- Full event history with timestamps and process attribution
- Alerts on modification to sensitive or watched paths
- Diff view for text file changes
- Bulk write detection for identifying high-frequency write patterns

---

### Global Search & Command Palette

Find anything. Do anything. Without reaching for the mouse.

**Global Search** (`Ctrl+K`):
- Search across processes, network connections, file events, timeline entries — everything
- Fuzzy matching with instant results
- Results grouped by domain with one-click navigation

**Command Palette** (`Ctrl+Shift+P`):
- Execute actions by name: "Kill all Chrome processes", "Start recording", "Export last hour"
- Context-aware suggestions based on current view
- Plugin-contributed commands

---

### AI Insights Module

Local AI that turns raw system data into understanding. Optional. 100% private.

- **Anomaly Explanation**: Click a CPU spike → get a plain-language explanation of what caused it and why it's unusual
- **Behavioral Summary**: "In the last hour, this process wrote 847 files to /tmp, opened 12 outbound connections to 3 unique IPs, and consumed 2.1GB of memory"
- **Natural Language Queries**: Ask "What's using the most bandwidth right now?" or "Show me processes that started in the last 5 minutes and are making network calls"
- **Baseline Comparison**: Record a "healthy" system state → compare current behavior → highlight deviations

**Implementation:**
- Powered by a local GGUF model via llama.cpp — no cloud, no API keys, no data leaves your machine
- Works without AI enabled — the module is entirely optional
- Users can connect their own Ollama instance as an alternative backend

---

### Smart Notifications

Alerts that understand context, not just thresholds.

- Pattern-based detection: not just "CPU > 90%" but "CPU 3x higher than your average for this time of day"
- Escalation levels: subtle indicator → toast → sound → system notification
- Configurable per-process and per-event-type rules
- Notification history with links back to the triggering event chain

---

### Workspace Profiles

Save and restore different monitoring configurations.

- **Development**: Focus on process tree, file watcher on project directory, build tool metrics
- **Security Audit**: Full timeline, network monitor with alerting, sandbox ready
- **Performance Test**: Dashboard with max metrics, recording enabled
- Custom profiles with exported/imported configuration files

---

### Isolation Profiles

Define controlled execution environments and enforce them at the OS level.

- Sandbox profiles with granular permission scopes
- Resource limits per profile: CPU, memory, disk I/O, network bandwidth
- Process group assignment and enforcement
- Execution policies: allow list, deny list, or audit-only mode
- Profile export and import for reproducible environments
- Violation logging with full event trace
- Windows enforcement via Job Objects and Restricted Tokens

---

### Cross-Event Correlation Engine

The feature that turns isolated observations into a complete picture.

Events from the process explorer, network monitor, file activity monitor, and timeline are linked automatically. Follow any event in any direction:

- From a network connection → to the process that opened it → to the files it read first
- From a file write → to the process responsible → to its full resource history
- From a process → to every network and filesystem action it has ever taken in the session

**Visual correlation graph**: a force-directed view showing live process → resource relationships.

No context switching. No copy-pasting PIDs between tools. One click.

---

### Session Recording and Playback

Capture a complete system activity session and replay it at any time.

```
[REC] ● 00:04:32  —  Recording: "incident_2024_11_04"
      Events captured:  84,201
      Processes tracked: 312
      File events:       9,047
      Network events:    2,103
```

Useful for incident post-mortems, demonstrations, security research, and sharing observations with colleagues without requiring them to reproduce conditions.

---

### Plugin System

Extend Severance without touching core code. Two SDKs — pick the one that fits your language:

**C++ Plugin SDK** (`IPlugin` interface):
- Full C++ class-based interface
- Implement `IPlugin`, export `CreatePlugin()`
- Access to the `IPluginAPI` proxy for logging and events

**C Plugin SDK** (`severance_plugin_api.h`):
- Pure C17 header — no C++ compiler needed
- Implement 5 exported functions: `sev_plugin_create`, `sev_plugin_destroy`, `sev_plugin_get_info`, `sev_plugin_initialize`, `sev_plugin_shutdown`
- Receive a `SevPluginAPI` function-pointer table from the host
- Compatible with any language that can produce a DLL with C exports (Rust, Zig, Go, etc.)

**SDK surface area:**
- Event subscription and emission via the Event Bus
- UI panel and widget injection
- Custom timeline event types
- Dashboard widget registration
- Data export format hooks
- Settings schema contribution
- Command palette command registration

**Bundled plugins:**

| Plugin | Language | Purpose |
|---|---|---|
| `sample_plugin` | C++ | Reference C++ plugin demonstrating the IPlugin interface |
| `sample_c_plugin` | C | Reference C plugin demonstrating the C Plugin SDK |
| `network_plugin` | C++ | Deep traffic analysis, anomaly alerting, per-connection bandwidth tracking |
| `filesystem_plugin` | C++ | Extended file event attribution and bulk-write pattern detection |
| `security_plugin` | C++ | Behavioral heuristics: process hollowing indicators, unusual parentage, suspicious write patterns |
| `graph_plugin` | C++ | Custom visualization widgets for the dashboard and timeline |
| `export_plugin` | C++ | Structured telemetry report generation (JSON, CSV, Markdown) |
| `process_scanner` | C++ | Heuristic process classification and risk scoring |

Community plugins install from the in-app Plugin Marketplace without restarting.

---

## Architecture

```
+-------------------------------------------------------------------------+
|                             GUI Layer (C++)                             |
|-------------------------------------------------------------------------|
| Dashboard | Process Explorer | Timeline | Network | Files | Isolation   |
| Global Search | Command Palette | AI Insights Panel                    |
+-------------------------------------------------------------------------+
|                            Event Bus (C++)                              |
|              All modules communicate here — and only here               |
+-------------------------------------------------------------------------+
|                          Core Services (C++)                            |
|-------------------------------------------------------------------------|
| Process Manager    | Network Manager  | File Monitor  | Sandbox Manager |
| Plugin Manager     | Session Recorder | Correlation Engine              |
| AI Engine          | Notification Mgr | Thread Pool                     |
| Event Store (SQLite)                  | Workspace Manager               |
+-------------------------------------------------------------------------+
|                        C ABI Plugin Bridge                              |
|-------------------------------------------------------------------------|
| CPluginBridge: wraps C plugins → IPlugin C++ interface                  |
| severance_plugin_api.h: stable C17 ABI for plugin development           |
+-------------------------------------------------------------------------+
|                    Windows Platform Layer (C/C++)                        |
|-------------------------------------------------------------------------|
| sev_platform.h (C17)               | ETW (Event Tracing for Windows)   |
| WFP (Windows Filtering Platform)   | PDH (Performance Data Helper)     |
| Job Objects / Restricted Tokens    | WMI / COM                         |
| NVML / DXGI (GPU)                                                       |
+-------------------------------------------------------------------------+
```

The GUI layer communicates exclusively through the Event Bus. Core services never reach into the interface directly. The platform layer exposes Windows-native APIs through stable internal interfaces — when Linux and macOS support is added, the same core and UI code will work unchanged.

The C ABI Plugin Bridge sits between the core and any C-based plugin. It translates the host's C++ `IPluginAPI` into a C function-pointer table (`SevPluginAPI`) and wraps C plugin instances in a `CPluginBridge` adapter that implements the `IPlugin` C++ interface. From the PluginManager's perspective, C and C++ plugins are indistinguishable.

---

## Technology Stack

| Component | Technology |
|---|---|
| Languages | C++23 (core, GUI), C17 (plugin SDK, platform layer) |
| GUI Framework | Qt 6.5+ |
| Build System | CMake 3.28+ (multi-language: C + CXX) |
| Package Manager | vcpkg |
| Plugin SDK | C++ (`IPlugin` interface) + C17 (`severance_plugin_api.h`) |
| Event Store | SQLite (via SQLiteCpp) |
| AI Engine | llama.cpp (GGUF models) |
| Logging | spdlog |
| Testing | Catch2 |
| GPU Monitoring | NVML (NVIDIA) + DXGI (general) |
| System Tracing | ETW (Event Tracing for Windows) |
| Network Filtering | Windows Filtering Platform |
| Sandboxing | Job Objects + Restricted Tokens |
| Graph Rendering | Custom (Qt RHI — GPU-accelerated) |
| Code Formatting | clang-format |
| Static Analysis | clang-tidy |
| CI/CD | GitHub Actions |
| Installer | WiX Toolset |

---

## Repository Structure

```
severance/
│
├── .github/
│   ├── workflows/
│   │   ├── ci.yml
│   │   ├── release.yml
│   │   └── static-analysis.yml
│   ├── ISSUE_TEMPLATE/
│   └── PULL_REQUEST_TEMPLATE.md
│
├── cmake/
│   ├── CompilerWarnings.cmake
│   ├── Sanitizers.cmake
│   ├── Dependencies.cmake
│   └── Version.cmake
│
├── docs/
│   ├── architecture.md
│   ├── plugin-sdk.md
│   ├── event-system.md
│   ├── correlation-engine.md
│   ├── session-recording.md
│   ├── ai-engine.md
│   ├── isolation-profiles.md
│   ├── contributing.md
│   └── roadmap.md
│
├── assets/
│   ├── icons/
│   ├── themes/
│   │   ├── dark.json
│   │   ├── light.json
│   │   └── high-contrast.json
│   ├── fonts/
│   └── screenshots/
│
├── include/
│   └── severance/
│       ├── core/
│       ├── gui/
│       ├── plugins/
│       │   ├── IPluginAPI.hpp           # C++ plugin API interface
│       │   └── severance_plugin_api.h   # C17 plugin SDK (stable ABI)
│       ├── platform/
│       │   └── sev_platform.h           # C17 platform abstraction
│       └── utils/
│
├── src/
│   ├── main.cpp
│   │
│   ├── core/
│   │   ├── application/
│   │   ├── events/
│   │   ├── process/
│   │   ├── network/
│   │   ├── filesystem/
│   │   ├── sandbox/
│   │   ├── plugins/
│   │   ├── correlation/
│   │   ├── session/
│   │   ├── ai/
│   │   ├── notifications/
│   │   ├── workspace/
│   │   └── logging/
│   │
│   ├── gui/
│   │   ├── windows/
│   │   ├── dashboard/
│   │   ├── process_view/
│   │   ├── timeline/
│   │   ├── network_view/
│   │   ├── file_view/
│   │   ├── isolation_view/
│   │   ├── search/
│   │   ├── command_palette/
│   │   ├── ai_panel/
│   │   ├── graphs/
│   │   └── widgets/
│   │
│   ├── platform/
│   │   └── windows/
│   │       ├── sev_platform_win32.c     # C17 platform implementation
│   │       ├── etw/
│   │       ├── wfp/
│   │       ├── gpu/
│   │       └── sandbox/
│   │
│   └── utils/
│
├── plugins/
│   ├── sample_plugin/                   # Reference C++ plugin
│   ├── sample_c_plugin/                 # Reference C plugin
│   ├── network_plugin/
│   ├── filesystem_plugin/
│   ├── security_plugin/
│   ├── graph_plugin/
│   ├── export_plugin/
│   └── process_scanner/
│
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── gui/
│   └── benchmarks/
│
├── scripts/
│   ├── bootstrap.ps1
│   └── package.ps1
│
├── CMakeLists.txt
├── vcpkg.json
├── .clang-format
├── .clang-tidy
├── LICENSE
└── README.md
```

---

## Build

### Requirements

- **Windows 10 21H2 or later** (Windows 11 recommended)
- C++23 and C17 capable compiler: MSVC 2022 (17.8+)
- Qt 6.5 or later
- CMake 3.28 or later
- Git
- vcpkg

> **Note:** All builds run through GitHub Actions. Local builds are optional — see the CI workflow for the canonical build configuration.

---

### Clone

```powershell
git clone https://github.com/thrive-spectrexq/severance.git
cd severance
```

---

### Configure

```powershell
cmake -B build -S .
```

With sanitizers and debug symbols:

```powershell
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
```

---

### Build

```powershell
cmake --build build --config Release
```

---

### Run

```powershell
.\build\Release\severance.exe
```

---

### Run Tests

```powershell
cd build; ctest --output-on-failure
```

---

## Roadmap

### Phase 1 — Foundation *(complete)*

- [x] Core framework skeleton
- [x] Event bus
- [x] Main window shell
- [x] Dashboard layout
- [x] Logging infrastructure
- [x] Theme system (dark, light, high-contrast)

### Phase 2 — Daily Driver *(in progress)*

- [x] Process Explorer with full tree view and process tagging
- [x] System Dashboard with GPU and thermal monitoring
- [x] Global Search (`Ctrl+K`) across all domains
- [x] Command Palette (`Ctrl+Shift+P`)
- [x] Activity Timeline with zoom and per-type filtering
- [x] Thread pool for async event processing
- [x] SQLite-backed event store for persistence
- [x] Workspace Profiles

### Phase 3 — Deep Monitoring *(Completed)*

- [x] Network Monitor with live connection table and per-process graphs
- [x] File Activity Monitor with diff view
- [x] Cross-Event Correlation Engine with visual graph
- [x] Advanced GPU-accelerated graph widgets (Qt RHI / QOpenGLWidget)
- [x] ETW integration for kernel-level tracing
- [x] Smart Notifications with pattern-based detection

### Phase 4 — Intelligence *(Completed)*

- [x] AI Insights Module (local LLM via llama.cpp / Ollama)
- [x] Natural language system queries
- [x] Anomaly explanation engine
- [x] Baseline recording and comparison

### Phase 5 — Ecosystem

- [x] Plugin SDK (public API + full documentation)
- [x] Dynamic module loading at runtime
- [ ] Plugin Marketplace (in-app browser and installer)
- [ ] Plugin sandboxing and declared permission model

### Phase 6 — Isolation and Control *(Completed)*

- [x] Sandbox engine via Job Objects and Restricted Tokens
- [x] Isolation Profiles with enforcement
- [x] Security analysis tooling
- [x] Behavioral heuristics via `security_plugin`

### Phase 7 — Record and Report *(Completed)*

- [x] Session Recording and Playback (via Telemetry Reports)
- [x] Exportable Telemetry Reports (JSON, Markdown)
- [x] Incident annotation and session sharing

### Future — Cross-Platform Expansion

- [ ] Linux support (procfs, netlink, inotify, eBPF)
- [ ] macOS support (Endpoint Security Framework, libproc, kqueue)
- [ ] Companion Mode (view system data from phone/browser over LAN)

---

## Themes

Severance ships with three built-in themes and full support for community palettes defined as JSON files.

| Theme | Description |
|---|---|
| **Dark** | Deep teal accents on near-black. The default. |
| **Light** | Clean off-white with muted teal accents. |
| **High Contrast** | Maximum legibility for accessibility or bright ambient environments. |

Custom themes load at startup from the themes directory — no rebuild required.

---

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl+K` | Global Search |
| `Ctrl+Shift+P` | Command Palette |
| `Ctrl+1` through `Ctrl+6` | Switch between main views |
| `Ctrl+R` | Start/stop session recording |
| `Ctrl+,` | Open settings |
| `Ctrl+N` | New workspace profile |
| `F5` | Refresh current view |
| `Escape` | Close overlay / return to previous view |

---

## Contributing

Contributions are welcome.

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature`
3. Commit with a clear, descriptive message
4. Push to your fork and open a pull request

All submissions must pass CI, conform to `.clang-format`, pass `clang-tidy` with no new warnings, and include tests for any new behavior.

Please read [`docs/contributing.md`](docs/contributing.md) before submitting.

---

## License

This project is licensed under the [MIT License](LICENSE).

---

## Vision

Severance exists to answer three questions instantly:

**What opened that connection?**
**What did that process write?**
**Why is memory climbing and not coming back down?**

Not a collection of terminal commands to memorize. Not a cloud dashboard requiring an account. A native desktop application — fast, offline, and precise — that shows the full picture and explains it in plain language.

The long-term vision: a tool serious enough for security researchers conducting behavioral analysis, practical enough for developers tracking down performance regressions, intelligent enough to explain what it finds, and clear enough that anyone curious about their machine can start using it in minutes.
