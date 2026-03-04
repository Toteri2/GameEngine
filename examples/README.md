# Lua Scripting Test - How to Verify Everything Works

This directory contains a **complete test** to verify that Lua scripts can successfully control entities in your R-Type game engine.

## What the Test Does

The `example_script.lua` will make entities:
1. **Move in a circular pattern** (100 pixel radius)
2. **Change velocity** dynamically to follow the circle
3. **Oscillate health** between 50-100
4. **Print colorful status updates** every second
5. **Print ASCII art** every 5 seconds
6. **Show initialization and destruction** messages

This is **VERY NOTICEABLE** - you'll see:
- Entities physically moving in circles
- Console output with emojis and ASCII art
- Health bars updating
- Position coordinates changing

---

## Quick Start - Two Ways to Test

### **Method 1: Use the Test Program** (Recommended)

Add this to your `server/CMakeLists.txt` or create a separate test executable:

```cmake
# Add test executable for scripting
add_executable(test_scripting
    ../engine/examples/test_scripting.cpp
)

target_link_libraries(test_scripting PRIVATE
    game_engine_scripting
    # Add other necessary libraries
)

# Copy the Lua script to build directory
configure_file(
    ${CMAKE_SOURCE_DIR}/engine/examples/example_script.lua
    ${CMAKE_BINARY_DIR}/engine/examples/example_script.lua
    COPYONLY
)
```

Then compile and run:

```bash
cmake --build build
./build/test_scripting
```

You should see **colorful console output** showing the entity moving in circles!

---

### **Method 2: Add to Your Existing Game/Server**

In your server's main initialization code (e.g., `server/src/main.cpp`):

```cpp
#include "../engine/include/GameEngine/ScriptingHelpers.hpp"
#include "../engine/src/Systems/ScriptingSystem/ScriptingSystem.hpp"

// In your initialization code:
void setupScriptingTest(EcsManager& ecs, ScriptingSystem& scriptingSystem) {
    auto& scriptManager = scriptingSystem.getScriptManager();

    // Register components (do this ONCE at startup)
    REGISTER_LUA_COMPONENT(scriptManager, Position, Rtype::PositionComponent, x, y);
    REGISTER_LUA_COMPONENT(scriptManager, Velocity, Rtype::VelocityComponent, vx, vy);
    REGISTER_LUA_COMPONENT(scriptManager, Health, Rtype::HealthComponent, health);

    // Create a test entity
    EntityId testEntity = ecs.createEntity();
    ecs.addComponent<Rtype::PositionComponent>(testEntity, {400.0f, 300.0f});
    ecs.addComponent<Rtype::VelocityComponent>(testEntity, {0.0f, 0.0f});
    ecs.addComponent<Rtype::HealthComponent>(testEntity, {100});

    // Attach the script
    Scripting::ScriptComponent scriptComp;
    scriptComp.path = "engine/examples/example_script.lua";
    scriptComp.autoInit = true;
    ecs.addComponent<Scripting::ScriptComponent>(testEntity, scriptComp);

    std::cout << "✓ Test entity created with Lua script!\n";
}

// In your game loop:
void gameLoop() {
    while (running) {
        float deltaTime = calculateDeltaTime();

        // This updates all Lua scripts!
        scriptingSystem.update(ecs, deltaTime);

        // ... rest of your game loop
    }
}
```

---

## What You Should See

When the test runs successfully, you'll see output like this:

```
============================================================
ENTITY 1 INITIALIZED - LUA SCRIPT ACTIVE!
============================================================
Starting Position: (400.00, 300.00)
Health set to: 100
Velocity set to: (50, 0)
Lua script will now control this entity's movement!
============================================================

┌─── Entity 1 Status (t=1.0s) ───
│ Position: (499.0, 341.9)
│ Velocity: (-84.1, 90.9)
│ Health: [███████░░░] 79%
│ Angle: 114°
└──────────────────────────────────────────────

┌─── Entity 1 Status (t=2.0s) ───
│ Position: (359.0, 392.8)
│ Velocity: (-181.9, -41.6)
│ Health: [████████░░] 84%
│ Angle: 229°
└──────────────────────────────────────────────

★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
    LUA SCRIPT IS WORKING!
    Entity 1 is moving in circles!
★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★

Entity 1 completed a full circle!

============================================================
ENTITY 1 DESTROYED!
Total lifetime: 15.00 seconds
Rotations completed: 4.77
Final health: 75
============================================================
```

