# Project Overview

## Introduction

R-Type is a network multiplayer game project that implements a **horizontally scrolling shooter** (Shoot'em'up) based on the legendary arcade game. The project emphasizes creating a robust, reusable game engine architecture while delivering a fully functional networked game.

## Project Features

### Core Gameplay
- **Playable Game**: A functional R-Type prototype with starfield background, player ships, enemy waves (Bydos), and projectile combat
- **Networked Gameplay**: Client-server architecture where multiple clients connect to a central authoritative server
- **Boss Fights**: Bydo boss with custom AI
- **4 Enemy Types**: Basic, Sine Wave, Tank, Shooter with unique behaviors

### Reusable Game Engine
- **8 Modular Engine Systems**: Rendering, Input, Audio, Network, Scripting, Level, Window, Logger
- **SDL3 Abstraction**: No client dependencies on SDL - fully engine-encapsulated
- **CMake Build Options**: Each system can be enabled/disabled independently
- **True Reusability**: Engine can be used for any 2D game, not just R-Type

### Engine Systems
- **[Rendering System](engine/rendering-system.md)**: Sprite, text, shape rendering with layering
- **[Input System](engine/input-system.md)**: Multi-device support, runtime key rebinding, context-based input
- **[Scripting System](engine/scripting-system.md)**: Lua 5.4.7 integration with hot reload
- **[Level System](engine/level-system.md)**: JSON-driven levels, prefab system, wave management
- **[Audio System](engine/audio-system.md)**: Sound pooling, music playback via miniaudio
- **[Network Architecture](network/network-architecture.md)**: Entity synchronization (60-120 Hz), client-server model

### Content Creation
- **JSON Level Editor**: Define waves, enemies, bosses via configuration files
- **Lua Scripting**: Custom AI behaviors with hot reload support
- **UI/Menu System**: Settings menu with runtime key rebinding
- **Animation System**: Sprite animations for players and enemies
- **Prefab System**: Reusable entity templates

## Technical Requirements

### Build System & Dependencies

- **Build System**: CMake
- **Package Manager**: CMake CPM
- **Platform Support**: Linux, Windows with MSVC
- **Development Workflow**: Feature branches, merge requests, CI/CD integration

### Architecture

The project demonstrates a layered architecture:

```
┌─────────────────────────────────┐
│     Game Logic (R-Type)         │
├─────────────────────────────────┤
│      Game Engine (ECS)          │
├──────────┬──────────┬───────────┤
│ Rendering│ Physics  │ Network   │
├──────────┴──────────┴───────────┤
│    Platform Layer (SDL3)        │
└─────────────────────────────────┘
```

## Server Architecture

The **game server** implements the authoritative game logic:

- Multi-threaded to handle multiple clients without blocking
- Processes client inputs and updates game state
- Broadcasts game updates to all connected clients
- Robust error handling

## Client Architecture

The **game client** handles presentation and input:

- Graphical rendering using engine render system
- Input handling (keyboard, mouse)
- Network communication with the server

## Network Protocol

### Protocol Requirements

✓ **Binary Protocol**: All data transmitted in binary format, not text
✓ **UDP-based**: Primary communication uses UDP for real-time gameplay
✓ **Documented**: Full protocol specification with packet structures

## Game Engine Design

The engine uses an **Entity-Component-System (ECS)** architecture pattern:

- **Entities**: Unique IDs representing game objects
- **Components**: Data-only structures (Position, Velocity, Sprite, Health, etc.)
- **Systems**: Logic processors that operate on entities with specific component combinations

### Key Benefits of ECS

- **Decoupling**: Systems are independent and don't know about each other
- **Performance**: Cache-friendly data layout, efficient iteration
- **Flexibility**: Easy to add new components and systems
- **Reusability**: Game-agnostic engine code

## Documentation Requirements

The project includes comprehensive documentation:

1. **README**: Project overview, build instructions, quick start
2. **Developer Documentation**: Architecture diagrams, system descriptions, tutorials
3. **Protocol Documentation**: RFC-style network protocol specification
4. **Technical Study**: Comparative analysis of technologies, algorithms, and design choices

## Accessibility Considerations

The project considers accessibility for:

- Physical and Motor Disabilities
- Auditory and Visual Disabilities
- Mental and Cognitive Disabilities

Solutions may include configurable controls, visual/audio alternatives, adjustable difficulty, etc.

## Engine Architecture

The engine is a **professional, modular game development framework**:

```
┌───────────────────────────────────────────────────────┐
│                    R-Type Game                        │
├───────────────────────────────────────────────────────┤
│  Client Systems  │  Server Systems  │  Common Protocol│
│  - MenuSystem    │  - EnemyAISystem │  - Serialization│
│  - UISystem      │  - WeaponSystem  │  - PacketTypes  │
│  - Animation     │  - Collision     │                 │
└───────────────────┬───────────────────────────────────┘
                    │ Uses
        ┌───────────▼───────────────────────────┐
        │       Reusable Game Engine            │
        ├───────────────────────────────────────┤
        │  ┌─────────────┐  ┌─────────────┐     │
        │  │RenderSystem │  │ InputSystem │     │
        │  └─────────────┘  └─────────────┘     │
        │  ┌─────────────┐  ┌─────────────┐     │
        │  │ScriptSystem │  │ AudioSystem │     │
        │  └─────────────┘  └─────────────┘     │
        │  ┌─────────────┐  ┌─────────────┐     │
        │  │LevelSystem  │  │NetworkSystem│     │
        │  └─────────────┘  └─────────────┘     │
        │  ┌─────────────┐  ┌─────────────┐     │
        │  │WindowSystem │  │LoggerSystem │     │
        │  └─────────────┘  └─────────────┘     │
        ├───────────────────────────────────────┤
        │         ECS Core (EntityManager,      │
        │    ComponentManager, SystemManager)   │
        └───────────────────────────────────────┘
                    │ Uses
        ┌───────────▼───────────────────────────┐
        │       Platform Layer                  │
        │  SDL3, Boost.Asio, miniaudio, Lua     │
        └───────────────────────────────────────┘
```

### System Overview

| System | Purpose |
|--------|---------|
| **RenderSystem** | Sprites, text, shapes |
| **InputSystem** | Keyboard, mouse, gamepad |
| **AudioSystem** | Sound effects, music |
| **NetworkSystem** | Multiplayer, UDP |
| **ScriptingSystem** | Lua runtime logic |
| **LevelSystem** | JSON levels, prefabs |
| **WindowSystem** | Window management |
| **LoggerSystem** | ECS logging |

**All systems are:**
-  Modular (can be disabled via CMake)
-  Event-driven
-  Cross-platform
-  Documented

## Technologies Used

- **Language**: C++23
- **Graphics**: SDL3, SDL3_image, SDL3_ttf, SDL3_gfx
- **Audio**: miniaudio
- **Networking**: Boost.Asio (UDP)
- **Scripting**: Lua 5.4.7 + LuaBridge3
- **JSON**: nlohmann_json
- **Build**: CMake 3.20+
- **Package Management**: CPM (CMake Package Manager)
- **Documentation**: Docusaurus (this site!)
- **CI/CD**: GitHub Actions
