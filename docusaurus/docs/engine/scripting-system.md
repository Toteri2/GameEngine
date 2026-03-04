---
sidebar_position: 4
---

# Scripting System

Lua 5.4.7 integration with hot reload, component registration, and runtime game logic scripting.

## Overview

**Key Features:**
-  Lua 5.4.7 via LuaBridge3
-  Hot reload (edit scripts while game runs)
-  Component access from Lua
-  Entity manipulation API
-  Script lifecycle (onInit, onUpdate, onDestroy)
-  Component registration for Lua exposure

**Location:** `engine/src/Systems/ScriptingSystem/`

---

## Quick Start

### 1. Create a Lua Script

`scripts/enemy_ai.lua`:
```lua
function onInit(entity, ecs)
    print("Enemy initialized: " .. entity)
end

function onUpdate(entity, ecs, deltaTime)
    -- Access components
    if ecs:hasComponent_VelocityComponent(entity) then
        local vel = ecs:getComponent_VelocityComponent(entity)
        vel.x = -100  -- Move left
    end
end

function onDestroy(entity, ecs)
    print("Enemy destroyed: " .. entity)
end
```

### 2. Attach Script to Entity

```cpp
// Register component
ecs->registerComponent<ScriptComponent>();

// Create entity with script
EntityId enemy = ecs->createEntity();
ScriptComponent script{};
script.scriptPath = "scripts/enemy_ai.lua";
ecs->addComponent(enemy, script);
```

### 3. Script Runs Automatically

The ScriptingSystem will:
1. Load the script
2. Call `onInit(entity, ecs)` once
3. Call `onUpdate(entity, ecs, deltaTime)` every frame
4. Call `onDestroy(entity, ecs)` when entity destroyed

---

## ScriptComponent

```cpp
struct ScriptComponent : public GameEngine::IComponent {
    ScriptID scriptId = INVALID_SCRIPT_ID;     // Assigned by system
    std::optional<std::string> path;            // Path to .lua file
    bool autoInit = true;                       // Auto-initialize on load

    // Constructors
    ScriptComponent() = default;
    explicit ScriptComponent(const ScriptID id, const bool autoInitialize = true);
    explicit ScriptComponent(const std::string &path, const bool autoInitialize = true);
};
```

**Example:**
```cpp
EntityId player = ecs->createEntity();

// Create with path (preferred)
ecs->addComponent(player, ScriptComponent("scripts/player_ai.lua"));

// Or disable auto-initialization
ecs->addComponent(player, ScriptComponent("scripts/player_ai.lua", false));
```

---

## Lua API

### Entity Operations

```lua
-- Get entity ID
local entityId = entity

-- Check if entity exists
local exists = ecs:entityExists(entityId)
```

### Component Operations

```lua
-- Check if component exists
if ecs:hasComponent_PositionComponent(entity) then
    -- Get component
    local pos = ecs:getComponent_PositionComponent(entity)

    -- Modify component
    pos.x = 100
    pos.y = 200
end
```

## Hot Reload

The ScriptingSystem watches the `scripts/` directory. When you edit a `.lua` file:

1. **File saved**
2. **System detects change**
3. **Script reloaded**
4. **`onInit` called again** (with `initialized = false`)

**Benefits:**
- Tweak AI behavior without restarting
- Rapid iteration
- Live debugging
---

## Component Registration

To expose a component to Lua, register it:

```cpp
#include <GameEngine/ScriptingSystem.hpp>

// Define component
struct HealthComponent : public IComponent {
    float current;
    float maximum;
};

// Register with ECS
ecs->registerComponent<HealthComponent>();

// Register with Lua
auto scriptEntities = ecs->getAllEntitiesWith<ScriptManagerComponent>();
auto& scriptMgrComp = ecs->getComponent<ScriptManagerComponent>(scriptEntities[0]);

// To avoid complexity, we can use a macro
REGISTER_LUA_COMPONENT(scriptManager, Health, Rtype::HealthComponent, health);

// If you have more than 5 variables you can still use the classic method
GameEngine::ScriptManager::registerComponent<HealthComponent>(
    scriptMgrComp.scriptManager->getLuaState(),
    "HealthComponent"
);
```

Now Lua scripts can access it:
```lua
if ecs:hasComponent_HealthComponent(entity) then
    local health = ecs:getComponent_HealthComponent(entity)
    health.current = health.current - 10  -- Take damage
end
```

---

## Script Lifecycle

### onInit(entity, ecs)
Called **once** when:
- Entity created with ScriptComponent
- Script hot-reloaded
- Component added to existing entity

```lua
function onInit(entity, ecs)
    -- Setup
    print("Initializing entity " .. entity)
end
```

### onUpdate(entity, ecs, deltaTime)
Called **every frame** while entity exists:

```lua
function onUpdate(entity, ecs, deltaTime)
    -- Game logic
    if ecs:hasComponent_PositionComponent(entity) then
        local pos = ecs:getComponent_PositionComponent(entity)
        pos.x = pos.x + 100 * deltaTime
    end
end
```

### onDestroy(entity, ecs)
Called **once** when:
- Entity destroyed
- ScriptComponent removed

```lua
function onDestroy(entity, ecs)
    -- Cleanup
    print("Entity " .. entity .. " destroyed")
end
```

---

## Example: Enemy AI Script

`scripts/sine_wave_enemy.lua`:
```lua
local time = 0
local startY = 0

function onInit(entity, ecs)
    if ecs:hasComponent_PositionComponent(entity) then
        local pos = ecs:getComponent_PositionComponent(entity)
        startY = pos.y
    end
end

function onUpdate(entity, ecs, deltaTime)
    time = time + deltaTime

    if ecs:hasComponent_PositionComponent(entity) then
        local pos = ecs:getComponent_PositionComponent(entity)

        -- Sine wave pattern
        pos.y = startY + math.sin(time * 2.0) * 50.0
    end

    if ecs:hasComponent_VelocityComponent(entity) then
        local vel = ecs:getComponent_VelocityComponent(entity)
        vel.x = -150  -- Move left
    end
end

function onDestroy(entity, ecs)
    print("Sine wave enemy destroyed")
end
```

---

## When to Use Scripts vs C++

| Use Lua | Use C++ |
|---------|---------|
| AI behavior | Performance-critical code |
| Level-specific logic | Core engine systems |
| Rapid prototyping | Physics calculations |
| Mod support | Rendering |
| Content creation | Network protocol |
| Quick tweaks | Memory management |

**Rule of thumb:** Scripts for **game logic**, C++ for **engine systems**.

---

## Best Practices

- **Keep scripts simple** - complex logic in C++
- **Check components exist** before accessing
- **Use hot reload** for rapid iteration
- **Separate AI per enemy type** (different scripts)
- **Register all needed components** for Lua

---

## Troubleshooting

### Script Not Loading
- Check path relative to executable
- Verify `scripts/` directory exists
- Check for Lua syntax errors (see console)

### Component Not Accessible
- Register component with `ScriptManager::registerComponent<T>()`
- Use exact component name: `getComponent_PositionComponent`

### Hot Reload Not Working
- FileWatcher may not support your OS
- Try manual reload (stop/start game)
- Check file permissions

---

**Last Updated:** November 2025

**System Version:** ScriptingSystem 1.0
