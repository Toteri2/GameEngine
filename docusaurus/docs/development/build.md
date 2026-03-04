---
sidebar_position: 1
---

# Build Instructions

This guide covers how to build the R-Type project on Linux and Windows.
For Linux, we will assume that the examples are shown for an ubuntu system.

## Prerequisites

### All Platforms

- **CMake** 3.16 or higher
- **C++ Compiler** with C++23 support:
  - Linux: GCC 11+ or Clang 14+
  - Windows: MSVC 19.30+ (Visual Studio 2022)
- **Git**
- **Package Manager**:
  - CMake CPM (bundled)
- **Linter**
  - Clang-tidy
  - Clang-format

### Linux-Specific

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake git

# Fedora
sudo dnf install gcc-c++ cmake git

# Arch
sudo pacman -S base-devel cmake git
```

### Windows-Specific

- Install [Visual Studio 2022](https://visualstudio.microsoft.com/) with C++ workload
- Install [CMake](https://cmake.org/download/)
- Install [Git for Windows](https://git-scm.com/download/win)

## Cloning the Repository

```bash
git clone https://github.com/Mambokara/R-Type.git
cd R-Type
git submodule update --init --recursive
```

## Building on Linux

### Using CMake

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)
```

### Using Make

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Building on Windows

### Using Visual Studio

```bash
# Create build directory
mkdir build && cd build

# If your are not in powershell dev terminal
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

# Generate Visual Studio solution
cmake -S .. -B . -G "Visual Studio 17 2022" `
      -DCMAKE_BUILD_TYPE=Release `
      -DAOM_TARGET_CPU=generic `
      -DSDLIMAGE_AVIF=OFF `
      -DDAV1D_ASM=OFF

# Build
cmake --build . --config Release
```

### Using Ninja (Faster)

```bash
# From Visual Studio Developer Command Prompt
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
      -DCMAKE_BUILD_TYPE=Release `
      -DAOM_TARGET_CPU=generic `
      -DSDLIMAGE_AVIF=OFF `
      -DDAV1D_ASM=OFF
ninja
```

## Build Outputs

After building, you'll find:

```
bin/ (Release/)
├── r-type_server       # Game server executable
└── r-type_client       # Game client executable
```

## Running the Game

### Start the Server

```bash
# Linux
./bin/r-type_server

# Windows
.\bin\Release\r-type_server.exe
```

### Start the Client

```bash
# Linux
./bin/r-type_client

# Windows
.\bin\Release\r-type_client.exe
```

## Build Options

### CMake Options

```bash
# Enable/disable components
cmake .. -DBUILD_TESTS=ON
cmake .. -DBUILD_DOCS=OFF

# Set install prefix
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local

# Use specific compiler
cmake .. -DCMAKE_CXX_COMPILER=clang++
```

### Package Manager Selection

The project uses CMake CPM by default, but you can use others:

```bash
# Use Conan
cmake .. -DUSE_CONAN=ON

# Use Vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg.cmake
```

## Dependencies

The project automatically fetches and builds:

- **SDL3**: Graphics, audio, and input
- **Asio** (optional): Networking library

All dependencies are managed automatically - no manual installation required!

## Troubleshooting

### CMake can't find SDL3

If SDL3 isn't found:

```bash
# Clear CMake cache and reconfigure
rm -rf build/
mkdir build && cd build
cmake ..
```

### Compiler Errors

Ensure you're using a C++23-compatible compiler:

```bash
# Check GCC version (need 11+)
g++ --version

# Check Clang version (need 14+)
clang++ --version

# Check MSVC version (need 19.30+)
cl /?
```

### Linker Errors on Linux

If you see undefined reference errors:

```bash
# Install development libraries
sudo apt install libsdl3-dev  # If using system SDL

# Or clean build directory
rm -rf build && mkdir build && cd build && cmake ..
```

### Windows: MSVC Not Found

Make sure to run CMake from the Visual Studio Developer Command Prompt, or add MSVC to your PATH.

## Clean Build

To start fresh:

```bash
# Remove build artifacts
rm -rf build/
```

## Cross-Compilation

### Building Windows Executable on Linux

Using MinGW:

```bash
sudo apt install mingw-w64

mkdir build-win && cd build-win
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64.cmake
make
```

## IDE Setup

### Visual Studio Code

Install extensions:
- C/C++ (Microsoft)
- CMake Tools (Microsoft)
- CMake (twxs)

Open the folder and CMake Tools will auto-configure.

### CLion

CLion has native CMake support. Simply open the project folder.

### Visual Studio

Generate solution files:
```bash
cmake -G "Visual Studio 17 2022" -A x64 ..
```

Open `R-Type.sln`.

## Development Build

For faster iteration during development:

```bash
# Use Ninja for faster builds
cmake .. -G Ninja

# Enable compile_commands.json for IDEs
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Parallel build
cmake --build . -j$(nproc)
```

## Formatting

In order to check if your code is correctly formatted, you can use a little tool.

### Install on Linux

```bash
sudo apt install clang-tidy clang-format
```

### Install on Windows

```powershell
$installerUrl = "https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.7/LLVM-18.1.7-win64.exe"
$installerPath = "$env:TEMP\LLVM-18.1.7-win64.exe"

Write-Host "Downloading LLVM installer..."
Invoke-WebRequest -Uri $installerUrl -OutFile $installerPath

Write-Host "Running installer..."
Start-Process -FilePath $installerPath -ArgumentList "/S" -Wait

Write-Host "LLVM installation complete."
```

And next, you can run this command to check your formatting:
```bash
cmake --build build --target clang_tidy_check
```

## Testing

```bash
# Build with tests
cmake .. -DBUILD_TESTS=ON
cmake --build .

# Run tests
ctest --output-on-failure

# Or run directly
./build/tests/rtype_tests
```

## Next Steps

- [Engine Architecture](../engine/architecture.md) - Understanding the ECS architecture
- [Network Protocol](../network/protocol.md) - Network communication protocol
- [Project Overview](../project-overview.md) - High-level project information
