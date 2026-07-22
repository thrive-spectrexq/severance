# Severance
> "The work is mysterious and important."
> A faithful recreation of the Macrodata Refinement terminal from Lumon Industries.

*Inspired by the Macrodata Refinement application from Apple TV's 'Severance'.*

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-23-1A7A5C.svg)
![C](https://img.shields.io/badge/C-17-1A7A5C.svg)
![Qt](https://img.shields.io/badge/Qt-6-1B4F72.svg)
![Build](https://img.shields.io/github/actions/workflow/status/thrive-spectrexq/severance/ci.yml)
![Platform](https://img.shields.io/badge/platform-Windows-132A2E.svg)

---

## Overview

Welcome to Lumon Industries. You have been selected for Macrodata Refinement.

This desktop application recreates the experience of being an Innie on the Severed Floor. Refine data. Sort numbers into bins. Complete your quotas. Do not ask questions. Please enjoy each number equally.

Built on C++23 and C17 with Qt 6. The core application uses modern C++23. Designed with the institutional Lumon aesthetic — clinical, sterile, and precise.

> **Your Outie chose this life for you. You should be grateful.**

---

> "Let not weakness live in your veins. Cherished workers, drown it inside you." — Kier Eagan

## Macrodata Refinement

Your primary task. The work that defines your existence on the Severed Floor.

### The Number Grid

A vast grid of numbers fills your terminal. Each cell contains a digit from 0–9, arranged across a field of Perlin noise-mapped gradients. The numbers are not random. They are waiting.

**GENERATE**
An N×N grid is created. Each cell is filled with a random integer between 0–9.

**MAP**
A Perlin noise map assigns smooth gradient values to the grid. This ensures numbers are naturally grouped rather than randomly scattered.

**THRESHOLD**
Each cell's value is compared to a set threshold. Values above the threshold are marked as 'scary' numbers — the ones that evoke dread, malice, woe, or frolic.

**GROUP**
'Scary' numbers are grouped based on proximity to one another. At random intervals, a group visible in the viewport will be activated:
- If hovered over, they become 'super active', extending their active time and appearing agitated.
- If clicked, the group is 'refined', animating into a pre-determined bin and resetting.

### The Bins

Five bins receive your refined data. Each represents a category that you will come to understand through feeling, not explanation:

| Bin | Meaning |
|---|---|
| **WO** | Woe |
| **FC** | Frolic |
| **DR** | Dread |
| **MA** | Malice |
| **—** | The fifth category |

Each bin has a progress bar. Fill all five to complete your quota.

### The Experience

- A moving Perlin noise map offsets each number vertically and horizontally, giving the grid an organic, living quality.
- Activated scary groups expand, contract, and jitter — they feel *alive*.
- Numbers scale up as your cursor approaches, reacting to your presence.
- Refined groups animate smoothly into their assigned bin, with progress bars tracking your quota completion.
- The grid can be navigated using arrow keys and zoom controls.

### Idle Mode

Step away from your terminal and the Lumon Industries logo screensaver will appear. The work will wait. It always does.

### Debug Mode

A hidden debug mode reveals the underlying parameters — noise thresholds, activation intervals, grid dimensions — which can be saved and loaded from disk as JSON.

---

## The Severed Floor

Beyond Macrodata Refinement, the application provides views into the other departments and protocols of Lumon Industries.

### Personnel Registry

Review the profiles of your fellow Innies. Mark S., Helly R., Irving B., Dylan G. — each with their compliance score, defiance index, and severance chip status. You may annotate their records. You may not befriend them excessively.

### Perimeter Grid

An interactive floor plan of the Severed Floor. Macrodata Refinement. Optics & Design. The Break Room. Management. Navigate the labyrinth of identical hallways. Watch the elevator pulse between SEVERED and NON-SEVERED floors. Motion sensors ping at random intervals.

### Temporal Ledger

A scrollable timeline of everything that has occurred during the shift. Innie arrivals, data refinement events, break room sessions, perimeter movements. Each event is timestamped. Each event is logged. Each event is watched.

### Break Room

*"Forgive me for the damage I have done this world. Neither combative nor competitive. I will do no harm."*

The disciplinary view. When compliance fails, the Break Room awaits. Read the statement. Read it again. Read it until you mean it.

### Severance Chip Status

A clinical display of your severance chip's operational parameters. Signal strength, memory partition integrity, containment compliance percentage. All nominal. Always nominal.

### Observation Protocol

Capture complete shift recordings for review by Management. Behaviors captured, Innies tracked, refinement events, perimeter events — all logged with clinical precision.

```
[REC] ● 00:04:32  —  Recording: "incident_helly_r"
      Behaviors captured:  84,201
      Innies tracked:      4
      Refinement events:   9,047
      Perimeter events:    2,103
```

### Board Communications

A terminal interface for receiving directives from The Board. Messages appear. You may respond. The Board may or may not be satisfied with your answers. The Board's communications are not to be questioned.

### Optics & Design

A gallery of procedural artwork created by the Optics & Design department. Abstract vector compositions with titles from Lumon lore: "The Kier Collection", "Macrodata Topography", "Waffle Party", "Defiant Jazz".

### Supplemental Intelligence

A local AI assistant for Lumon inquiries. Ask about protocol, compliance, the Eagan family legacy, or the nature of your work. Answers are provided. Understanding is not guaranteed.

---

> "Be merry, for you are the chosen ones." — Kier Eagan

## Build Protocol

### Requirements

- **Windows 10 21H2 or later** (Windows 11 recommended)
- C++23 and C17 capable compiler: MSVC 2022 (17.8+)
- Qt 6.5 or later
- CMake 3.28 or later
- Git
- vcpkg

> **Note:** All builds run through automated compliance pipelines (GitHub Actions). Local builds are optional.

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

## Aesthetics

The platform ships with the standard Lumon aesthetic to enforce focus and compliance.

| Theme | Description |
|---|---|
| **Lumon Dark** | Deep teal accents on near-black. Clinical. Sterile. The default standard. |
| **Lumon Light** | Clean off-white with muted teal accents. For specialized observation. |
| **High Contrast** | Maximum legibility for extended refinement sessions. |

The interface is characterized by:
- Near-black backgrounds with cold blue-green undertones
- Teal and cyan accents — the colors of institutional control
- Monospace typography — every character precisely placed
- Minimal UI chrome — the grid dominates
- Subtle glow effects on interactive elements
- Clinical precision in every pixel

---

## Terminal Navigation

| Shortcut | Action |
|---|---|
| `Ctrl+K` | Global Search |
| `Ctrl+Shift+P` | Command Palette |
| `Ctrl+1` - `Ctrl+0` | Switch between department views |
| `Ctrl+R` | Start/stop Observation Protocol |
| `Ctrl+,` | Open Configuration |
| `F5` | Refresh current view |
| `Escape` | Close overlay / return to previous view |
| `~` | Toggle terminal overlay |
| `Arrow Keys` | Navigate the Macrodata Refinement grid |
| `Scroll Wheel` | Zoom in/out of the grid |

---

## Contributing

Contributions to the work are welcome. Please enjoy each task equally.

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-refinement`
3. Commit with a clear, descriptive message
4. Push to your fork and open a pull request

All submissions must pass the compliance pipeline, conform to `.clang-format`, pass `clang-tidy` with no new warnings, and include tests for any new behavior.

---

## The Eagan Principles

> *"The remembered man does not decay."* — Kier Eagan

This application is a work of fiction inspired by the Apple TV+ series *Severance*. It is a fan project and is not affiliated with, endorsed by, or connected to Apple Inc., Endeavor Content, Red Hour Films, or any entity involved in the production of the series.

All Lumon Industries branding, character names, and lore references are the intellectual property of their respective owners and are used here in a transformative, non-commercial context.

---

## License

This project is licensed under the [MIT License](LICENSE).
