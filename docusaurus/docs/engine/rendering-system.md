---
sidebar_position: 2
---

# Rendering System

The **RenderSystem** is one of the core engine systems, providing SDL3-based rendering capabilities abstracted away from client code. It handles sprites, text, shapes, and texture management with a component-based approach.

## Overview

**Key Features:**
-  SDL3 abstraction - no SDL dependencies in client code
-  Sprite rendering with scaling, rotation, color modulation
-  Text rendering with TrueType fonts (SDL3_ttf)
-  Shape primitives (rectangles, circles) via SDL3_gfx
-  Texture caching and management
-  Layering system for z-ordering
-  Event-driven architecture (`RenderEvent`, `WindowResizedEvent`)

**Location:** `engine/src/Systems/RenderSystem/`

---

## Architecture

### Component-Based Rendering

The RenderSystem follows a pure component-based architecture where the client code never directly interacts with SDL3:

```
┌─────────────────────────────┐
│    Game/Client Code         │
│                             │
│  - Creates entities         │
│  - Adds render components   │
│  - No SDL dependencies!     │
└──────────────┬──────────────┘
               │ Components
               ▼
┌──────────────────────────────┐
│      Engine Layer            │
│                              │
│  ┌────────────────────────┐  │
│  │   RenderSystem         │  │
│  │                        │  │
│  │  - Processes components│  │
│  │  - Manages textures    │  │
│  │  - Handles SDL3        │  │
│  └────────────────────────┘  │
└──────────────────────────────┘
               │
               ▼
        SDL3 (Graphics API)
```

**Benefits:**
- Game code uses only engine components
- Complete SDL3 abstraction
- Engine is reusable across projects
- Clean separation of concerns

---

## Components

The RenderSystem works with these engine-provided components:

### 1. SpriteComponent

For rendering textured sprites:

```cpp
struct SpriteComponent : public GameEngine::IComponent {
    std::string textureId;          // Texture identifier

    // Source rectangle (what part of texture to use)
    float srcX = 0, srcY = 0;
    float srcW = 0, srcH = 0;

    // Destination rectangle (where/how to render)
    float dstX = 0, dstY = 0;
    float dstW = 0, dstH = 0;

    // Rendering properties
    std::uint8_t colorR = 255;      // Color modulation
    std::uint8_t colorG = 255;
    std::uint8_t colorB = 255;
    std::uint8_t alpha = 255;        // Transparency

    int layer = 0;                   // Z-order (higher = front)
};
```

**Example Usage:**
```cpp
// Register component
ecs->registerComponent<SpriteComponent>();

// Create sprite entity
EntityId player = ecs->createEntity();

SpriteComponent sprite{};
sprite.textureId = "player_ship";     // Loaded by RenderManager
sprite.srcX = 0; sprite.srcY = 0;
sprite.srcW = 64; sprite.srcH = 64;   // 64x64 sprite
sprite.dstX = 100; sprite.dstY = 200;
sprite.dstW = 64; sprite.dstH = 64;
sprite.layer = 10;                    // Render on layer 10

ecs->addComponent(player, sprite);
```

### 2. TextComponent

For rendering text with TrueType fonts:

```cpp
struct TextComponent : public GameEngine::IComponent {
    std::string fontId;              // Font identifier
    std::string text;                // Text to render

    std::uint8_t colorR = 255;       // Text color
    std::uint8_t colorG = 255;
    std::uint8_t colorB = 255;
    std::uint8_t alpha = 255;

    int layer = 0;
    bool needsUpdate = true;         // Regenerate texture when text changes
};
```

**Example Usage:**
```cpp
EntityId scoreText = ecs->createEntity();
ecs->addComponent(scoreText, TransformComponent{10.0F, 10.0F, 0.0F});

TextComponent text{};
text.fontId = "main_font_24";        // Loaded as "main_font_24"
text.text = "Score: 1000";
text.colorR = 255; text.colorG = 255; text.colorB = 255;
text.layer = 100;                    // Render UI on top

ecs->addComponent(scoreText, text);

// Update text later
auto& score = ecs->getComponent<TextComponent>(scoreText);
score.text = "Score: 1500";
score.needsUpdate = true;            // Tells system to regenerate texture
```

