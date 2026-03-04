---
sidebar_position: 1
---

# Getting Started

Build and run your first R-Type game with the engine in under 10 minutes.

## Prerequisites

### Required Tools

- **C++23 Compiler**: GCC 13+, Clang 16+, or MSVC 2022+
- **CMake**: 3.20 or higher
- **Git**: For cloning the repository

### Platform-Specific

**Linux:**
```bash
# Ubuntu/Debian
sudo apt install build-essential cmake git

# Fedora
sudo dnf install gcc-c++ cmake git
```

**Windows:**
- Visual Studio 2022 with C++ Desktop Development
- CMake (via Visual Studio or standalone)
- Git for Windows

---

## Quick Start

### 1. Clone the Repository

```bash
git clone https://github.com/your-org/R-Type.git
cd R-Type
```

### 2. Build the Project

**Linux:**
```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

**Windows:**
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

**Build time:** ~2-5 minutes (downloads dependencies automatically)

### 3. Run the Game

```bash
# Start server (Terminal 1)
./bin/r-type_server

# Start client (Terminal 2)
./bin/r-type_client
```

**That's it!** You should see the R-Type menu.

---

## Project Structure

```
R-Type/
├── engine/              # Reusable game engine (modular package)
│   ├── include/         # Public API headers
│   ├── src/             # Engine implementation
│   │   ├── Core/        # ECS core (EcsManager, ComponentManager, etc.)
│   │   ├── Systems/     # Engine systems (Render, Input, Audio, etc.)
│   │   └── managers/    # Core managers (BusEventManager)
│   └── CMakeLists.txt   # Engine build configuration
│
├── client/              # R-Type client
│   └── src/
│       ├── systems/     # Client systems (MenuSystem, UISystem, etc.)
│       ├── components/  # Client components
│       └── main.cpp
│
├── server/              # R-Type server
│   └── src/
│       ├── systems/     # Server systems (MovementSystem, WeaponSystem, etc.)
│       ├── components/  # Server components
│       └── main.cpp
│
├── common/              # Shared code (network protocol, serialization)
├── assets/              # Game assets (textures, sounds, fonts)
│   ├── sprites/         # Sprite images
│   ├── sounds/          # Sound effects
│   ├── music/           # Music files
│   └── fonts/           # TrueType fonts
│
├── scripts/             # Lua scripts for AI and game logic
├── docusaurus/          # Documentation (this site!)
└── CMakeLists.txt       # Root build configuration (adds engine via CPM)
```

---

## Understanding the Engine

The R-Type engine is built on **Entity-Component-System (ECS)** architecture:

- **Entities**: Game objects (player, enemies, bullets)
- **Components**: Data (Position, Velocity, Health)
- **Systems**: Logic (Movement, Collision, Rendering)

### Example: Creating an Entity

```cpp
#include <GameEngine/EngineBuilder.hpp>