---

## Verification Checklist

After running the test, verify:

- [ ] **Initialization message appears** with entity ID
- [ ] **Position changes every second** (coordinates should vary)
- [ ] **Velocity changes** to follow circular path
- [ ] **Health oscillates** between 50-100 (you'll see the health bar change)
- [ ] **Angle increases** from 0° to 360° and wraps
- [ ] **"Completed a full circle" message** appears multiple times
- [ ] **ASCII art appears** every 5 seconds
- [ ] **Destruction message** shows total lifetime and rotations

**If you see all of the above, Lua scripting is working perfectly!**

---

## Troubleshooting

### Problem: "Script file not found"
**Solution:** Make sure the path is correct. Try absolute path:
```cpp
scriptComp.path = "/full/path/to/engine/examples/example_script.lua";
```

### Problem: No output after "ENTITY X INITIALIZED"
**Possible causes:**
1. `scriptingSystem.update()` is not being called in game loop
2. Components aren't registered before script loads
3. Entity doesn't have the required components

**Fix:**
- Add debug print in game loop: `std::cout << "Updating scripts..." << std::endl;`
- Register components BEFORE creating entities with scripts
- Verify entity has Position, Velocity, and Health components

### Problem: "attempt to call a nil value (global 'getPosition')"
**Cause:** Components not registered with Lua

**Fix:** Add before loading any scripts:
```cpp
REGISTER_LUA_COMPONENT(scriptManager, Position, Rtype::PositionComponent, x, y);
REGISTER_LUA_COMPONENT(scriptManager, Velocity, Rtype::VelocityComponent, vx, vy);
REGISTER_LUA_COMPONENT(scriptManager, Health, Rtype::HealthComponent, health);
```

### Problem: Position doesn't change
**Possible causes:**
1. Entity doesn't have PositionComponent
2. Script's onUpdate isn't being called
3. deltaTime is 0 or very small

**Debug:** Add to Lua script's onUpdate:
```lua
print("DEBUG: onUpdate called, deltaTime=" .. deltaTime)
```

---

## Creating Your Own Scripts

Use this template for new scripts:

```lua
local entities = {}

function onInit(entity)
    print("Initializing entity: " .. entity)
    entities[entity] = {
        -- Your state here
    }
end

function onUpdate(entity, deltaTime)
    local state = entities[entity]
    if not state then return end

    -- Modify components here
    if hasPosition(entity) then
        local pos = getPosition(entity)
        pos.x = pos.x + 100 * deltaTime  -- Move right
    end
end

function onDestroy(entity)
    print("Destroying entity: " .. entity)
    entities[entity] = nil
end
```

---

## 🎮 Next Steps

Once the test works:

1. **Create enemy AI scripts** - Use the circular motion as inspiration
2. **Create bullet patterns** - Modify velocity for different bullet behaviors
3. **Create boss patterns** - Complex movement and attack patterns in Lua
4. **Hot reload** - Modify scripts while the game runs (see SCRIPTING_REVIEW.md)

---

## Available Lua Functions

After registering components, you have these functions:

| Function | Description | Example |
|----------|-------------|---------|
| `hasPosition(entity)` | Check if entity has position | `if hasPosition(42) then ... end` |
| `getPosition(entity)` | Get position component | `local pos = getPosition(42)` |
| `hasVelocity(entity)` | Check if entity has velocity | `if hasVelocity(42) then ... end` |
| `getVelocity(entity)` | Get velocity component | `local vel = getVelocity(42)` |
| `hasHealth(entity)` | Check if entity has health | `if hasHealth(42) then ... end` |
| `getHealth(entity)` | Get health component | `local hp = getHealth(42)` |

You can modify component fields directly:
```lua
local pos = getPosition(entity)
pos.x = 100  -- This ACTUALLY changes the entity's position in C++!
pos.y = 200
```

**Note:** The changes are immediately visible in C++ because Lua modifies the actual component data!

---

## Success Criteria

Your test is successful if:
1. Entity appears at starting position (400, 300)
2. Position changes smoothly in a circular pattern
3. Velocity updates to match circular motion
4. Health oscillates between 50-100
5. Console shows updates every second
6. Script runs for ~15 seconds with continuous output
7. Destruction message appears at the end

**Congratulations! Your Lua scripting system is fully functional!**