### 3. RectangleComponent

For filled rectangles:

```cpp
struct RectangleComponent : public GameEngine::IComponent {
    float width, height;

    std::uint8_t colorR = 255;
    std::uint8_t colorG = 255;
    std::uint8_t colorB = 255;
    std::uint8_t alpha = 255;

    bool filled = true;
    int layer = 0;
};
```

**Note:** Position is determined by `TransformComponent`. Add both components to the same entity:

```cpp
EntityId rect = ecs->createEntity();
ecs->addComponent(rect, TransformComponent{100.0F, 200.0F, 0.0F});  // Position
ecs->addComponent(rect, RectangleComponent{50.0F, 30.0F});          // Width x Height
```

### 4. CircleComponent

For filled circles (via SDL3_gfx):

```cpp
struct CircleComponent : public GameEngine::IComponent {
    float radius;

    std::uint8_t colorR = 255;
    std::uint8_t colorG = 255;
    std::uint8_t colorB = 255;
    std::uint8_t alpha = 255;

    bool filled = true;
    int layer = 0;
};
```

**Note:** Position is determined by `TransformComponent`. Add both components to the same entity:

```cpp
EntityId circle = ecs->createEntity();
ecs->addComponent(circle, TransformComponent{400.0F, 300.0F, 0.0F});  // Position
ecs->addComponent(circle, CircleComponent{25.0F});                     // Radius
```

### 5. TransformComponent

Position, rotation, and scaling:

```cpp
struct TransformComponent : public GameEngine::IComponent {
    float x = 0.0F;
    float y = 0.0F;
    float rotation = 0.0F;  // Rotation in degrees
    float scaleX = 1.0F;
    float scaleY = 1.0F;

    // Constructor
    TransformComponent(float posX, float posY, float rot = 0.0F,
                       float sX = 1.0F, float sY = 1.0F);
};
```

---

## Render Manager

The `RenderManager` class manages textures and fonts:

### Texture Loading

**Recommended: Event-Based Approach**

```cpp
// Load textures via events (preferred)
ecs->publishEvent(GameEngine::LoadTextureEvent("player_ship", "assets/player.png"));
ecs->publishEvent(GameEngine::LoadTextureEvent("background", "assets/background.jpg"));
```

**Alternative: Direct RenderManager Access**

```cpp
// Access RenderManager directly (advanced usage)
auto& rendererComp = ecs->getComponent<RendererComponent>(rendererEntity);
auto* renderMgr = static_cast<RenderManager*>(rendererComp.renderManager);

// Requires SDL_Renderer parameter
SDL_Renderer* renderer = /* get from RendererComponent */;
SDL_Texture* texture = renderMgr->loadTexture(renderer, "player_ship", "assets/player.png");
```

**Texture Caching:**
- Textures loaded once, reused by texture ID
- `hasTexture(id)` checks if already loaded
- `getTexture(id)` retrieves cached texture

### Font Loading

**Recommended: Event-Based Approach**

```cpp
// Load fonts via events (preferred)
ecs->publishEvent(GameEngine::LoadFontEvent("title_72", "assets/OpenSans.ttf", 72));
ecs->publishEvent(GameEngine::LoadFontEvent("ui_24", "assets/OpenSans.ttf", 24));
```

---

## Rendering Pipeline

### Update Loop

The RenderSystem executes every frame:

```cpp
void RenderSystem::update(EcsManager& ecs, float deltaTime) {
    1. Clear screen
    2. Query all renderable entities (Sprite, Text, Rectangle, Circle)
    3. Sort by layer (lowest to highest)
    4. Render each entity type
    5. Present frame to screen
}
```

### Layering System

Entities are rendered by **layer**:

