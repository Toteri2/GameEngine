---
sidebar_position: 3
---

# Input System

Modern input abstraction supporting keyboard, mouse, and gamepad with runtime remapping and context-based input management.

## Overview

**Key Features:**
-  Multi-device support (keyboard, mouse, gamepad, joystick)
-  Runtime key rebinding
-  Context-based input (Gameplay, UI, Menu contexts with priorities)
-  Action/Axis mapping (buttons → actions, keys → axes)
-  Event-driven architecture
-  No SDL dependencies in client code

**Location:** `engine/src/Systems/InputSystem/`

---

## Quick Start

### 1. Setup Input Context

```cpp
// Create input context with priority
GameEngine::CreateContextEvent createCtx("Gameplay", 100);
ecs->publishEvent(createCtx);

// Bind keys to actions
GameEngine::ChangeBindingEvent upBinding(
    "Gameplay",           // Context name
    0,                    // Device type (0=keyboard)
    0,                    // Device index
    26,                   // Key scancode (W)
    "Move Up",            // Action name
    -1.0F                 // Axis value
);
ecs->publishEvent(upBinding);
```

### 2. Subscribe to Input Events

```cpp
// Subscribe to high-level action events
ecs->subscribeEvent<GameEngine::InputActionEvent>(
    [](const GameEngine::InputActionEvent& event) {
        if (event.actionName == "Jump" && event.pressed) {
            // Player jumped!
        }
    }
);

// Subscribe to mouse movement
ecs->subscribeEvent<GameEngine::MouseMotionEvent>(
    [](const GameEngine::MouseMotionEvent& event) {
        float mouseX = event.x;
        float mouseY = event.y;
    }
);
```

---

## Architecture

### Key Components

| Component | Purpose |
|-----------|---------|
| **InputManager** | Coordinates input processing |
| **SDLInputDevice** | Abstracts SDL3 input (keyboard, mouse, gamepad) |
| **InputBindingManager** | Maps physical inputs to logical actions |
| **InputState** | Stores current input state for polling |

---

## Event Types

### InputActionEvent
For button-based actions (shoot, jump, pause):

```cpp
struct InputActionEvent {
    std::string actionName;    // "Jump", "Shoot", "Pause"
    bool pressed;              // true = pressed, false = released
    float value;               // Analog value (0.0-1.0)
};
```

### InputAxisEvent
For continuous input (movement, aiming):

```cpp
struct InputAxisEvent {
    std::string axisName;      // "MoveX", "MoveY", "LookX"
    float value;               // -1.0 to 1.0
};
```

### KeyboardTouchEvent
For text input (UI, chat, rebinding):

```cpp
struct KeyboardTouchEvent {
    std::string textKey;       // "W", "Space", "Escape"
    std::uint32_t scancode;    // SDL scancode
    GameEngine::InputEventType eventType;  // BUTTON_PRESSED/RELEASED
};
```

### MouseMotionEvent
For mouse movement:

```cpp
struct MouseMotionEvent {
    float x, y;                // Screen coordinates
    float deltaX, deltaY;      // Movement delta
};
```

---

## Context System

Contexts allow different input modes (gameplay vs menu) with priority-based resolution:

```cpp
// Gameplay context (priority 100)
CreateContextEvent gameplay("Gameplay", 100);
ecs->publishEvent(gameplay);

// UI context (priority 200 - higher priority)
CreateContextEvent ui("UI", 200);
ecs->publishEvent(ui);

// When UI is active, it handles input first
// If UI doesn't consume input, Gameplay receives it
```

---

## Runtime Key Rebinding

### Step 1: Request Current Bindings

```cpp
GameEngine::RecoverBindingsEvent request("Gameplay");
ecs->publishEvent(request);
```

### Step 2: Receive Bindings

```cpp
ecs->subscribeEvent<GameEngine::SharedBindingsEvent>(
    [](const GameEngine::SharedBindingsEvent& event) {
        // event.bindings contains all bindings for the context
        for (const auto& binding : event.bindings) {
            std::cout << binding.targetName << " -> " << binding.keyName << "\n";
        }
    }
);
```

### Step 3: Change Binding

```cpp
GameEngine::ChangeBindingEvent newBinding(
    "Gameplay",        // Context
    0,                 // Keyboard
    0,                 // Device index
    44,                // New scancode (Space)
    "Jump",            // Action to rebind
    0.0F               // Value
);
ecs->publishEvent(newBinding);
```

**Example:** See `client/src/systems/MenuSystem/` for complete rebinding UI implementation.

---

## Device Support

### Keyboard
- Full scancode support
- Text input events
- Modifier keys

### Mouse
- Position tracking
- Button events (left, right, middle)
- Motion events with delta

### Gamepad
- Button mapping
- Analog sticks (axes)
- Triggers
- Hat switches

### Joystick
- Hat direction support
- Custom button mapping

---

## Example: Complete Input Setup

```cpp
// 1. Register InputSystem
ecs->registerSystem<GameEngine::InputSystem>(*ecs);

// 2. Create context
GameEngine::CreateContextEvent ctx("Gameplay", 100);
ecs->publishEvent(ctx);

// 3. Bind keys
std::vector<std::pair<int, std::string>> bindings = {
    {26, "Move Up"},     // W
    {22, "Move Down"},   // S
    {4, "Move Left"},    // A
    {7, "Move Right"},   // D
    {44, "Shoot"},       // Space
    {41, "Pause"}        // Escape
};

for (const auto& [scancode, action] : bindings) {
    GameEngine::ChangeBindingEvent binding(
        "Gameplay", 0, 0, scancode, action, 0.0F
    );
    ecs->publishEvent(binding);
}

// 4. Subscribe to actions
ecs->subscribeEvent<GameEngine::InputActionEvent>(
    [](const GameEngine::InputActionEvent& event) {
        if (event.actionName == "Shoot" && event.pressed) {
            // Fire weapon
        }
    }
);
```

---

## Integration with Game Systems

### Player Movement System

```cpp
class PlayerMovementSystem : public ISystem {
    void update(EcsManager& ecs, float deltaTime) override {
        // Input already processed by InputSystem
        // Just handle resulting actions via events
    }
};

// Subscribe in constructor
ecs.subscribeEvent<InputActionEvent>([&ecs](const auto& event) {
    if (event.actionName == "Move Up" && event.pressed) {
        auto players = ecs.getAllEntitiesWith<PlayerComponent>();
        for (auto player : players) {
            auto& vel = ecs.getComponent<VelocityComponent>(player);
            vel.y = -300.0F;  // Move up
        }
    }
});
```

---

## Best Practices

- **Use action names**, not keycodes (allows rebinding)
- **Use contexts** for different game modes (menu vs gameplay)
- **Subscribe to events**, don't poll (event-driven)
- **Provide default bindings** on first run
- **Let users rebind** all actions

---

**Last Updated:** November 2025

**System Version:** InputSystem 2.0
