# Severance
> "The work is mysterious and important."
> Deep, interconnected visibility into the severed floor — innies, perimeter grid, memory compartments, and their relationships — all in one place.

*Inspired by Lumon Industries's Macrodata Refinement application from Apple TV's 'Severance'.*

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-23-1A7A5C.svg)
![C](https://img.shields.io/badge/C-17-1A7A5C.svg)
![Qt](https://img.shields.io/badge/Qt-6-1B4F72.svg)
![Build](https://img.shields.io/github/actions/workflow/status/thrive-spectrexq/severance/ci.yml)
![Platform](https://img.shields.io/badge/platform-Windows-132A2E.svg)

---

## Overview

Welcome to the Severance Monitoring Platform, a proprietary system developed by Lumon Industries. This desktop application allows Management to understand what our severed employees (innies) are doing — in real time, with clinical clarity and absolute depth.

Most outside system tools scatter the picture across a dozen messy utilities. Our platform brings it all together in a pristine, unified environment. Innie Activity Threads, Perimeter Communications, Memory Compartment Surveillance, and Severance Containment Protocols — all linked through our cross-domain correlation engine. 

Built on C++23 and C17 with Qt 6. The core application uses modern C++23. The extension protocol exposes a stable C17 ABI. Designed exclusively for Lumon Management, compliance officers, and the Department of Vigilance.

> **Current Focus: The Severed Floor (Windows).** Expansion to other departments is planned for future phases.

---

> "Let not weakness live in your veins. Cherished workers, drown it inside you." — Kier Eagan

## Why the Severance Monitoring Platform?

Our system is built on a fundamental principle of Lumon: **complete separation of concerns, with deep corporate visibility.**

Each module observes one domain cleanly. The correlation engine links events across all of them. The Supplemental Intelligence module assesses compliance. You don't need to be a neurosurgeon to understand the severed floor — you just need your terminal open.

### The Lumon Difference

| Outside Tools | Lumon Severance Platform |
|---|---|
| Scattered across 6+ tools | A unified, sterile interface |
| Raw data dumps | Supplemental Intelligence compliance assessments |
| Single-domain views | Deep correlation — follow any innie's full behavioral chain |
| Cluttered interfaces | Premium clinical aesthetic designed for optimal focus |
| Threshold-based alerts | Pattern-based Observation Protocol notifications |
| Static dashboards | Searchable, filterable, and exact |

---

## Macrodata Refinement (Number Grid Generation)

**GENERATE**
A N×N grid is created. Each cell is filled with a random integer between 0-9.

**MAP**
A Perlin noise map assigns smooth gradient values to the grid. This ensures numbers are naturally grouped rather than randomly scattered.

**THRESHOLD**
Each cell's value is compared to a set threshold. Values above the threshold are marked as 'bad'/'scary' numbers.

**GROUP**
'Bad' numbers are grouped based on proximity to one another. At random intervals, a group visible in the viewport will be activated:
- If hovered over, they become 'super active', extending their active time and appearing agitated.
- If clicked, the group is 'refined', animating into a pre-determined bin and resetting as no longer 'bad'.

### The Interface

- A moving Perlin noise map offsets each number (vertically or horizontally).
- Activated bad groups expand / contract / jitter.
- Numbers scale-up based on cursor position.
- Refined groups animate into bins, with a percent bar keeping track of your progress.

### Additional Features
- The application is cross-compatible, allowing compilation for both Linux and Raspberry Pi (ARM).
- A 'debug mode' reveals various settings (which can be saved / loaded from disk as JSON).
- An 'idle mode' can be enabled to display the Lumon logo screensaver.
- The user can navigate the full grid using arrow and zoom keys.

---

## Core Protocols

**Modern C++ with a Stable C ABI**
The platform is built with structural integrity. C++23 throughout the core. RAII everywhere. The Extension Protocol exposes a C17 ABI boundary — maintaining total isolation between core systems and departmental plugins.

**Performance**
Sub-100ms response to every interaction, ensuring absolute precision when monitoring the severed floor. Low memory footprint. Native execution.

**Keyboard-First Navigation**
Every action is reachable without a mouse. Navigate the system with the efficiency Kier intended.

**Departmental Extensibility**
Dynamic module loading. Clean separation between core monitoring and departmental extensions.

**Supplemental Intelligence**
Local AI integration for compliance assessment, behavioral summarization, and natural-language queries. 100% contained — no data leaves the severed floor.

---

## System Departments

### Macrodata Refinement Console
Your primary view into the health of the severed floor.

- Innie capacity usage per core
- Memory compartment breakdowns
- Perimeter grid activity summary
- Live event ticker for high-priority behavioral anomalies
- Customizable widget layout; arrange your terminal to suit your supervisory duties

### Personnel Registry (Process Explorer)
Deep, real-time inspection of every active Innie.

- Full hierarchy with parent-child relationships
- Per-innie cognitive and resource consumption
- Search and filter by name, ID, or activity
- Tagging and notes — annotate innies ("compliant", "defiant jazz", "investigate")
- Abnormal termination detection and flagging

### Temporal Ledger (Activity Timeline)
A single scrollable view of everything that has occurred during the shift.

- Innie creation, suspension, and termination events
- Memory compartment access and mutation
- Perimeter grid connection lifecycles
- Click any event to see its full cross-context chain
- Exportable logs for the Department of Vigilance

### Perimeter Grid (Network Monitor)
Complete real-time visibility into every connection attempting to breach the severed floor.

- Live connection list with protocol, state, and duration
- Per-innie traffic statistics
- Unusual outbound connection alerting (leakage prevention)
- Deep inspection protocols via departmental extensions

### Document Processing Surveillance (File Monitor)
Watch the memory compartments as events happen.

- Creation, modification, deletion, and rename events
- Filter by compartment, extension, innie, or event type
- Alerts on modification to sensitive paths
- Bulk write detection for identifying frantic or anomalous behavior

### Global Search & Command Palette
Find anything. Do anything. With perfect efficiency.

**Global Search** (`Ctrl+K`):
- Search across all innies, grid connections, memory events, and the temporal ledger.

**Command Palette** (`Ctrl+Shift+P`):
- Execute actions by name: "Suspend all active innies", "Start observation recording"
- Context-aware suggestions based on your current department view

### Supplemental Intelligence Module
Revolving assessment that turns raw behavioral data into understanding.

- **Anomaly Explanation**: Get a plain-language explanation of why an innie's behavior is unusual.
- **Behavioral Summary**: "In the last hour, this innie accessed 847 memories, attempted 12 perimeter connections, and exhibited signs of defiance."
- **Baseline Comparison**: Record a "compliant" state → compare current behavior → highlight deviations.

### Smart Notifications
Alerts that understand context, ensuring total compliance.

- Escalation levels: subtle indicator → toast → sound → system notification
- Configurable per-innie rules
- Notification history with links back to the triggering event chain

### Severance Containment Protocols
Define controlled execution environments and enforce them at the deepest level.

- Isolation profiles with granular permission scopes
- Resource limits per profile: cognitive load, memory access
- Execution policies: allow list, deny list, or audit-only mode
- Violation logging with full event trace

### Observation Protocol (Session Recording)
Capture a complete shift and replay it at any time.

```
[REC] ● 00:04:32  —  Recording: "incident_helly_r"
      Behaviors captured:  84,201
      Innies tracked:      4
      Memory events:       9,047
      Perimeter events:    2,103
```
Useful for incident post-mortems, compliance audits, and sharing observations with the Board.

---

> "Be merry, for you are the chosen ones." — Kier Eagan

## Build Protocol

### Requirements

- **Windows 10 21H2 or later** (Windows 11 recommended for full containment)
- C++23 and C17 capable compiler: MSVC 2022 (17.8+)
- Qt 6.5 or later
- CMake 3.28 or later
- Git
- vcpkg

> **Note:** All builds run through our automated compliance pipelines (GitHub Actions). Local builds are optional.

### Clone

```powershell
git clone https://github.com/thrive-spectrexq/severance.git
cd severance
```

### Configure

```powershell
cmake -B build -S .
```

With sanitizers and debug symbols:

```powershell
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
```

### Build

```powershell
cmake --build build --config Release
```

### Run

```powershell
.\build\Release\severance.exe
```

---

## Aesthetics & Compliance

The platform ships with the standard Lumon aesthetic to enforce focus and compliance.

| Theme | Description |
|---|---|
| **Lumon Dark** | Deep teal accents on near-black. The default standard. |
| **Lumon Light** | Clean off-white with muted teal accents. |
| **High Contrast** | Maximum legibility for specialized observation environments. |

---

## Supervisory Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl+K` | Global Search |
| `Ctrl+Shift+P` | Command Palette |
| `Ctrl+1` - `Ctrl+6` | Switch between department views |
| `Ctrl+R` | Start/stop Observation Protocol |
| `Ctrl+,` | Open Severance Configuration |
| `Ctrl+N` | New containment profile |
| `F5` | Refresh current view |
| `Escape` | Close overlay / return to previous view |

---

## Department Onboarding (Contributing)

Contributions to the work are welcome. Please enjoy each task equally.

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-refinement`
3. Commit with a clear, descriptive message
4. Push to your fork and open a pull request

All submissions must pass the compliance pipeline, conform to `.clang-format`, pass `clang-tidy` with no new warnings, and include tests for any new behavior.

---

## License

This project is licensed under the [MIT License](LICENSE).

