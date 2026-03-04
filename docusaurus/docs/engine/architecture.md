---
sidebar_position: 1
---

# Engine Architecture

The R-Type game engine is built around the **Entity-Component-System (ECS)** architectural pattern, providing a flexible, performant, and decoupled foundation for game development.

## What is ECS?

Entity-Component-System is a design pattern commonly used in game engines that favors composition over inheritance:

- **Entities**: Unique identifiers (IDs) representing game objects
- **Components**: Pure data structures with no logic (Position, Velocity, Sprite, Health, etc.)
- **Systems**: Logic processors that operate on entities possessing specific component combinations

> **New to the engine?** See [Practical Integration Guide](#practical-integration-guide) for a complete step-by-step tutorial.

## Architecture Overview

The engine is split into two layers:

1. **Engine Core** (`engine/src/`) - Generic, game-agnostic ECS framework
2. **Game Layer** (`client/src/`, `server/src/`) - Game-specific systems and components

The engine uses three key architectural patterns:
- **[Event-Driven Architecture](#event-driven-architecture)** - Systems communicate via events instead of direct calls
- **[CMake Modular Architecture](#cmake-modular-architecture)** - Engine is a reusable external package
- **[Builder Pattern](#builder-pattern--system-registration)** - Easy initialization with EngineBuilder

### Visual Architecture Diagram

```
┌────────────────────────────────────────────────────────────┐
│                       EcsManager                           │
│  ┌──────────────────────────────────────────────────────┐  │
│  │            BusEventManager (Event Hub)               │  │
│  │  - publishEvent<T>()                                 │  │
│  │  - subscribeEvent<T>()                               │  │
│  │  - Priority-based dispatch                           │  │
│  └────────────┬─────────────────────────┬───────────────┘  │
│               │                         │                  │
│  ┌────────────▼──────────┐  ┌───────────▼────────────┐     │
│  │  ComponentManager     │  │   SystemManager        │     │
│  │  - Stores components  │  │   - Runs systems       │     │
│  └───────────────────────┘  └────────────────────────┘     │
│               │                         │                  │
│  ┌────────────▼─────────────────────────▼───────────────┐  │
│  │              EntityManager                           │  │
│  │              - Creates entities                      │  │
│  └──────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────┘
         │                   │                 │
         ▼                   ▼                 ▼
  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
  │ RenderSys   │    │  InputSys   │    │ NetworkSys  │
  │             │    │             │    │             │
  │ Subscribes: │    │ Publishes:  │    │ Subscribes: │
  │ - LoadTex   │    │ - KeyDown   │    │ - SendPkt   │
  │ - CreateWin │    │ - MouseMove │    │ Publishes:  │
  │             │    │             │    │ - RecvPkt   │
  └─────────────┘    └─────────────┘    └─────────────┘
```

**Key Principles**:
- The engine core (`engine/src/`) knows **nothing** about R-Type, SDL, networking, or game-specific logic
- Systems communicate via **events**, never direct calls
- The **BusEventManager** is the central communication hub

## Core Components

### EcsManager

The `EcsManager` class (located at `engine/src/Core/EcsManager.hpp`) acts as the main facade for the entire ECS architecture. It coordinates four core managers:

```cpp
class EcsManager {
private:
    std::unique_ptr<ComponentManager> _componentManager;
    std::unique_ptr<EntityManager> _entityManager;
    std::unique_ptr<SystemManager> _systemManager;
    std::unique_ptr<BusEventManager> _busEventManager;

public:
    // Entity operations
    EntityId createEntity();
    void destroyEntity(EntityId);

    // Component operations
    template<typename T> void registerComponent();
    template<typename T> void addComponent(EntityId, T);
    template<typename T> void removeComponent(EntityId);
    template<typename T> T& getComponent(EntityId);
    template<typename T> bool hasComponent(EntityId);

    // System operations
    template<typename T, typename... Args>
    T* registerSystem(Args&&...);
    void runEngine(float deltaTime);

    // Query operations
    template<typename... ComponentTypes>
    std::vector<EntityId> getAllEntitiesWith();

    // Event operations
    template<typename EventType>
    void publishEvent(const EventType& event);

    template<typename EventType>
    void publishEventDeferred(const EventType& event);

    template<typename EventType>
    HandlerId subscribeEvent(std::function<void(const EventType&)> callback,
                             const HandlerOptions& options = {});
};
```

**Key Point**: Systems communicate via **events** (publishEvent/subscribeEvent).

### Entity Manager

The `EntityManager` (`engine/src/managers/EntityManager/EntityManager.hpp`) handles entity lifecycle:

- Creates unique entity IDs
- Tracks active entities
- Manages entity destruction

**Key Implementation Details:**
- Entity IDs are simple `uint32_t` values
- Uses a queue-based system for efficient entity management
- `INVALID_ENTITY` (ID = 0) represents an invalid/null entity

### Component Manager

The `ComponentManager` (`engine/src/managers/ComponentManager/ComponentManager.hpp`) handles all component storage and retrieval:

- Registers component types at runtime
- Stores components in type-indexed storage arrays
- Provides fast component access via entity ID

**Implementation Highlights:**
- Uses `std::type_index` for runtime type identification
- Each component type gets a unique storage container (`ComponentStorage<T>`)
- Components are stored in a contiguous array for cache-friendly access

### System Manager

The `SystemManager` (`engine/src/managers/SystemManager/SystemManager.hpp`) orchestrates all game systems:

- Registers systems with dependency injection support
- Executes systems each frame in registration order
- Provides the update loop backbone

**System Interface:**
All systems must implement the `ISystem` interface:

```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(EcsManager& world, float deltaTime) = 0;
};
```

### Bus Event Manager

The `BusEventManager` (`engine/src/managers/BusEventManager/BusEventManager.hpp`) provides a **event-driven communication system** that enables decoupled system interactions:

- Publish/subscribe pattern for type-safe events
- Priority-based handler execution
- Event categories and filtering
- Deferred and delayed event processing
- Event history and replay capabilities

This is a **critical component** that enables systems to communicate without direct dependencies. See [Event-Driven Architecture](#event-driven-architecture) section below for detailed examples.

---

## Event-Driven Architecture

The R-Type engine uses an **event bus** for inter-system communication, allowing systems to remain decoupled while still coordinating complex behaviors.

### Why Events?

Instead of systems calling each other directly (tight coupling):
```cpp
class InputSystem {
    NetworkSystem* networkSystem;  // Direct dependency

    void processInput() {
        networkSystem->sendPacket(data);  // Direct call
    }
};
```

The engine uses events (loose coupling):
```cpp
class InputSystem {
    void processInput(EcsManager& world) {
        SendPunctualPacketEvent event(packetId, data);
        world.publishEvent(event);  // Any system can handle this
    }
};
```

### BusEventManager API

**Publishing Events:**
```cpp
// Immediate event - processed right away
world.publishEvent(KeyDownEvent(SDL_SCANCODE_SPACE));

// Deferred event - processed at end of frame
world.publishEventDeferred(SpawnEnemyEvent(enemyType));

// Delayed event - processed after delay
world.publishEventDelayed(BossSpawnEvent(), 3.0f);  // 3 seconds
```

**Subscribing to Events:**
```cpp
// Basic subscription
world.subscribeEvent<KeyDownEvent>([](const KeyDownEvent& event) {
    std::cout << "Key pressed: " << event.scancode << "\n";
});

// Subscription with priority (higher = executes first)
HandlerOptions options;
options.priority = 100;  // Default is 0
world.subscribeEvent<InputActionEvent>(handler, options);

// Subscription with filter
auto filter = [](const DamageEvent& e) { return e.damage > 50; };
world.subscribeEvent<DamageEvent>(handler, {}, filter);

// Subscription with category (can enable/disable whole groups)
HandlerOptions opts;
opts.category = "ui";
world.subscribeEvent<QuitEvent>(handler, opts);
world.setCategoryEnabled("ui", false);  // Disables all "ui" handlers
```

### Real-World Event Examples

**Rendering Events** (`engine/include/GameEngine/RenderEventTypes.hpp`):
```cpp
// Client publishes to load a texture
LoadTextureEvent loadEvent("player_ship", "assets/player.png");
world.publishEvent(loadEvent);

// RenderSystem subscribes and loads the texture
world.subscribeEvent<LoadTextureEvent>([this](const LoadTextureEvent& e) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, e.path.c_str());
    textureCache[e.textureId] = texture;
});
```

**Network Events** (`engine/include/GameEngine/EventTypes.hpp`):
```cpp
// Start network client
StartNetworkEvent startEvent(57208, "127.0.0.1");
world.publishEvent(startEvent);

// NetworkSystem subscribes and creates socket
world.subscribeEvent<StartNetworkEvent>([this](const StartNetworkEvent& e) {
    networkManager.connect(e.serverAddress, e.port);
});

// Send packet to server
std::vector<uint8_t> data = {0x01, 0x02, 0x03};
SendPunctualPacketEvent sendEvent(1, data);  // packetId=1
world.publishEvent(sendEvent);
```

**Input Events** (`engine/include/GameEngine/InputEventTypes.hpp`):
```cpp
// InputSystem publishes raw input
world.publishEvent(KeyDownEvent(SDL_SCANCODE_W));
world.publishEvent(MouseButtonDownEvent(SDL_BUTTON_LEFT, x, y));

// Game systems subscribe to high-level actions
world.subscribeEvent<InputActionEvent>([](const InputActionEvent& e) {
    if (e.actionName == "Jump" && e.pressed) {
        // Player jumped!
    }
});
```

### Event Categories and Priority

**Use Case**: UI systems should process input before gameplay systems.

```cpp
// UI system subscribes with high priority
HandlerOptions uiOptions;
uiOptions.priority = 100;
uiOptions.category = "ui";

world.subscribeEvent<MouseButtonDownEvent>([](const MouseButtonDownEvent& e) {
    if (buttonClicked(e.x, e.y)) {
        return;  // Consume event, don't propagate
    }
}, uiOptions);

// Gameplay system subscribes with default priority (0)
world.subscribeEvent<MouseButtonDownEvent>([](const MouseButtonDownEvent& e) {
    fireWeapon(e.x, e.y);
});
```

**Priority execution order**: Priority 100 runs before priority 0.

### Event History and Replay

The BusEventManager can record and replay events:

```cpp
// Enable event history
world.enableEventHistory<DamageEvent>();

// Events are automatically recorded
world.publishEvent(DamageEvent(entity, 50));
world.publishEvent(DamageEvent(entity, 30));

// Retrieve recent events (specify count)
auto history = world.getRecentEvents<DamageEvent>(10);  // Get last 10 events
for (const auto& event : history) {
    std::cout << "Damage: " << event.damage << "\n";
}
```

**Use cases**: Replay systems, debugging, networked game state replay.

### Benefits of Event-Driven Architecture

- **Decoupling**: Systems don't know about each other
- **Flexibility**: Add new systems without modifying existing ones
- **Testability**: Systems can be tested in isolation
- **Extensibility**: Subscribe to events from anywhere
- **Debugging**: Event history helps trace bugs
- **Performance**: Deferred events batch processing

---

## CMake Modular Architecture

The R-Type engine is structured as a **separate, reusable library** that game projects depend on. This enables true modularity and reusability.

### Engine as External Package

The root `CMakeLists.txt` treats the engine as an external CPM package:

```cmake
# Root CMakeLists.txt
include(cmake/CPM.cmake)

# Configure which engine systems to build
set(GAMEENGINE_BUILD_RENDER_SYSTEM ON)
set(GAMEENGINE_BUILD_INPUT_SYSTEM ON)
set(GAMEENGINE_BUILD_AUDIO ON)
set(GAMEENGINE_BUILD_NETWORK ON)
set(GAMEENGINE_BUILD_SCRIPTING ON)
# ... etc

# Add engine as external package
CPMAddPackage(
  NAME GameEngine
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine
)
```

This creates **modular targets** in the `GameEngine::` namespace:
- `GameEngine::core` - ECS foundation (header-only)
- `GameEngine::render_system` - Rendering (SDL3)
- `GameEngine::input_system` - Input handling
- `GameEngine::audio` - Sound and music (miniaudio)
- `GameEngine::network` - Networking (Boost.Asio)
- `GameEngine::scripting` - Lua runtime
- `GameEngine::level_system` - JSON level loading
- `GameEngine::time` - Time utilities
- `GameEngine::logger` - Logging system

### Conditional Compilation

Each subsystem can be **enabled or disabled** via CMake flags:

```cmake
# engine/CMakeLists.txt
if(GAMEENGINE_BUILD_RENDER_SYSTEM)
    # Only build RenderSystem if enabled
    add_library(game_engine_render_system STATIC
        src/Systems/RenderSystem/RenderSystem.cpp
        src/Systems/RenderSystem/RenderManager.cpp
        # ...
    )

    # Link SDL3 dependencies
    target_link_libraries(game_engine_render_system PUBLIC
        GameEngine::core
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        SDL3_gfx::SDL3_gfx
    )

    # Define compile flag for conditional includes
    target_compile_definitions(game_engine_render_system PUBLIC
        GAMEENGINE_HAS_RENDER_SYSTEM
    )

    # Export as GameEngine::render_system
    add_library(GameEngine::render_system ALIAS game_engine_render_system)
endif()
```

**Benefits**:
- Reduce compile times (only build what you need)
- Minimize dependencies (e.g., no SDL if rendering is disabled)
- Enable different configurations per game

### Linking the Engine in Your Game

**Client example**:
```cmake
# client/CMakeLists.txt
add_executable(r-type_client
    src/main.cpp
    src/systems/MenuSystem.cpp
    # ...
)

# Link only the engine systems you need
target_link_libraries(r-type_client PRIVATE
    GameEngine::core
    GameEngine::render_system
    GameEngine::input_system
    GameEngine::audio
    GameEngine::network
    GameEngine::scripting
    GameEngine::level_system
)
```

**Server example** (no rendering):
```cmake
# server/CMakeLists.txt
add_executable(r-type_server
    src/main.cpp
    src/systems/MovementSystem.cpp
    # ...
)

# Server doesn't need rendering, input, or audio!
target_link_libraries(r-type_server PRIVATE
    GameEngine::core
    GameEngine::network
    GameEngine::scripting
    GameEngine::level_system
)
```

### Using the Engine in Your Own Game

To use the R-Type engine for your own game project:

**1. Add the engine as a subdirectory:**
```cmake
# your-game/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(MyGame)

# Configure which systems you need
set(GAMEENGINE_BUILD_RENDER_SYSTEM ON)
set(GAMEENGINE_BUILD_INPUT_SYSTEM ON)
set(GAMEENGINE_BUILD_AUDIO ON)
set(GAMEENGINE_BUILD_NETWORK OFF)  # Single-player game
set(GAMEENGINE_BUILD_SCRIPTING ON)

# Add R-Type engine as CPM package
CPMAddPackage(
        NAME GameEngine
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../R-Type/engine
)
# Your game executable
add_executable(my_game src/main.cpp)

# Link engine systems
target_link_libraries(my_game PRIVATE
    GameEngine::core
    GameEngine::render_system
    GameEngine::input_system
    GameEngine::audio
    GameEngine::scripting
)
```

**2. Use the engine in your code:**
```cpp
#include <GameEngine/EngineBuilder.hpp>

int main() {
    auto ecs = GameEngine::EngineBuilder::create();
    // Engine is ready to use!
}
```

## Builder Pattern & System Registration

The engine provides a **builder pattern** for easy initialization and system registration.

### EngineBuilder

Located at `engine/include/GameEngine/EngineBuilder.hpp`, the `EngineBuilder` provides a static factory method that creates a fully-initialized `EcsManager` with all enabled engine systems:

```cpp
class EngineBuilder {
public:
    static auto create() -> std::unique_ptr<EcsManager> {
        auto ecs = std::make_unique<EcsManager>();

        // Conditionally register systems based on compile flags
#ifdef GAMEENGINE_HAS_LOGGER_SYSTEM
        ecs->registerSystem<Logger::LoggerSystem>();
#endif

#ifdef GAMEENGINE_HAS_SCRIPTING_SYSTEM
        ecs->registerSystem<Scripting::ScriptingSystem>(*ecs);
#endif

#ifdef GAMEENGINE_HAS_NETWORK_SYSTEM
        ecs->registerSystem<NetworkSystem>();
#endif

#ifdef GAMEENGINE_HAS_INPUT_SYSTEM
        ecs->registerSystem<InputSystem>(*ecs);
#endif

#ifdef GAMEENGINE_HAS_RENDER_SYSTEM
        ecs->registerSystem<RenderSystem>(*ecs);
#endif

#ifdef GAMEENGINE_HAS_AUDIO_SYSTEM
        ecs->registerSystem<Audio::AudioSystem>();
#endif

        return ecs;
    }
};
```

**Key Features**:
- Systems are **automatically registered** based on CMake configuration
- Systems receive constructor dependencies via perfect forwarding
- Compile-time guards ensure only built systems are registered
- Returns ready-to-use `EcsManager`

### System Registration Lifecycle

**1. Create EcsManager**:
```cpp
auto ecs = GameEngine::EngineBuilder::create();
// Engine systems (Render, Input, etc.) are now registered
```

**2. Register custom components**:
```cpp
ecs->registerComponent<PlayerComponent>();
ecs->registerComponent<BulletComponent>();
ecs->registerComponent<EnemyComponent>();
```

**3. Register custom game systems**:
```cpp
// Systems can have dependencies injected
ecs->registerSystem<MenuSystem>(*ecs);      // Needs EcsManager reference
ecs->registerSystem<WeaponSystem>();        // No dependencies
ecs->registerSystem<CollisionSystem>(*ecs); // Needs EcsManager reference
```

**4. Initialize via events**:
```cpp
// Engine systems respond to initialization events
ecs->publishEvent(GameEngine::CreateWindowEvent("R-Type", 1280, 720));
ecs->publishEvent(GameEngine::CreateRendererEvent());
ecs->publishEvent(GameEngine::StartNetworkEvent(57208, "127.0.0.1"));
```

**5. Main game loop**:
```cpp
GameEngine::TimeManager time(60.0f);  // 60 FPS
bool running = true;

while (running) {
    time.update([&](float deltaTime) {
        ecs->runEngine(deltaTime);  // Updates all systems
    });
}
```

### System Registration Internals

The `SystemManager::registerSystem<T>()` method uses **perfect forwarding** and **type indexing**:

```cpp
template<typename T, typename... Args>
auto SystemManager::registerSystem(Args&&... args) -> T* {
    // Use RTTI for unique system identification
    const std::type_index typeIndex(typeid(T));

    // Check if system already registered
    if (_systems.contains(typeIndex)) {
        throw std::runtime_error("System already registered");
    }

    // Create system with forwarded constructor arguments
    auto system = std::make_unique<T>(std::forward<Args>(args)...);
    T* systemPtr = system.get();

    // Store in map (maintains insertion order in C++17+)
    _systems[typeIndex] = std::move(system);

    return systemPtr;
}
```

**Benefits**:
- Systems registered by **type**, not name (type-safe)
- Constructor arguments are **forwarded** (no copies)
- Returns pointer for immediate use
- Throws if duplicate registration

### System Execution Order

Systems execute in **registration order**:

```cpp
// Execution order matches registration order
ecs->registerSystem<InputSystem>(*ecs);      // Runs 1st
ecs->registerSystem<MovementSystem>();       // Runs 2nd
ecs->registerSystem<CollisionSystem>(*ecs);  // Runs 3rd
ecs->registerSystem<RenderSystem>(*ecs);     // Runs 4th (last)
```

**Important**: Register systems in logical order. For example:
1. Input systems (gather input)
2. AI systems (decision making)
3. Physics systems (movement, collision)
4. Rendering systems (display results)
5. Cleanup systems (delete entities)

### Custom System Example

Creating and registering a custom system:

```cpp
// 1. Define your system
class ScoreSystem : public ISystem {
private:
    int totalScore = 0;

public:
    void update(EcsManager& world, float deltaTime) override {
        // Query entities with score component
        auto entities = world.getAllEntitiesWith<EnemyComponent, MarkedForDeletionComponent>();

        for (EntityId entity : entities) {
            auto& enemy = world.getComponent<EnemyComponent>(entity);
            totalScore += enemy.scoreValue;

            std::cout << "Enemy killed! Score: " << totalScore << "\n";
        }
    }
};

// 2. Register in main
auto ecs = GameEngine::EngineBuilder::create();
ecs->registerComponent<EnemyComponent>();
ecs->registerComponent<MarkedForDeletionComponent>();
ecs->registerSystem<ScoreSystem>();  // Registered!

// 3. System automatically runs each frame
while (running) {
    ecs->runEngine(deltaTime);  // ScoreSystem::update() called
}
```

---

## System Communication Patterns

Let's explore real-world examples of how systems communicate through the event bus.

### Pattern 1: RenderSystem Event Subscriptions

The `RenderSystem` subscribes to various events for texture loading, window creation, etc.

**Location**: `engine/src/Systems/RenderSystem/RenderSystem.cpp`

```cpp
RenderSystem::RenderSystem(EcsManager& ecsManager) : _ecsManager(ecsManager) {
    // Subscribe to window creation
    ecsManager.subscribeEvent<CreateWindowEvent>(
        [this](const CreateWindowEvent& event) {
            onCreateWindow(event);
        });

    // Subscribe to renderer creation
    ecsManager.subscribeEvent<CreateRendererEvent>(
        [this](const CreateRendererEvent& event) {
            onCreateRenderer(event);
        });

    // Subscribe to texture loading
    ecsManager.subscribeEvent<LoadTextureEvent>(
        [this](const LoadTextureEvent& event) {
            onLoadTexture(event);
        });

    // Subscribe to font loading
    ecsManager.subscribeEvent<LoadFontEvent>(
        [this](const LoadFontEvent& event) {
            onLoadFont(event);
        });
}
```

**Event Handlers**:
```cpp
void RenderSystem::onCreateWindow(const CreateWindowEvent& event) const {
    // Create SDL window
    void* window = SDL3Wrapper::createWindow(
        event.title, event.width, event.height, event.flags);

    // Store window in ECS
    EntityId windowEntity = _ecsManager.createEntity();
    WindowComponent windowComp;
    windowComp.window = window;
    windowComp.width = event.width;
    windowComp.height = event.height;
    _ecsManager.addComponent(windowEntity, windowComp);
}

void RenderSystem::onLoadTexture(const LoadTextureEvent& event) const {
    // Get renderer component
    auto rendererEntities = _ecsManager.getAllEntitiesWith<RendererComponent>();
    auto& rendererComp = _ecsManager.getComponent<RendererComponent>(rendererEntities[0]);

    // Load texture via RenderManager
    rendererComp.renderManager->loadTexture(event.textureId, event.path);
}
```

**Usage from client**:
```cpp
// Client publishes events, RenderSystem handles them
ecs->publishEvent(GameEngine::CreateWindowEvent("R-Type", 1280, 720));
ecs->publishEvent(GameEngine::CreateRendererEvent());
ecs->publishEvent(GameEngine::LoadTextureEvent("player", "assets/player.png"));
ecs->publishEvent(GameEngine::LoadFontEvent("ui_font", "assets/font.ttf", 24));
```

### Pattern 2: NetworkSystem Packet Flow

The `NetworkSystem` both publishes and subscribes to events.

**Location**: `engine/src/Systems/NetworkSystem/NetworkSystem.cpp`

**Subscription Side** (receiving commands):
```cpp
NetworkSystem::NetworkSystem() {
    // Subscribe to start network command
    world.subscribeEvent<StartNetworkEvent>(
        [this](const StartNetworkEvent& event) {
            onStartNetwork(event);
        });

    // Subscribe to send packet command
    world.subscribeEvent<SendPunctualPacketEvent>(
        [this](const SendPunctualPacketEvent& event) {
            onSendPunctualPacket(event);
        });

    // Subscribe to stop network command
    world.subscribeEvent<StopNetworkEvent>(
        [this](const StopNetworkEvent& event) {
            onStopNetwork(event);
        });
}

void NetworkSystem::onSendPunctualPacket(const SendPunctualPacketEvent& event) {
    // Queue packet for sending
    _networkManager.sendPacket(event.id, event.data);
}
```

**Publishing Side** (broadcasting received data):
```cpp
void NetworkSystem::update(EcsManager& world, float deltaTime) {
    // Send queued packets
    _networkManager.update(deltaTime);

    // Receive incoming packets and publish events
    processReceivedPackets(world);
}

void NetworkSystem::processReceivedPackets(EcsManager& world) {
    // Get packets from network manager
    auto packets = _networkManager.receivePackets();

    // Publish event for each received packet
    for (auto& packet : packets) {
        ReceivedPacket receivedPacket(std::move(packet.data));
        world.publishEvent(receivedPacket);  // Other systems receive this!
    }
}
```

### Pattern 3: Input to Action Mapping

The `InputSystem` and `InputManager` convert low-level input to high-level actions.

**Location**: `engine/src/Systems/InputSystem/InputManager.cpp`

```cpp
void InputManager::onLowLevelInputEvent(const LowLevelInputEvent& event) {
    // Process raw SDL event
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN: {
            // Check key bindings
            auto actionName = getActionForKey(event.key.scancode);
            if (!actionName.empty()) {
                // Publish high-level action event
                InputActionEvent actionEvent(actionName, true, 0);
                _ecsManager->publishEvent(actionEvent);
            }
            break;
        }
        // ...
    }
}
```

**Subscriber Example** (from client):
```cpp
// Client subscribes to high-level actions
ecs->subscribeEvent<GameEngine::InputActionEvent>(
    [&](const GameEngine::InputActionEvent& event) {
        if (event.actionName == "Shoot" && event.pressed) {
            std::cout << "Player shooting!\n";

            // Create network packet for server
            auto packet = Serializer::serializePacket(
                PacketType::INPUT,
                InputPayload{/* shoot action */}
            );

            // Send to server
            ecs->publishEvent(GameEngine::SendPunctualPacketEvent(1, packet));  // packetId=1
        }
    });
```

### Pattern 4: Multi-System Coordination

Example: Enemy death triggers multiple systems

```cpp
// CollisionSystem detects bullet hit enemy
class CollisionSystem : public ISystem {
    void update(EcsManager& world, float deltaTime) override {
        // Detect collision
        if (bulletHitEnemy) {
            // Mark enemy for deletion
            world.addComponent(enemyEntity, MarkedForDeletionComponent{});

            // Publish event for other systems
            world.publishEvent(EnemyDestroyedEvent{enemyEntity, playerWhoShot});
        }
    }
};

// ScoreSystem listens for enemy deaths
class ScoreSystem : public ISystem {
    ScoreSystem(EcsManager& world) {
        world.subscribeEvent<EnemyDestroyedEvent>(
            [&](const EnemyDestroyedEvent& e) {
                auto& player = world.getComponent<PlayerComponent>(e.playerId);
                player.score += 100;
                std::cout << "Score: " << player.score << "\n";
            });
    }
};

// AudioSystem plays sound effect
class AudioSystem : public ISystem {
    AudioSystem(EcsManager& world) {
        world.subscribeEvent<EnemyDestroyedEvent>(
            [&](const EnemyDestroyedEvent& e) {
                playSound("explosion");
            });
    }
};

// ParticleSystem spawns explosion
class ParticleSystem : public ISystem {
    ParticleSystem(EcsManager& world) {
        world.subscribeEvent<EnemyDestroyedEvent>(
            [&](const EnemyDestroyedEvent& e) {
                spawnExplosion(e.entityId);
            });
    }
};
```

**Result**: One event triggers multiple independent behaviors.

### Common Event Patterns

**1. Request-Response Pattern**:
```cpp
// System A requests data
world.publishEvent(RequestEntityDataEvent{entityId});

// System B responds
world.subscribeEvent<RequestEntityDataEvent>([](const auto& e) {
    // Respond with data event
    world.publishEvent(EntityDataResponseEvent{entityId, data});
});
```

**2. Broadcast Pattern**:
```cpp
// One system broadcasts state change
world.publishEvent(GameStateChangedEvent{newState});

// Multiple systems react
world.subscribeEvent<GameStateChangedEvent>([](const auto& e) {
    // Update UI
});
world.subscribeEvent<GameStateChangedEvent>([](const auto& e) {
    // Update audio
});
```

**3. Chain Pattern**:
```cpp
// System A publishes event
world.publishEvent(Event1{});

// System B receives Event1, publishes Event2
world.subscribeEvent<Event1>([](const auto& e) {
    world.publishEvent(Event2{});
});

// System C receives Event2
world.subscribeEvent<Event2>([](const auto& e) {
    // Final action
});
```

---

## Component Storage

Components are stored using a sparse-set-like pattern:

```cpp
template<typename T>
class ComponentStorage : public IComponentStorage<T> {
    // Dense array of components
    std::vector<T> components;

    // Map entity ID -> component index
    std::unordered_map<EntityId, size_t> entityToIndex;

    // List of entity IDs (parallel to components array)
    std::vector<EntityId> entities;
};
```

**Benefits:**
- O(1) component access by entity ID
- Cache-friendly iteration over all components of a type
- Minimal memory overhead

## Query System

The `getAllEntitiesWith<Components...>()` method enables efficient multi-component queries:

```cpp
// Get all entities with Position AND Velocity
auto entities = ecsManager.getAllEntitiesWith<Position, Velocity>();

// Get all entities with Position, Velocity, AND Sprite
auto renderables = ecsManager.getAllEntitiesWith<Position, Velocity, Sprite>();
```

**Optimization:**
- Finds the smallest component set first
- Uses hash sets for intersection testing
- Minimizes iteration overhead

## System Update Loop

The engine update loop follows this pattern:

```cpp
float deltaTime = calculateDeltaTime();

// EcsManager calls SystemManager
ecsManager.runEngine(deltaTime);

// SystemManager executes each registered system
for (auto& system : systems) {
    system->update(ecsManager, deltaTime);
}

// Process deferred events at end of frame
busEventManager.processDeferredEvents();
```

Each system typically:
1. **Subscribes to events** - React to other systems
2. **Queries entities** with required components
3. **Reads/modifies** component data
4. **Publishes events** - Notify other systems of changes
5. Deferred events are processed **after all systems update**

## Example System Implementation

Here's how to implement a system using the **event-driven pattern**:

```cpp
class MovementSystem : public ISystem {
private:
    EcsManager& _ecs;

public:
    // Constructor: subscribe to events
    explicit MovementSystem(EcsManager& ecs) : _ecs(ecs) {
        // Subscribe to collision events
        ecs.subscribeEvent<CollisionEvent>([this](const CollisionEvent& e) {
            onCollision(e);
        });

        // Subscribe to spawn events
        ecs.subscribeEvent<SpawnEntityEvent>([this](const SpawnEntityEvent& e) {
            std::cout << "Entity spawned: " << e.entityId << "\n";
        });
    }

    // Update: process entities and publish events
    void update(EcsManager& world, float deltaTime) override {
        // Query entities with Position and Velocity
        auto entities = world.getAllEntitiesWith<PositionComponent, VelocityComponent>();

        for (EntityId entity : entities) {
            auto& pos = world.getComponent<PositionComponent>(entity);
            auto& vel = world.getComponent<VelocityComponent>(entity);

            // Update position
            pos.x += vel.x * deltaTime;
            pos.y += vel.y * deltaTime;

            // Check bounds and publish event if out of bounds
            if (pos.x < 0 || pos.x > 1280) {
                world.publishEvent(OutOfBoundsEvent{entity, pos.x, pos.y});
            }
        }
    }

private:
    void onCollision(const CollisionEvent& event) {
        // React to collision
        if (_ecs.hasComponent<VelocityComponent>(event.entityId)) {
            auto& vel = _ecs.getComponent<VelocityComponent>(event.entityId);
            vel.x = -vel.x;  // Bounce
        }
    }
};
```

**Key patterns shown**:
-  Subscribe to events in **constructor**
-  Publish events during **update()**
-  React to events with **callbacks**
-  No direct system-to-system calls

> **📖 See Also**: [System Communication Patterns](#system-communication-patterns) for more event-driven examples.

## Adding New Components

To add a new component type:

1. **Define the component struct**:
```cpp
struct HealthComponent : public GameEngine::IComponent {
    float current;
    float maximum;
};
```

2. **Register it with the ECS**:
```cpp
ecs->registerComponent<HealthComponent>();
```

3. **Add to entities**:
```cpp
EntityId player = ecs->createEntity();
ecs->addComponent(player, HealthComponent{100.0F, 100.0F});
```

4. **Publish events when component changes** (event-driven pattern):
```cpp
// System that modifies health
class DamageSystem : public ISystem {
    void update(EcsManager& world, float deltaTime) override {
        auto entities = world.getAllEntitiesWith<HealthComponent, DamagedComponent>();

        for (auto entity : entities) {
            auto& health = world.getComponent<HealthComponent>(entity);
            auto& damaged = world.getComponent<DamagedComponent>(entity);

            health.current -= damaged.amount;

            // Publish event so other systems can react
            world.publishEvent(HealthChangedEvent{entity, health.current});

            if (health.current <= 0) {
                world.publishEvent(EntityDeathEvent{entity});
            }

            world.removeComponent<DamagedComponent>(entity);
        }
    }
};
```

**Best practice**: Publish events when important component changes occur

---

## Practical Integration Guide

This section provides a complete, step-by-step guide for using the R-Type engine in your own game project.

### Step 1: Project Setup

**Directory Structure**:
```
my-game/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── systems/
│   │   ├── PlayerSystem.cpp
│   │   └── PlayerSystem.hpp
│   └── components/
│       └── PlayerComponents.hpp
└── external/
    └── R-Type/engine/  (git submodule or copy)
```

**CMakeLists.txt**:
```cmake
cmake_minimum_required(VERSION 3.20)
project(MyGame CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure engine systems
set(GAMEENGINE_BUILD_RENDER_SYSTEM ON)
set(GAMEENGINE_BUILD_INPUT_SYSTEM ON)
set(GAMEENGINE_BUILD_AUDIO ON)
set(GAMEENGINE_BUILD_NETWORK OFF)  # Not needed for single-player
set(GAMEENGINE_BUILD_SCRIPTING ON)
set(GAMEENGINE_BUILD_LEVEL_SYSTEM ON)

# Add engine as external package
CPMAddPackage(
        NAME GameEngine 
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine
)
# Your game executable
add_executable(my_game
    src/main.cpp
    src/systems/PlayerSystem.cpp
)

target_link_libraries(my_game PRIVATE
    GameEngine::core
    GameEngine::render_system
    GameEngine::input_system
    GameEngine::audio
    GameEngine::scripting
    GameEngine::level_system
)
```

### Step 2: Define Components

**src/components/PlayerComponents.hpp**:
```cpp
#pragma once
#include <GameEngine/IComponent.hpp>

struct PlayerComponent : public GameEngine::IComponent {
    int lives = 3;
    int score = 0;
    bool invincible = false;
};

struct PositionComponent : public GameEngine::IComponent {
    float x = 0.0F;
    float y = 0.0F;
};

struct VelocityComponent : public GameEngine::IComponent {
    float x = 0.0F;
    float y = 0.0F;
};
```

### Step 3: Create Custom Systems

**src/systems/PlayerSystem.hpp**:
```cpp
#pragma once
#include <GameEngine/ISystem.hpp>
#include <GameEngine/EcsManager.hpp>

class PlayerSystem : public GameEngine::ISystem {
private:
    GameEngine::EcsManager& _ecs;

public:
    explicit PlayerSystem(GameEngine::EcsManager& ecs);
    void update(GameEngine::EcsManager& world, float deltaTime) override;
};
```

**src/systems/PlayerSystem.cpp**:
```cpp
#include "PlayerSystem.hpp"
#include "../components/PlayerComponents.hpp"
#include <GameEngine/InputEventTypes.hpp>
#include <iostream>

PlayerSystem::PlayerSystem(GameEngine::EcsManager& ecs) : _ecs(ecs) {
    // Subscribe to input actions
    ecs.subscribeEvent<GameEngine::InputActionEvent>(
        [this](const GameEngine::InputActionEvent& event) {
            if (event.actionName == "MoveUp" && event.pressed) {
                // Get player entity
                auto players = _ecs.getAllEntitiesWith<PlayerComponent, VelocityComponent>();
                if (!players.empty()) {
                    auto& vel = _ecs.getComponent<VelocityComponent>(players[0]);
                    vel.y = -200.0F;  // Move up
                }
            }
            else if (event.actionName == "MoveDown" && event.pressed) {
                auto players = _ecs.getAllEntitiesWith<PlayerComponent, VelocityComponent>();
                if (!players.empty()) {
                    auto& vel = _ecs.getComponent<VelocityComponent>(players[0]);
                    vel.y = 200.0F;  // Move down
                }
            }
        });
}

void PlayerSystem::update(GameEngine::EcsManager& world, float deltaTime) {
    // Query all player entities
    auto players = world.getAllEntitiesWith<PlayerComponent, PositionComponent, VelocityComponent>();

    for (auto entity : players) {
        auto& pos = world.getComponent<PositionComponent>(entity);
        auto& vel = world.getComponent<VelocityComponent>(entity);

        // Apply velocity
        pos.x += vel.x * deltaTime;
        pos.y += vel.y * deltaTime;

        // Clamp to screen bounds
        if (pos.x < 0.0F) pos.x = 0.0F;
        if (pos.x > 1280.0F) pos.x = 1280.0F;
        if (pos.y < 0.0F) pos.y = 0.0F;
        if (pos.y > 720.0F) pos.y = 720.0F;

        // Friction
        vel.x *= 0.95F;
        vel.y *= 0.95F;
    }
}
```

### Step 4: Main Game Loop

**src/main.cpp**:
```cpp
#include <GameEngine/EngineBuilder.hpp>
#include <GameEngine/TimeManager.hpp>
#include <GameEngine/RenderComponents.hpp>
#include <GameEngine/RenderEventTypes.hpp>
#include <GameEngine/InputEventTypes.hpp>
#include "components/PlayerComponents.hpp"
#include "systems/PlayerSystem.hpp"
#include <iostream>

int main() {
    // 1. Create engine with all built-in systems
    auto ecs = GameEngine::EngineBuilder::create();
    std::cout << "Engine created\n";

    // 2. Register custom components
    ecs->registerComponent<PlayerComponent>();
    ecs->registerComponent<PositionComponent>();
    ecs->registerComponent<VelocityComponent>();
    ecs->registerComponent<GameEngine::SpriteComponent>();
    ecs->registerComponent<GameEngine::TransformComponent>();
    std::cout << "Components registered\n";

    // 3. Register custom systems
    ecs->registerSystem<PlayerSystem>(*ecs);
    std::cout << "Systems registered\n";

    // 4. Initialize engine via events
    ecs->publishEvent(GameEngine::CreateWindowEvent("My Game", 1280, 720));
    ecs->publishEvent(GameEngine::CreateRendererEvent());
    std::cout << "Window and renderer created\n";

    // 5. Load assets
    ecs->publishEvent(GameEngine::LoadTextureEvent("player", "assets/player.png"));
    ecs->publishEvent(GameEngine::LoadFontEvent("ui_font", "assets/font.ttf", 24));
    std::cout << "Assets loaded\n";

    // 6. Create player entity
    GameEngine::EntityId player = ecs->createEntity();
    ecs->addComponent(player, PlayerComponent{3, 0, false});
    ecs->addComponent(player, PositionComponent{640.0F, 360.0F});
    ecs->addComponent(player, VelocityComponent{0.0F, 0.0F});
    ecs->addComponent(player, GameEngine::TransformComponent{640.0F, 360.0F, 0.0F});

    GameEngine::SpriteComponent sprite{};
    sprite.textureId = "player";
    sprite.srcX = 0; sprite.srcY = 0;
    sprite.srcW = 64; sprite.srcH = 64;
    sprite.dstX = 640; sprite.dstY = 360;
    sprite.dstW = 64; sprite.dstH = 64;
    sprite.layer = 10;
    ecs->addComponent(player, sprite);
    std::cout << "Player created\n";

    // 7. Subscribe to quit event
    bool shuttingDown = false;
    ecs->subscribeEvent<GameEngine::QuitEvent>([&](const GameEngine::QuitEvent&) {
        std::cout << "Quit requested\n";
        shuttingDown = true;
    });

    // 8. Main game loop
    GameEngine::TimeManager time(60.0F);  // 60 FPS target
    std::cout << "Starting game loop\n";

    while (!shuttingDown) {
        time.update([&](float deltaTime) {
            ecs->runEngine(deltaTime);  // Updates all systems
        });
    }

    std::cout << "Game exiting\n";
    return 0;
}
```

### Step 5: Configure Input Bindings

Create `assets/bindings.json`:
```json
{
  "contexts": [
    {
      "name": "gameplay",
      "actions": [
        {"name": "MoveUp", "keys": ["W", "Up"]},
        {"name": "MoveDown", "keys": ["S", "Down"]},
        {"name": "MoveLeft", "keys": ["A", "Left"]},
        {"name": "MoveRight", "keys": ["D", "Right"]},
        {"name": "Shoot", "keys": ["Space"]}
      ]
    }
  ]
}
```

Load bindings in main.cpp:
```cpp
#include <GameEngine/InputEventTypes.hpp>

// After creating engine
ecs->publishEvent(GameEngine::LoadInputBindingsEvent("assets/bindings.json"));
ecs->publishEvent(GameEngine::SetInputContextEvent("gameplay"));
```

### Step 6: Build and Run

```bash
# Build
mkdir build && cd build
cmake ..
cmake --build .

# Run
./my_game
```

### Integration Checklist

 **Engine Setup**
-  CMakeLists.txt configured with engine subdirectory
-  Engine systems selected (GAMEENGINE_BUILD_* flags)
-  Correct link targets (GameEngine::core, etc.)

 **Component Design**
-  All components inherit from `IComponent`
-  Components registered with `registerComponent<T>()`
-  Components are pure data (no logic)

 **System Implementation**
-  All systems inherit from `ISystem`
-  Implemented `update(EcsManager&, float)` method
-  Systems registered with `registerSystem<T>()`
-  Event subscriptions in constructor

 **Initialization**
-  Used `EngineBuilder::create()` to create engine
-  Published `CreateWindowEvent` and `CreateRendererEvent`
-  Loaded assets via `LoadTextureEvent`, `LoadFontEvent`
-  Created entities and added components

 **Game Loop**
-  Used `TimeManager` for fixed timestep
-  Called `ecs->runEngine(deltaTime)` each frame
-  Subscribed to `QuitEvent` for graceful shutdown

### Common Integration Patterns

**Pattern 1: Spawning Entities**
```cpp
// Define a spawn function
GameEngine::EntityId spawnEnemy(GameEngine::EcsManager& ecs, float x, float y) {
    auto enemy = ecs.createEntity();
    ecs.addComponent(enemy, EnemyComponent{});
    ecs.addComponent(enemy, PositionComponent{x, y});
    ecs.addComponent(enemy, VelocityComponent{-100.0F, 0.0F});

    GameEngine::SpriteComponent sprite{};
    sprite.textureId = "enemy";
    sprite.layer = 10;
    ecs.addComponent(enemy, sprite);

    return enemy;
}
```

**Pattern 2: Cleanup System**
```cpp
class CleanupSystem : public GameEngine::ISystem {
public:
    void update(GameEngine::EcsManager& world, float deltaTime) override {
        auto entities = world.getAllEntitiesWith<MarkedForDeletionComponent>();
        for (auto entity : entities) {
            world.destroyEntity(entity);
        }
    }
};
```

**Pattern 3: Event Communication**
```cpp
// Define custom event
struct PlayerDeathEvent {
    GameEngine::EntityId playerId;
};

// System publishes event
class HealthSystem : public GameEngine::ISystem {
    void update(GameEngine::EcsManager& world, float deltaTime) override {
        auto entities = world.getAllEntitiesWith<HealthComponent>();
        for (auto entity : entities) {
            auto& health = world.getComponent<HealthComponent>(entity);
            if (health.current <= 0) {
                world.publishEvent(PlayerDeathEvent{entity});
                world.destroyEntity(entity);
            }
        }
    }
};

// Other systems subscribe
class UISystem : public GameEngine::ISystem {
    UISystem(GameEngine::EcsManager& ecs) {
        ecs.subscribeEvent<PlayerDeathEvent>([](const PlayerDeathEvent& e) {
            std::cout << "Game Over!\n";
        });
    }
};
```

### Debugging Tips

**1. Enable Logger System**:
```cmake
set(GAMEENGINE_BUILD_LOGGER_SYSTEM ON)
```

```cpp
#include <GameEngine/LoggerSystem.hpp>

// Set log level
ecs->publishEvent(GameEngine::SetLogLevelEvent(GameEngine::LogLevel::DEBUG));

// Log messages
LOG_DEBUG("Player position: {}, {}", pos.x, pos.y);
LOG_INFO("Enemy spawned");
LOG_ERROR("Failed to load texture");
```

**2. Query Debugging**:
```cpp
// Print all entities with a component
auto entities = ecs->getAllEntitiesWith<PlayerComponent>();
std::cout << "Found " << entities.size() << " players\n";
```

**3. Event Debugging**:
```cpp
// Enable event history
ecs->enableEventHistory<InputActionEvent>();

// Print recent events (last 20)
auto history = ecs->getRecentEvents<InputActionEvent>(20);
for (const auto& event : history) {
    std::cout << "Action: " << event.actionName << "\n";
}
```

### Next Steps

- **Add More Systems**: AI, Particles, etc.
- **Use ScriptingSystem**: Add Lua scripts for game logic
- **Use LevelSystem**: Load levels from JSON files
- **Add Networking**: Enable multiplayer with NetworkSystem
- **Optimize**: Profile and optimize performance bottlenecks

### Resources

- [Rendering System](rendering-system.md) - How to render sprites and text
- [Input System](input-system.md) - How to handle input
- [Scripting System](scripting-system.md) - How to use Lua
- [Level System](level-system.md) - How to load levels from JSON
- [Network Architecture](../network/network-architecture.md) - How to add multiplayer

---

## Performance Characteristics

| Operation | Complexity |
|-----------|------------|
| Create entity | O(1) |
| Add component | O(1) amortized |
| Get component | O(1) |
| Remove component | O(1) |
| Query entities | O(n) where n = smallest component set |
| System update | O(m) where m = matching entities |

## Design Benefits

- **Decoupling**: Systems don't depend on each other (see [Event-Driven Architecture](#event-driven-architecture))
- **Flexibility**: Easy to add/remove components dynamically
- **Performance**: Cache-friendly data layout
- **Reusability**: Engine is game-agnostic (see [CMake Modular Architecture](#cmake-modular-architecture))
- **Testability**: Systems can be tested in isolation

> **Ready to use the engine?** Jump to [Practical Integration Guide](#practical-integration-guide) to build your first game.

## Engine Systems

The engine provides 8 reusable systems that can be used in any game project:

### Core Engine Systems (`engine/src/Systems/`)

**RenderSystem**
- Sprite, text, and shape rendering
- SDL3-based graphics with abstraction layer
- Texture caching and management
- Layering system for z-ordering
- See [Rendering System](rendering-system.md) documentation

**InputSystem**
- Keyboard, mouse, and gamepad support
- Runtime key rebinding
- Context-based input management
- Action and axis mapping
- See [Input System](input-system.md) documentation

**AudioSystem**
- Sound effect playback with instance pooling
- Music playback with looping
- Volume control
- miniaudio backend
- See [Audio System](audio-system.md) documentation

**NetworkSystem**
- UDP networking via Boost.Asio
- Client-server communication
- Packet serialization/deserialization
- Connection management
- See [Network Architecture](../network/network-architecture.md) documentation

**ScriptingSystem**
- Lua 5.4.7 runtime integration
- Component access from scripts
- Hot reload support
- Script lifecycle management (onInit, onUpdate, onDestroy)
- See [Scripting System](scripting-system.md) documentation

**LevelSystem**
- JSON-based level loading
- Prefab system for entity templates
- Wave management
- Dynamic entity spawning
- See [Level System](level-system.md) documentation

**WindowSystem**
- SDL window lifecycle management
- Window creation and configuration
- Event handling
- Cross-platform window support

**LoggerSystem**
- ECS-integrated logging
- Multiple log levels
- System-specific log filtering

---

## R-Type Game Systems

The R-Type game extends the engine with game-specific systems for client presentation and server gameplay logic.

### Client Systems (`client/src/systems/`)

Client systems handle presentation, UI, and network communication:

**MenuSystem** (`MenuSystem/`)
- Main menu, pause menu, settings UI
- Handles menu navigation and state transitions
- Integrates with InputSystem for menu controls
- Runtime key rebinding interface

**UISystem** (`UISystem/`)
- HUD rendering (health, score, lives)
- UI element positioning and updates
- Player status visualization

**AnimationSystem** (`AnimationSystem/`)
- Sprite animation playback
- Frame-based animation states
- Animation timing and transitions

**PlayerAnimationSystem** (`PlayerAnimationSystem/`)
- Player-specific animation states
- Movement and shooting animations
- Animation blending and transitions

**BlinkingSystem** (`BlinkingSystem/`)
- Invincibility visual feedback
- Damage indication effects
- Toggles entity visibility at intervals

**InputMappingSystem** (`InputMappingSystem/`)
- Maps InputActionEvents to network packets
- Converts player input to server commands
- Handles input → gameplay translation

**NetworkPacketSystem** (`NetworkPacketSystem/`)
- Receives entity updates from server
- Applies server state to local entities
- Handles connection management

### Server Systems (`server/src/systems/`)

Server systems implement authoritative game logic and state management:

**MovementSystem** (`MovementSystem/`)
- Applies velocity to entity positions
- Physics simulation and movement updates
- Example:
```cpp
class MovementSystem : public ISystem {
    void update(EcsManager &ecs, float deltaTime) override {
        auto entities = ecs.getAllEntitiesWith<PositionComponent, VelocityComponent>();
        for (auto entity : entities) {
            auto& pos = ecs.getComponent<PositionComponent>(entity);
            auto& vel = ecs.getComponent<VelocityComponent>(entity);
            pos.x += vel.x * deltaTime;
            pos.y += vel.y * deltaTime;
        }
    }
};
```

**WeaponSystem** (`WeaponSystem/`)
- Processes shoot requests from players
- Creates projectile entities
- Manages fire rates and cooldowns
- Autofire support

**CollisionSystem** (`CollisionSystem/`)
- Bullet-enemy collision detection
- Bullet-player collision detection
- Damage application
- Circular collision algorithm

**EnemySpawnSystem** (`EnemySpawnSystem/`)
- Wave-based enemy spawning
- Progressive difficulty (spawn interval decreases)
- Supports 4 enemy types: BASIC, SINE_WAVE, TANK, SHOOTER

**EnemyAISystem** (`EnemyAISystem/`)
- AI movement patterns per enemy type
- Enemy shooting behavior
- Sine wave, straight line, and advanced patterns

**ServerLevelSystem** (`ServerLevelSystem/`)
- Level progression and wave management
- Boss spawning after waves complete
- Integrates with LevelSystem for JSON configuration

**EntitySynchronizerSystem** (`EntitySynchronizerSystem/`)
- Broadcasts entity state to all clients
- Configurable sync rate (60-120 Hz)
- Optimized for bandwidth efficiency

**BoundsCleanupSystem** (`BoundsCleanupSystem/`)
- Marks off-screen entities for deletion
- Prevents memory leaks from escaped entities
- Configurable boundary margins

**EntityCleanupSystem** (`EntityCleanupSystem/`)
- Removes entities marked for deletion
- Notifies clients of destroyed entities
- Runs last in update order

**PlayerDeathSystem** (`PlayerDeathSystem/`)
- Player respawn logic
- Lives management
- Invincibility periods

**ScoreSystem** (`ScoreSystem/`)
- Score tracking per player
- Kill counting and point awards

**NetworkPacketSystem** (`NetworkPacketSystem/`)
- Receives client input packets
- Processes connection/disconnection
- Handles protocol messages

### Component Examples

**Server Components** (`server/src/components/`)
- `PositionComponent` - Entity position (x, y)
- `VelocityComponent` - Movement speed
- `HealthComponent` - Entity health
- `WeaponComponent` - Weapon state and cooldown
- `NetworkComponent` - Network sync data
- `BulletComponent` - Projectile properties
- `MarkedForDeletionComponent` - Tag component for entities pending deletion
- `PlayerComponent` - Player-specific state (score, lives, death, invincibility)
- `EnemyComponent` - Enemy-specific state
- `CollideRectComponent` - Collision detection bounds
- `TransformComponent` - Transform data (position, rotation)
- `ShootRequestComponent` - Tag component requesting weapon firing (processed by WeaponSystem)

**Client Components** (`client/src/components/`)
- `KeyboardComponent` - Keyboard input state
- `MouseComponent` - Mouse position and clicks
- `SDLComponents` - Sprite, texture rendering data

## Separation of Concerns

| Layer | Location | Responsibility | Examples |
|-------|----------|----------------|----------|
| Engine Core | `engine/src/` | ECS infrastructure | EntityManager, ComponentManager, SystemManager |
| Game Systems | `client/src/systems/` | Client-side logic | RenderSystem, InputSystem |
| Game Systems | `server/src/systems/` | Server-side logic | MovementSystem, WeaponSystem |
| Game Components | `client/src/components/` | Client data | KeyboardComponent, SDLComponents |
| Game Components | `server/src/components/` | Server data | PositionComponent, VelocityComponent |

## Why This Separation?

✓ **Reusability**: The engine core can be used for any game, not just R-Type
✓ **Maintainability**: Clear boundaries between generic and specific code
✓ **Testability**: Engine core can be tested without game dependencies
✓ **Modularity**: Game systems can be added/removed without touching the engine
✓ **Clarity**: Developers immediately know where to add game features

## Next Steps

### For Engine Users (Game Developers)

1. **[Practical Integration Guide](#practical-integration-guide)** - Complete step-by-step tutorial
2. **[Rendering System](rendering-system.md)** - Display sprites and text
3. **[Input System](input-system.md)** - Handle keyboard, mouse, gamepad
4. **[Scripting System](scripting-system.md)** - Add Lua logic with hot reload
5. **[Level System](level-system.md)** - Load levels from JSON files
6. **[Audio System](audio-system.md)** - Play sounds and music

### For Contributors

- [Network Protocol](../network/protocol.md) - Network communication protocol
- [Build Instructions](../development/build.md) - How to compile the project
- [Project Overview](../project-overview.md) - High-level project information

### Architecture Deep Dives

- [Event-Driven Architecture](#event-driven-architecture) - How systems communicate
- [CMake Modular Architecture](#cmake-modular-architecture) - How the engine is structured
- [Builder Pattern & System Registration](#builder-pattern--system-registration) - Initialization patterns
- [System Communication Patterns](#system-communication-patterns) - Real-world event examples

---

**Last Updated:** November 2025

**Version:** Engine Architecture 2.0
