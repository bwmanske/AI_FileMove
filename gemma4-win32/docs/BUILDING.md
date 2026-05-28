# Building FileMove

This document provides instructions for building the FileMove project on both Linux and Windows environments.

## Prerequisites

### Common
- **CMake** (version 3.14 or higher)
- **Git**
- A C++17 compliant compiler

### Linux
- **GCC** or **Clang**
- Build tools (e.g., `make`, `build-essential`)

### Windows
- **Visual Studio** (with "Desktop development with C++" workload installed)
- **PowerShell**

---

## Building on Linux

1. Navigate to the `gemma4-win32` directory:
   ```bash
   cd gemma4-win32
   ```

2. Configure the project using CMake:
   ```bash
   cmake -S . -B build
   ```

3. Build the project:
   ```bash
   cmake --build build
   ```

### Clean Build (Linux)
To perform a completely clean build, remove the `build` directory and start over:
```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

The executable will be located in `build/FileMove`.

---

## Building on Windows

### Method 1: Manual Command Line (PowerShell)

1. Open PowerShell and navigate to the `gemma4-win32` directory.
2. Run the provided build script:
   ```powershell
   .\build_windows.ps1
   ```
   *(If you encounter execution policy restrictions, run `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope Process` first.)*

### Clean Build (Windows)
To perform a completely clean build in PowerShell, remove the `build` directory and start over:
```powershell
Remove-Item -Recurse -Force build
.\build_windows.ps1
```

The executable will be located in `build/Release/FileMove.exe`.

### Method 2: Using Visual Studio IDE

1. Open Visual Studio.
2. Select **Open a local folder**.
3. Choose the `gemma4-win32` directory.
4. CMake will automatically configure the project.
5. Once configuration is complete, select the `FileMove` target and click **Build > Build FileMove**.

---

## Running Tests

After building the project, you can run the unit tests using CMake:

### Linux
```bash
cd gemma4-win32/build
./FileMoveTests
```

### Windows (PowerShell)
```powershell
cd gemma4-win32/build
.\FileMoveTests.exe
```
