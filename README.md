# Severance

> A modern C++ desktop application for visualizing, monitoring, and isolating system activity — with a clean, precise interface built for daily use.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![Qt](https://img.shields.io/badge/Qt-6-green.svg)
![Build](https://img.shields.io/github/actions/workflow/status/thrive-spectrexq/severance/ci.yml)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)

---

## Overview

Severance is a unified workspace for observing and controlling what your machine is doing — in real time, with clarity.

Most system tools scatter the picture across a dozen utilities. Severance brings it together: processes, network connections, filesystem events, and sandboxed execution environments — all in one application, all speaking to each other through a shared event model.

Built on C++23 and Qt 6, it is designed to be fast, precise, and extensible. Whether you are a developer tracking down a rogue background process, a security researcher watching what a binary actually does, or a power user who simply wants to know what is happening on their machine — Severance is built for daily, habitual use.

---

## Why Severance?

Most monitoring tools are either too narrow (one metric, one view) or too noisy (dashboards that bury signal in decoration). Severance is built on a different premise: **complete separation of concerns, with deep cross-context connection**.

Each module observes one domain cleanly. The correlation engine links events across all of them. You can start from a suspicious network connection and follow it back to the process that opened it, the file it read before doing so, and the parent that spawned it — without switching tools or losing context.

---

## Core Principles

**Modern C++**
C++23 throughout. RAII everywhere. Smart pointers only — no raw ownership. Strong type safety. STL-first design.

**Performance**
Low memory footprint. Native execution with no runtime overhead. Multi-threaded event processing. Efficient data structures built for high-frequency system events.

**Extensibility**
Plugin architecture with a public SDK. Event-driven communication via a central event bus. Dynamic module loading at runtime. Clean separation between core and extensions.

**Cross-Platform**
Windows, Linux, and macOS. Platform-specific implementations are compiled conditionally and hidden behind stable interfaces.

---

## Features

### System Dashboard

Your machine's health, at a glance, before you drill into anything.

- CPU usage per core with trend history
- Memory, swap, and virtual memory breakdown
- Disk I/O summary per device
- Network activity summary across all interfaces
- Live event ticker for high-priority system activity
- Anomaly indicators — spikes, stalls, and unexpected pattern changes
- Thermal and power draw data where platform APIs permit
- Customizable widget layout; arrange the view to suit your workflow

---

### Process Explorer

Deep, real-time inspection of every running process.

- Full process tree with parent-child relationships
- Per-process CPU, memory, disk I/O, and thread count
- Handle and file descriptor enumeration
- Search and filter by name, PID, user, or binary path
- Sortable columns and expandable detail panels
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
- Exportable event logs in JSON, CSV, or structured report formats

---

### Network Monitor

Complete real-time visibility into every connection your machine makes or accepts.

- Live connection list with protocol, state, and duration
- Per-process traffic statistics with historical graphs
- Port usage breakdown and anomaly detection
- DNS resolution log
- Unusual outbound connection alerting
- Geographic IP resolution and map visualization
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

### Isolation Profiles

Define controlled execution environments and enforce them at the OS level.

- Sandbox profiles with granular permission scopes
- Resource limits per profile: CPU, memory, disk I/O, network bandwidth
- Process group assignment and enforcement
- Execution policies: allow list, deny list, or audit-only mode
- Profile export and import for reproducible environments
- Violation logging with full event trace
- OS-native enforcement: Seccomp on Linux, App Sandbox on macOS, Job Objects on Windows

---

### Cross-Event Correlation Engine

The feature that turns isolated observations into a complete picture.

Events from the process explorer, network monitor, file activity monitor, and timeline are linked automatically. Follow any event in any direction:

- From a network connection → to the process that opened it → to the files it read first
- From a file write → to the process responsible → to its full resource history
- From a process → to every network and filesystem action it has ever taken in the session

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

Extend Severance without touching core code. The plugin SDK exposes the full event model, UI injection points, and data export hooks.

**SDK surface area:**
- Event subscription and emission via the Event Bus
- UI panel and widget injection
- Custom timeline event types
- Dashboard widget registration
- Data export format hooks
- Settings schema contribution

**Bundled plugins:**

| Plugin | Purpose |
|---|---|
| `network_plugin` | Deep traffic analysis, anomaly alerting, per-connection bandwidth tracking |
| `filesystem_plugin` | Extended file event attribution and bulk-write pattern detection |
| `security_plugin` | Behavioral heuristics: process hollowing indicators, unusual parentage, suspicious write patterns |
| `graph_plugin` | Custom visualization widgets for the dashboard and timeline |
| `export_plugin` | Structured telemetry report generation (JSON, CSV, PDF, Markdown) |
| `process_scanner` | Heuristic process classification and risk scoring |

Community plugins install from the in-app Plugin Marketplace without restarting.

---

## Architecture

```
+-------------------------------------------------------------------------+
|                             GUI Layer                                   |
|-------------------------------------------------------------------------|
| Dashboard | Process Explorer | Timeline | Network | Files | Isolation   |
+-------------------------------------------------------------------------+
|                            Event Bus                                    |
|              All modules communicate here — and only here               |
+-------------------------------------------------------------------------+
|                          Core Services                                  |
|-------------------------------------------------------------------------|
| Process Manager    | Network Manager  | File Monitor  | Sandbox Manager |
| Plugin Manager     | Session Recorder | Correlation Engine              |
| Thread Pool        | Event Store (SQLite)                               |
+-------------------------------------------------------------------------+
|                         Platform Layer                                  |
|-------------------------------------------------------------------------|
| Windows                  | Linux                  | macOS               |
| ETW, WFP, Job Objects    | procfs, netlink,       | Endpoint Security,  |
|                          | inotify, eBPF          | libproc, kqueue     |
+-------------------------------------------------------------------------+
```

The GUI layer communicates exclusively through the Event Bus. Core services never reach into the interface directly. Platform implementations are compiled conditionally and presented through stable interfaces — the same plugin or UI code runs identically on all three platforms.

---

## Technology Stack

| Component | Technology |
|---|---|
| Language | C++23 |
| GUI Framework | Qt 6.5+ |
| Build System | CMake 3.28+ |
| Package Manager | vcpkg |
| Event Store | SQLite (via SQLiteCpp) |
| Logging | spdlog |
| Testing | Catch2 |
| Tracing — Linux | eBPF / libbpf |
| Tracing — macOS | Endpoint Security Framework |
| Tracing — Windows | ETW + Windows Filtering Platform |
| Sandboxing | OS-native (Seccomp / App Sandbox / Job Objects) |
| Code Formatting | clang-format |
| Static Analysis | clang-tidy |
| CI/CD | GitHub Actions |

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
│   ├── ebpf-integration.md
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
│       ├── platform/
│       └── utils/
│
├── src/
│   ├── main.cpp
│   │
│   ├── core/
│   │   ├── application/
│   │   │   ├── Application.cpp
│   │   │   └── Application.hpp
│   │   ├── events/
│   │   │   ├── EventBus.cpp
│   │   │   ├── EventBus.hpp
│   │   │   ├── Event.hpp
│   │   │   └── EventTypes.hpp
│   │   ├── process/
│   │   │   ├── ProcessManager.cpp
│   │   │   ├── ProcessManager.hpp
│   │   │   ├── ProcessInfo.hpp
│   │   │   └── ProcessTree.hpp
│   │   ├── network/
│   │   │   ├── NetworkManager.cpp
│   │   │   ├── NetworkManager.hpp
│   │   │   └── ConnectionInfo.hpp
│   │   ├── filesystem/
│   │   │   ├── FileMonitor.cpp
│   │   │   ├── FileMonitor.hpp
│   │   │   └── FileEvent.hpp
│   │   ├── sandbox/
│   │   │   ├── SandboxManager.cpp
│   │   │   ├── SandboxManager.hpp
│   │   │   ├── SandboxProfile.hpp
│   │   │   └── IsolationPolicy.hpp
│   │   ├── plugins/
│   │   │   ├── PluginManager.cpp
│   │   │   ├── PluginManager.hpp
│   │   │   ├── IPlugin.hpp
│   │   │   └── PluginLoader.cpp
│   │   ├── correlation/
│   │   │   ├── CorrelationEngine.cpp
│   │   │   └── CorrelationEngine.hpp
│   │   ├── session/
│   │   │   ├── SessionRecorder.cpp
│   │   │   ├── SessionRecorder.hpp
│   │   │   ├── SessionPlayer.cpp
│   │   │   └── SessionPlayer.hpp
│   │   └── logging/
│   │       ├── Logger.cpp
│   │       └── Logger.hpp
│   │
│   ├── gui/
│   │   ├── windows/
│   │   │   ├── MainWindow.cpp
│   │   │   ├── MainWindow.hpp
│   │   │   ├── SettingsWindow.cpp
│   │   │   └── SettingsWindow.hpp
│   │   ├── dashboard/
│   │   │   ├── DashboardView.cpp
│   │   │   └── DashboardView.hpp
│   │   ├── process_view/
│   │   │   ├── ProcessView.cpp
│   │   │   └── ProcessView.hpp
│   │   ├── timeline/
│   │   │   ├── TimelineView.cpp
│   │   │   └── TimelineView.hpp
│   │   ├── network_view/
│   │   │   ├── NetworkView.cpp
│   │   │   └── NetworkView.hpp
│   │   ├── file_view/
│   │   │   ├── FileView.cpp
│   │   │   └── FileView.hpp
│   │   ├── isolation_view/
│   │   │   ├── IsolationView.cpp
│   │   │   └── IsolationView.hpp
│   │   ├── graphs/
│   │   │   ├── GraphWidget.cpp
│   │   │   └── GraphWidget.hpp
│   │   └── widgets/
│   │       ├── CpuWidget.cpp
│   │       ├── MemoryWidget.cpp
│   │       ├── NetworkWidget.cpp
│   │       └── StatusBarWidget.cpp
│   │
│   ├── platform/
│   │   ├── windows/
│   │   ├── linux/
│   │   │   └── ebpf/
│   │   └── macos/
│   │       └── endpoint_security/
│   │
│   └── utils/
│       ├── ThreadPool.cpp
│       ├── ThreadPool.hpp
│       ├── UUID.cpp
│       ├── UUID.hpp
│       ├── Config.cpp
│       └── Config.hpp
│
├── plugins/
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
│   ├── bootstrap.sh
│   └── package.sh
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

- C++23-capable compiler: GCC 13+, Clang 16+, or MSVC 2022+
- Qt 6.5 or later
- CMake 3.28 or later
- Git
- **Linux only:** kernel 5.15+ for eBPF features; `libbpf` and `libelf`
- **macOS only:** macOS 13+; Endpoint Security entitlement required for full system event access

---

### Clone

```bash
git clone https://github.com/thrive-spectrexq/severance.git
cd severance
```

---

### Configure

```bash
cmake -B build -S .
```

With sanitizers and debug symbols:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
```

With eBPF support on Linux:

```bash
cmake -B build -S . -DENABLE_EBPF=ON
```

---

### Build

```bash
cmake --build build --config Release
```

---

### Run

```bash
./build/severance
```

---

### Run Tests

```bash
cd build && ctest --output-on-failure
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

### Phase 2 — Observability

- [ ] Process Explorer with full tree view
- [ ] Activity Timeline with zoom and per-type filtering
- [ ] Thread pool for async event processing
- [ ] SQLite-backed event store for persistence
- [ ] Cross-Event Correlation Engine

### Phase 3 — Monitoring

- [ ] Network Monitor with live connection table and per-process graphs
- [ ] File Activity Monitor with diff view
- [ ] Advanced graph widgets
- [ ] eBPF integration (Linux)
- [ ] Endpoint Security Framework integration (macOS)

### Phase 4 — Extensibility

- [ ] Plugin SDK (public API + full documentation)
- [ ] Dynamic module loading at runtime
- [ ] Plugin Marketplace (in-app browser and installer)
- [ ] Plugin sandboxing and declared permission model

### Phase 5 — Isolation and Control

- [ ] Sandbox engine
- [ ] Isolation Profiles with OS-native enforcement
- [ ] Security analysis tooling
- [ ] Behavioral heuristics via `security_plugin`

### Phase 6 — Record and Report

- [ ] Session Recording and Playback
- [ ] Exportable Telemetry Reports (JSON, CSV, PDF, Markdown)
- [ ] Incident annotation and session sharing

---

## Themes

Severance ships with three built-in themes and full support for community palettes defined as JSON files.

| Theme | Description |
|---|---|
| **Dark** | Deep teal accents on near-black. The default. |
| **Light** | Clean off-white with muted teal accents. |
| **High Contrast** | Maximum legibility for accessibility or bright ambient environments. |

Custom themes load at startup from the themes directory, no rebuild required.

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

Severance is built toward one goal: give any developer, engineer, or technically-minded user a single application they can open on any machine and immediately understand what that machine is doing.

Not a collection of terminal commands to memorize. Not a cloud dashboard requiring an account. A native desktop application — fast, offline, and precise — that shows the full picture: processes, network, filesystem, and their relationships, all in one place.

The long-term vision includes a tool serious enough for security researchers conducting behavioral analysis, practical enough for developers tracking down performance regressions, and clear enough that anyone curious about their machine can start using it in minutes.

**What opened that connection?**
**What did that process write?**
**Why is memory climbing and not coming back down?**

Severance answers these questions without ceremony.