```cpp
// Background layer
sprite.layer = 0;

// Game objects layer
sprite.layer = 10;

// Effects layer
sprite.layer = 50;

// UI layer
sprite.layer = 100;
```

**Rendering order:** `layer 0` → `layer 10` → `layer 50` → `layer 100`

---

## Events

### RenderEvent

Published each frame before rendering:

```cpp
struct RenderEvent {
    float deltaTime;
};

// Subscribe to render event
ecs->subscribeEvent<RenderEvent>([](const RenderEvent& event) {
    // Pre-render logic
});
```

### WindowResizedEvent

Published when window is resized:

```cpp
struct WindowResizedEvent {
    int width;
    int height;
};

ecs->subscribeEvent<WindowResizedEvent>([](const WindowResizedEvent& event) {
    std::cout << "Window resized to " << event.width << "x" << event.height << "\n";
});
```

---

## Example: Complete Rendering Setup

```cpp
#include <GameEngine/EcsManager.hpp>
#include <GameEngine/RenderComponents.hpp>
#include <GameEngine/RenderSystem.hpp>

int main() {
    auto ecs = std::make_unique<GameEngine::EcsManager>();

    // Load assets via events
    ecs->publishEvent(GameEngine::LoadTextureEvent("player", "assets/player.png"));
    ecs->publishEvent(GameEngine::LoadFontEvent("font_32", "assets/font.ttf", 32));

    // Create player sprite
    EntityId player = ecs->createEntity();
    ecs->addComponent(player, TransformComponent{400.0F, 300.0F, 0.0F});

    SpriteComponent sprite{};
    sprite.textureId = "player";
    sprite.srcX = 0; sprite.srcY = 0;
    sprite.srcW = 64; sprite.srcH = 64;
    sprite.dstX = 400; sprite.dstY = 300;
    sprite.dstW = 64; sprite.dstH = 64;
    sprite.layer = 10;
    ecs->addComponent(player, sprite);

    // Create score text
    EntityId score = ecs->createEntity();
    ecs->addComponent(score, TransformComponent{10.0F, 10.0F, 0.0F});

    TextComponent scoreText{};
    scoreText.fontId = "font_32";
    scoreText.text = "Score: 0";
    scoreText.layer = 100;  // Render on top
    ecs->addComponent(score, scoreText);

    // Game loop
    bool running = true;
    while (running) {
        ecs->update(deltaTime);  // RenderSystem automatically renders
    }

    return 0;
}
```

---

## Troubleshooting

### Sprite Not Rendering

1. **Check alpha:** `sprite.alpha > 0`
2. **Check layer:** Lower layers render first
3. **Check texture loaded:** Verify `renderMgr->hasTexture(textureId)`
4. **Check position:** Ensure entity has `TransformComponent` if needed
5. **Check component added:** Verify sprite component was added to entity

### Text Not Updating

Set `needsUpdate = true` when changing text:

```cpp
auto& text = ecs->getComponent<TextComponent>(entity);
text.text = "New Text";
text.needsUpdate = true;  // ← Important!
```

### Performance Issues

- **Too many texture loads:** Cache textures, don't reload each frame
- **Too many layers:** Minimize layer count for better batching
- **Large textures:** Use appropriate texture sizes

---

## Advanced Features

### Color Modulation

Change sprite colors at runtime:

```cpp
auto& sprite = ecs->getComponent<SpriteComponent>(entity);
sprite.colorR = 255;  // Red tint
sprite.colorG = 0;
sprite.colorB = 0;
```

### Transparency

Fade entities in/out:

```cpp
sprite.alpha = 128;  // 50% transparent
```

### Scaling

Scale sprites via Transform:

```cpp
auto& transform = ecs->getComponent<TransformComponent>(entity);
transform.scaleX = 2.0F;  // 2x wider
transform.scaleY = 2.0F;  // 2x taller
```

---

**Last Updated:** November 2024
**System Version:** RenderSystem 2.0
