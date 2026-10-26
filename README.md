# Industrial Sensor Monitoring System (C++17)

A robust, modular, multithreaded console application developed in **C++17** to simulate real-time sensor monitoring in industrial manufacturing and chemical processing environments.

---

## Table of Contents
1. [Project Overview](#project-overview)
2. [Key Features](#key-features)
3. [Architecture & Class Responsibilities](#architecture--class-responsibilities)
4. [Technologies Used](#technologies-used)
5. [System Requirements](#system-requirements)
6. [Build Instructions (Windows)](#build-instructions-windows)
7. [Run Instructions](#run-instructions)
8. [Testing & Verification](#testing--verification)
9. [Multithreading & Concurrency Architecture](#multithreading--concurrency-architecture)
10. [Example Console Output](#example-console-output)
11. [Limitations & Future Enhancements](#limitations--future-enhancements)
12. [Interview & Learning Guide](#interview--learning-guide)

---

## Project Overview

In industrial facilities (such as chemical reactors, steam boilers, power turbines, and cold-chain storage), monitoring environmental parameters like temperature is safety-critical. This project simulates an automated supervisory system that:
- Ingests and tracks temperature readings across diverse sensors.
- Automatically calculates operational statuses (`NORMAL`, `WARNING`, `CRITICAL`).
- Displays a real-time **Status Dashboard** summarizing operational counts and peak temperature.
- Records event-driven alert history upon status transitions.
- Executes realistic asynchronous temperature variations via background worker threads.
- Ensures persistence to disk with error recovery, empty-file tolerance, and input validation.

---

## Key Features

- **Industrial State Machine**:
  - `NORMAL`: Current temperature is strictly below warning threshold ($T < T_{\text{warn}}$).
  - `WARNING`: Current temperature is at or above warning threshold but below critical threshold ($T_{\text{warn}} \le T < T_{\text{crit}}$).
  - `CRITICAL`: Current temperature has reached or exceeded critical threshold ($T \ge T_{\text{crit}}$).
- **Real-Time Operational Dashboard**:
  - Automatically displayed at the top of the console screen.
  - Dynamically calculates: Total sensors, count of `NORMAL`, `WARNING`, and `CRITICAL` sensors.
  - Highlights peak temperature with sensor ID and description.
  - Displays real-time simulation background status (`RUNNING` / `STOPPED`).
- **Comprehensive Sensor Catalog Management**:
  - Add sensors with duplicate ID prevention and threshold validation ($T_{\text{warn}} < T_{\text{crit}}$).
  - **Search by ID** and **Search by Name** (case-insensitive substring search).
  - Delete sensors with mandatory **user confirmation prompt** (`y/n`).
  - Edit sensor thresholds and names.
  - Filter sensors by operational status (`NORMAL`, `WARNING`, `CRITICAL`).
  - Sort sensors by temperature (ascending or descending).
- **Event-Driven Alert History**:
  - Captures transitions (`NORMAL` $\to$ `WARNING`, `WARNING` $\to$ `CRITICAL`, recovery to `NORMAL`).
  - **Prevents duplicate alert spamming** when temperature changes without a state transition.
  - **Filter alert history** by Sensor ID or by severity level (`WARNING`, `CRITICAL`, `NORMAL`).
  - Logs timestamps (`YYYY-MM-DD HH:MM:SS`), sensor IDs, names, and temperatures to `data/alerts.txt`.
- **Thread-Safe Simulation Engine**:
  - Background `std::thread` executing stochastic thermal delta vectors.
  - Synchronized via `std::mutex` and RAII-based `std::lock_guard`.
  - Controlled shutdown via `std::atomic<bool>` with 50ms time slicing for instant response.
  - Zero detached threads; clean destructor `join()` prevents `std::terminate()`.
- **Fault-Tolerant File Persistence**:
  - Two-phase commit file loading pattern: pre-existing in-memory state is never lost if a file read fails.
  - Handles missing files, empty files (0 bytes), and malformed lines gracefully.
  - Delimited format (`|`) safely handles sensor names with spaces.
- **Automated Testing Suite**:
  - Fully integrated with CMake `CTest`.
  - 5 comprehensive test suites covering 100% of core business logic.

---

## Architecture & Class Responsibilities

```
IndustrialSensorMonitor/
├── CMakeLists.txt              # CMake build configuration with C++17 and CTest
├── README.md                   # Technical documentation (English)
├── .gitignore                  # Git exclusions for C++, CMake, and VS Code
├── docs/
│   └── LEARNING_GUIDE_VI.md    # In-depth architectural & interview preparation guide (Vietnamese)
├── include/
│   ├── Sensor.h                # Sensor domain model and state calculation
│   ├── SensorManager.h         # In-memory sensor catalog and thread-safe CRUD
│   ├── FileManager.h           # File I/O parsing, validation, and serialization
│   ├── AlertManager.h          # Alert event history and timestamped logging
│   ├── SimulationEngine.h      # Background multithreaded simulation controller
│   └── ConsoleUI.h             # Console presentation, dashboard, menus, and safe input
├── src/
│   ├── main.cpp                # Application bootstrap, CLI routing, and unit test runner
│   ├── Sensor.cpp              # Sensor business logic implementation
│   ├── SensorManager.cpp       # Catalog management, search algorithms, ASCII tables
│   ├── FileManager.cpp         # Resilient file I/O implementation
│   ├── AlertManager.cpp        # Alert history tracking, filtering, and persistence
│   └── SimulationEngine.cpp    # Concurrency and thread lifecycle management
├── tests/
│   ├── CMakeLists.txt          # Dedicated test target definitions
│   └── SensorManagerTests.cpp  # Standalone unit test suite source
└── data/
    ├── sensors.txt             # Persistent sensor records
    └── alerts.txt              # Persistent alert transition logs
```

### Class Responsibilities

| Class | Responsibilities |
|---|---|
| `Sensor` | Encapsulates ID, name, current temperature, warning threshold, and critical threshold. Computes status and enforces invariant $T_{\text{warn}} < T_{\text{crit}}$. |
| `SensorManager` | Thread-safe container managing active sensors. Provides lookup by ID, substring search by Name, filtering, sorting, and dynamic dashboard statistics. |
| `FileManager` | Reads and writes configuration files. Implements atomic-like loading (preserves data if file read fails), empty-file tolerance, and malformed record skipping. |
| `AlertManager` | Tracks state transitions across sensors with millisecond-resolution timestamps, preventing duplicate alerts for identical states. Supports filtering by ID and severity. |
| `SimulationEngine` | Manages background worker thread lifecycle, applying randomized thermal delta vectors while ensuring zero data races or blocking sleep locks. |
| `ConsoleUI` | Handles all user interaction, ASCII table formatting, dashboard presentation, confirmation dialogs, and safe input parsing. |

---

## Technologies Used

- **C++17 Language Standard**:
  - `std::optional<T>` for safe nullability without raw pointers.
  - `std::filesystem` for platform-agnostic file management.
  - `std::atomic<bool>` for thread interruption flags.
  - `std::lock_guard<std::mutex>` for RAII-based mutual exclusion.
  - `enum class` for strongly typed scoped enumerations.
- **Build System**: CMake 3.20+ with CTest test harness.
- **Compiler**: GCC / G++ 16.2.0 (MSYS2 MinGW-w64).

---

## System Requirements

- **Operating System**: Windows 10/11 (or Linux/macOS with standard GCC/Clang)
- **Compiler**: GCC / G++ 16.2.0 (or any compiler supporting standard C++17)
- **Build System**: CMake 3.20 or newer (tested with CMake 4.5.0)
- **Build Tool**: MinGW Makefiles (`mingw32-make`) or Ninja

---

## Build Instructions (Windows)

1. Open PowerShell or Command Prompt in the project root directory:
   ```powershell
   cd C:\CppProjects\IndustrialSensorMonitor
   ```

2. Configure CMake with the MinGW generator:
   ```powershell
   cmake -G "MinGW Makefiles" -B build
   ```

3. Build the executable:
   ```powershell
   cmake --build build
   ```
   The binary `IndustrialSensorMonitor.exe` will be generated inside `build/`.

---

## Run Instructions

### 1. Interactive Console Mode
Run the executable directly:
```cmd
cmd /c .\build\IndustrialSensorMonitor.exe
```
Or in PowerShell:
```powershell
.\build\IndustrialSensorMonitor.exe
```

### 2. Main Menu Options
```
========================================================================
                INDUSTRIAL SENSOR MONITORING DASHBOARD                  
========================================================================
  Tong so cam bien:  5 | NORMAL:  3 | WARNING:  1 | CRITICAL:  1
  Nhiet do cao nhat: 98.20 C (Cam bien [103] 'Exhaust Turbine 2')
  Trang thai mo phong : [DANG DUNG]
========================================================================
 1.  Hien thi toan bo cam bien
 2.  Them cam bien moi
 3.  Cap nhat nhiet do (Thu cong)
 4.  Chinh sua thong tin & nguong canh bao
 5.  Xoa cam bien (Co xac nhan)
 6.  Tim kiem cam bien (Theo ID hoac theo Ten)
 7.  Loc cam bien theo trang thai (NORMAL/WARNING/CRITICAL)
 8.  Sap xep cam bien theo nhiet do
 9.  Xem va loc lich su canh bao (Alert Log)
 10. Bat mo phong nhiet do thoi gian thuc
 11. Tat mo phong nhiet do thoi gian thuc
 12. Luu du lieu ra file
 13. Nap du lieu tu file
 0.  Luu du lieu va Thoat
------------------------------------------------------------------------
```

---

## Testing & Verification

The project includes 5 comprehensive automated test suites:
1. **Sensor Classification & Boundary Tests**: Verifies boundary values ($T = T_{\text{warn}}$, $T = T_{\text{crit}}$), invalid thresholds, negative temperatures.
2. **SensorManager CRUD, Search & Dashboard Tests**: Verifies duplicate ID rejection, name substring search, and dashboard statistic calculation.
3. **FileManager Persistence & Resiliency Tests**: Verifies two-phase commit file loading, missing file tolerance, empty file handling, and skipping corrupted records.
4. **AlertManager Transition & Filter Tests**: Verifies status change detection, alert deduplication, filtering by ID, and filtering by severity.
5. **Multithreaded Simulation Engine Lifecycle Tests**: Verifies thread start/stop lifecycle, absence of duplicate threads, data integrity, and RAII clean shutdown.

### Running Tests via CTest
```powershell
ctest --test-dir build --output-on-failure
```

### Running Tests via Application Flag
```cmd
cmd /c .\build\IndustrialSensorMonitor.exe --test
```

---

## Multithreading & Concurrency Architecture

### Concurrency Guarantees
1. **Thread Lifecycle**:
   - The simulation thread is non-detached (`std::thread`).
   - The thread is safely joined when stopping the simulation or during application shutdown (`~SimulationEngine()`).
   - Duplicate thread invocations are guarded against via atomic check (`m_running.load()`).
2. **State Synchronization**:
   - Shared sensor data within `SensorManager` is protected by `mutable std::mutex` using RAII-based `std::lock_guard<std::mutex>`.
   - Thread interruption is controlled by `std::atomic<bool> m_running`.
3. **Prevention of Deadlocks and Blocking**:
   - **No mutex is held during sleep or I/O operations**.
   - The worker thread retrieves a lightweight snapshot copy of the sensor collection, immediately releases the lock, computes temperature deltas, and updates individual records under brief, scoped locks.
   - Sleep intervals are segmented into 50ms slices, ensuring the thread responds promptly to stop requests.
4. **Pointer and Reference Safety**:
   - Queries return values (`std::vector<Sensor>` or `std::optional<Sensor>`) instead of raw pointers or references to internal container elements, eliminating dangling references during concurrent vector modifications.

---

## Example Console Output

```
========================================================================
                INDUSTRIAL SENSOR MONITORING DASHBOARD                  
========================================================================
  Tong so cam bien:  5 | NORMAL:  3 | WARNING:  1 | CRITICAL:  1
  Nhiet do cao nhat: 98.20 C (Cam bien [103] 'Exhaust Turbine 2')
  Trang thai mo phong : [DANG DUNG]
========================================================================

======================================================
  DANH SACH TOAN BO CAM BIEN DANG QUAN LY
======================================================
+-----+---------------------------+------------+------------+------------+----------+
| ID  | Ten Cam Bien              |   Temp (C) |   Warn (C) |   Crit (C) | TrangThai |
+-----+---------------------------+------------+------------+------------+----------+
| 101 | Boiler Reactor 1          |      65.50 |      75.00 |      90.00 | NORMAL   |
| 102 | Coolant Tank A            |      48.00 |      45.00 |      60.00 | WARNING  |
| 103 | Exhaust Turbine 2         |      98.20 |      85.00 |      95.00 | CRITICAL |
| 104 | Cold Storage Freezer      |     -18.50 |     -10.00 |       0.00 | NORMAL   |
| 105 | Hydraulic Pump Alpha      |      40.00 |      50.00 |      70.00 | NORMAL   |
+-----+---------------------------+------------+------------+------------+----------+
Tong so: 5 cam bien.

======================================================
  LICH SU CANH BAO (ALERT HISTORY)
======================================================
+---------------------+-----+---------------------------+------------+-------------------------+
| Thoi Gian           | ID  | Ten Cam Bien              |   Temp (C) | Chuyen Trang Thai       |
+---------------------+-----+---------------------------+------------+-------------------------+
| 2026-10-26 21:44:26 | 106 | Furnace A                 |     125.00 | WARNING -> CRITICAL     |
+---------------------+-----+---------------------------+------------+-------------------------+
Tong so canh bao: 1
```

---

## Limitations & Future Enhancements

- **Physical Protocol Drivers**: Interface with real industrial hardware via Modbus TCP, OPC UA, or MQTT protocols.
- **Embedded Database**: Replace text file persistence with SQLite for relational querying or InfluxDB for time-series metrics.
- **Graphical Dashboard**: Add an interactive desktop GUI using Qt Widgets / QML.

---

## Interview & Learning Guide

A dedicated, comprehensive Vietnamese technical guide is available in [`docs/LEARNING_GUIDE_VI.md`](docs/LEARNING_GUIDE_VI.md). It details:
- Complete step-by-step architectural flows.
- Concurrency deep dive (deadlock prevention, lock scopes, atomic flags).
- Two-phase commit file persistence mechanism.
- Common C++ interview questions and model answers based on this codebase.
- Practical script for presenting this project during technical interviews.
