# Project Progress

## Spec v1.0.4

### Status Overview
- **Core Foundation**: ✅ Completed
- **CLI Parsing**: ✅ Completed
- **Storage Logic (Scaffolded)**: ✅ Completed
- **Main UI (Win32 Scaffolding)**: ✅ Completed
- **Move Engine**: ✅ Completed
- **Secondary Windows & UX**: ✅ Completed
- **Testing Suite**: ✅ Completed

### Planned Phases
#### Phase 1: Foundation & Scaffolding
- [x] Project structure and CMake configuration.
- [x] Dependency management (nlohmann/json, GTest).
- [x] Basic Win32/Linux entry point scaffolding.

#### Phase 2: Foundation & Persistence
- [x] Implementation of `CommandLineParser` with full spec compliance.
- [x] Data model definitions (`Group`, `Settings`, etc.).
- [x] Unit testing for CLI parsing logic (All tests passed).
- [x] Scaffolding for `StorageManager`.

#### Phase(s) 3: Main UI & Group Management
- [x] Implement the main list view (Win32/WPF).
- [x] Build "Add/Modify Group" dialogs.
- [x] Implement destination validation and status icons (`✓`, `X`, `?`).
- [x] Integrate Search window and Clipboard usage.

#### Phase 4: The Move Engine (The Core)
- [x] Develop the two-stage Queue/Release system.
- [x] Build the background worker thread for file operations.
- [x] Implement safety protocols and `.filemove-queued` sidecars.

#### Phase 5: Secondary Windows & UX
- [x] About window (Image rendering, layout compliance).
- [x] Settings window (Sort, Placement, Preview).
- [x] Status window (Queue status, JSON/Log management).
- [x] Shutdown prompt implementation (Custom modal dialog with spec-defined options).

#### Phase 6: Logging, Debugging & Integration
- [x] Implement dual-format logging (CSV and `---->`).
- [x] Implement Debug Mode (`/D`) and console output.
- [x] Full Explorer integration (Drag & Drop, Clipboard).

#### Phase 7: Feature Implementation
- [suspended] UI density audit.
- [suspended] Comprehensive end-to-end testing.

---

## Spec v1.1.0

### Summary of Changes
- **Enhanced Tooltips**: Group tooltips now display group names and per-destination status icons.
- **Queue Window**: Added a new non-modal window for real-time queue monitoring.
- **New Settings Options**: Added configuration for Directory Moves, Sidecar Files, and Hidden Source files.
- **Expanded Context Menu**: `New / Settings` menu now includes the `Queue Window` with dynamic count.

### Status Overview
- Data Model: ✅ Completed

### Implementation
#### Phase 1: Data Model & Persistence
- [x] Update Data Model for new settings (Directory Moves, Sidecars, Hidden Files).

### Planned Phases
#### Phase 2: UI Updates
- [x] Implement `Queue` window (non-modal, real-time updates).
- [x] Enhance Group Tooltips with per-destination status icons.
- [x] Update Settings UI with new configuration options.

#### Phase 3: Engine Expansion
- [x] Expand Directory support in Move Engine (recursive expansion).

#### Phase 4: Refinement & QA
- [ ] UI density audit.
- [ ] Comprehensive end-to-end testing.