int main() {
    // Create engine with all built-in systems
    auto ecs = GameEngine::EngineBuilder::create();

    // Register components
    ecs->registerComponent<PositionComponent>();
    ecs->registerComponent<VelocityComponent>();

    // Create entity
    GameEngine::EntityId player = ecs->createEntity();

    // Add components
    ecs->addComponent(player, PositionComponent{100.0F, 200.0F, 0.0F});
    ecs->addComponent(player, VelocityComponent{50.0F, 0.0F});

    return 0;
}
```

**Key difference**: Use `EngineBuilder::create()` instead of manually creating `EcsManager`. This automatically registers all enabled engine systems.

---

## Building Your First Game

### Step 1: Create a New Project

```bash
mkdir my-game && cd my-game
```

### Step 2: Add CMake Configuration

`CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.20)
project(MyGame CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download CPM (CMake Package Manager)
file(DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.8/CPM.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

# Configure which engine systems to build
set(GAMEENGINE_BUILD_RENDER_SYSTEM ON)
set(GAMEENGINE_BUILD_INPUT_SYSTEM ON)
set(GAMEENGINE_BUILD_AUDIO ON)
set(GAMEENGINE_BUILD_SCRIPTING ON)
set(GAMEENGINE_BUILD_NETWORK OFF)  # Not needed for single-player

# Add R-Type engine as CPM package
CPMAddPackage(
    NAME GameEngine
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../R-Type/engine
)

# Your game executable
add_executable(my_game main.cpp)

# Link only the engine systems you need
target_link_libraries(my_game PRIVATE
    GameEngine::core
    GameEngine::render_system
    GameEngine::input_system
    GameEngine::audio
    GameEngine::scripting
)
```

**Key points:**
- Use **CPM** to add the engine as a modular package
- Configure which systems to build with `GAMEENGINE_BUILD_*` flags
- Link specific `GameEngine::*` targets, not a monolithic library

### Step 3: Write Your Game

`main.cpp`:
```cpp
#include <GameEngine/EngineBuilder.hpp>
#include <GameEngine/TimeManager.hpp>
#include <GameEngine/RenderEventTypes.hpp>
#include <GameEngine/InputEventTypes.hpp>
#include <iostream>

int main() {
    // 1. Create engine (automatically registers built-in systems)
    auto ecs = GameEngine::EngineBuilder::create();
    std::cout << "Engine created\n";

    // 2. Initialize window and renderer via events
    ecs->publishEvent(GameEngine::CreateWindowEvent("My Game", 800, 600));
    ecs->publishEvent(GameEngine::CreateRendererEvent());
    std::cout << "Window created\n";

    // 3. Subscribe to quit event
    bool running = true;
    ecs->subscribeEvent<GameEngine::QuitEvent>([&](const GameEngine::QuitEvent&) {
        std::cout << "Quit requested\n";
        running = false;
    });

    // 4. Main game loop with delta time calculation
    std::cout << "Starting game loop\n";
    std::uint64_t lastTime = GameEngine::TimeManager::getTimeMs();

    while (running) {
        std::uint64_t currentTime = GameEngine::TimeManager::getTimeMs();
        float deltaTime = static_cast<float>(currentTime - lastTime) / 1000.0F;
        lastTime = currentTime;

        ecs->runEngine(deltaTime);  // Updates all systems
    }

    std::cout << "Game exiting\n";
    return 0;
}
```

**Key changes:**
- Use `EngineBuilder::create()` - engine systems are auto-registered
- Initialize via **events** (`CreateWindowEvent`, `CreateRendererEvent`)
- Use `runEngine(deltaTime)` not `update(deltaTime)`
- Subscribe to events for clean shutdown

### Step 4: Build and Run

```bash
mkdir build && cd build
cmake ..
cmake --build .
./my_game
```

---

## Next Steps

### Learn the Systems

1. **[Engine Architecture](../engine/architecture.md)** - Understand ECS
2. **[Rendering System](../engine/rendering-system.md)** - Display sprites
3. **[Input System](../engine/input-system.md)** - Handle input
4. **[Scripting System](../engine/scripting-system.md)** - Add Lua logic

### Explore R-Type Code

- `client/src/systems/MenuSystem/` - Complete UI example
- `server/src/systems/WeaponSystem/` - Game logic example

### Customize R-Type

- Modify `assets/levels/*.json` - Create new levels
- Edit `scripts/*.lua` - Change AI behavior
- Add components in `server/src/components/` - New features

---

## Common Issues

### Build Fails

**"SDL3 not found"**
```bash
# CPM should download automatically, but if it fails:
git submodule update --init --recursive
```

**"Boost not found"**
```bash
# Install Boost
sudo apt install libboost-all-dev  # Linux
# Or let CPM download it (takes longer)
```

### Game Won't Start

**"Failed to load assets"**
- Run from project root, not `/bin/`
- Ensure `assets/` folder exists

**"Address already in use" (server)**
- Port 57208 is taken
- Change port in `server/src/main.cpp`

---

## Controls

### In-Game

- **WASD** / **Arrow Keys**: Move
- **Space**: Shoot
- **Escape**: Pause

### Menu

- **Mouse**: Click buttons
- **Enter**: Confirm
- **Escape**: Back

---

## Getting Help

- **Documentation**: `docusaurus/` folder
- **Issues**: GitHub Issues

---

## What's Next?

Now that you have the game running:

1.  Read [Project Overview](../project-overview.md)
2.  Explore [Engine Architecture](../engine/architecture.md)
3.  Try modifying a Lua script in `scripts/`
4.  Create a new enemy prefab in `assets/prefabs/`
5.  Build your own game with the engine!

---

**Last Updated:** November 2025
