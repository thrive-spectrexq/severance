# Severance

> A modern C++ desktop application for visualizing, monitoring, and isolating system activity through a plugin-driven architecture.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-23-blue.svg)
![Qt](https://img.shields.io/badge/Qt-6-green.svg)
![Build](https://img.shields.io/github/actions/workflow/status/your-org/severance/ci.yml)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)

---

## Overview

The goal is to provide a unified workspace for observing and managing system activity through an intuitive graphical interface.

Inspired by the concept of separation and isolation, Severance allows users to analyze processes, monitor system events, inspect network activity, and manage isolated execution environments — all through a modular plugin system that can be extended without touching core code.

---

## Core Principles

### Modern C++

- C++23 throughout
- RAII everywhere
- Smart pointers only — no raw ownership
- Strong type safety
- STL-first design

### Performance

- Low memory footprint
- Native execution — no runtime overhead
- Multi-threaded event processing
- Efficient data structures built for high-frequency events

### Extensibility

- Plugin architecture with a public SDK
- Event-driven communication via a central event bus
- Dynamic module loading at runtime
- Clean separation between core and extensions

### Cross-Platform

- Windows
- Linux
- macOS

---

## Features

### Dashboard

A unified system overview in a single glance.

- Active processes at a glance
- Memory usage trends
- CPU statistics
- Network activity summary
- Live event timeline

---

### Process Explorer

Deep visual inspection of running processes.

- Full process tree with parent-child relationships
- Per-process resource usage
- Search and filtering by name, PID, or user
- Sortable columns and detail panels

---

### Activity Timeline

Track system activity as it happens, in real time.

- Process creation and termination
- File access events
- Network connection opens and closes
- Plugin-emitted events
- Scrollable, zoomable timeline view

---

### Network Monitor

Real-time visibility into network activity.

- Active connection list with protocol and state
- Port usage breakdown
- Historical connection log
- Per-process traffic statistics

---

### File Activity Monitor

Observe filesystem changes as they occur.

- File creation, deletion, and modification events
- Watched directory configuration
- Filtering by path, extension, or process
- Event history with timestamps

---

### Isolation Profiles

Define and enforce isolated execution environments.

- Sandbox definitions with granular permissions
- Resource limits per profile
- Process group assignment
- Execution policies for controlled environments

---

### Plugin System

Extend Severance without modifying core code.

Built-in examples:

- Network plugins
- Security plugins
- Custom monitoring plugins
- Custom visualizations and graph widgets

---

## Architecture

```
+------------------------------------------------+
|                    GUI Layer                   |
+------------------------------------------------+
| Dashboard | Graphs | Timeline | Settings       |
+------------------------------------------------+
|                  Event Bus                     |
+------------------------------------------------+
| Core Services                                  |
|------------------------------------------------|
| Process Manager                                |
| Network Manager                                |
| Filesystem Monitor                             |
| Sandbox Manager                                |
| Plugin Manager                                 |
+------------------------------------------------+
| Platform Layer                                 |
|------------------------------------------------|
| Windows API                                    |
| Linux APIs                                     |
| macOS APIs                                     |
+------------------------------------------------+
```

The GUI layer communicates exclusively through the **Event Bus** — core services never reach up into the interface directly. Platform-specific implementations are compiled conditionally and hidden behind stable interfaces, keeping cross-platform logic out of the core.

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
│   ├── contributing.md
│   └── roadmap.md
│
├── assets/
│   ├── icons/
│   ├── themes/
│   ├── fonts/
│   └── screenshots/
│
├── third_party/
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
│   │   └── macos/
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
│   ├── sample_plugin/
│   ├── network_plugin/
│   ├── filesystem_plugin/
│   └── security_plugin/
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

## Technology Stack

| Component        | Technology     |
| ---------------- | -------------- |
| Language         | C++23          |
| GUI Framework    | Qt 6           |
| Build System     | CMake 3.28+    |
| Package Manager  | vcpkg          |
| Logging          | spdlog         |
| Testing          | Catch2         |
| Code Formatting  | clang-format   |
| Static Analysis  | clang-tidy     |
| CI/CD            | GitHub Actions |

---

## Build

### Requirements

- C++23-capable compiler (GCC 13+, Clang 16+, MSVC 2022+)
- Qt 6.5 or later
- CMake 3.28 or later
- Git

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

To enable sanitizers or set a build type:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
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

### Phase 1 — Foundation

- [x] Core framework skeleton
- [x] Event bus
- [x] Main window shell
- [x] Dashboard layout
- [x] Logging infrastructure

### Phase 2 — Observability

- [ ] Process explorer with tree view
- [ ] Activity timeline system
- [ ] Thread pool for async event processing
- [ ] Data persistence layer

### Phase 3 — Monitoring

- [ ] Network monitoring with live connections
- [ ] File activity monitor
- [ ] Advanced graph widgets

### Phase 4 — Extensibility

- [ ] Plugin SDK (public API + documentation)
- [ ] Dynamic module loading
- [ ] Plugin marketplace support

### Phase 5 — Isolation

- [ ] Sandbox engine
- [ ] Isolation profiles
- [ ] Security analysis tooling

---

## Contributing

Contributions are welcome and appreciated.

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Commit your changes with a clear message
4. Push to your fork and open a pull request

Please read [`docs/contributing.md`](docs/contributing.md) before submitting. All code must pass CI, conform to `.clang-format`, and pass `clang-tidy` checks with no new warnings.

---

## License

This project is licensed under the [MIT License](LICENSE).

---

## Vision

Severance aims to become a powerful open-source desktop platform for observing, understanding, and controlling system activity — built on modern C++ engineering and a clean, extensible architecture.

The long-term goal is a tool serious enough for security researchers and systems engineers, yet approachable enough that any developer can drop it onto a machine and immediately understand what's happening under the hood.

A polished release could eventually include:

**Observability**
- Process Explorer — deep inspection of running processes, threads, and handles
- Network Monitor — live traffic analysis with per-connection breakdowns
- File Activity Monitor — filesystem event stream with filtering and alerting

**Control**
- Sandbox Profiles — define and enforce isolated execution environments with resource policies
- Session Recording — capture and replay full system activity sessions for post-mortem analysis

**Ecosystem**
- Plugin Marketplace — discover, install, and update community-built extensions in-app
- Theme System — first-class theming support with light, dark, and custom palette definitions
- Exportable Telemetry Reports — generate structured reports from captured sessions for sharing or archival
