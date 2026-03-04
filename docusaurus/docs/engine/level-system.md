---
sidebar_position: 5
---

# Level System

JSON-driven level loading with prefab system, wave management, and dynamic entity spawning.

## Overview

**Key Features:**
-  JSON-based level configuration
-  Prefab system (entity templates)
-  Wave-based progression
-  Boss spawning support
-  Runtime entity instantiation
-  No hardcoded level data

**Location:** `engine/src/Systems/LevelSystem/`

---

## Quick Start

### 1. Define a Prefab

`assets/prefabs/enemy_basic.json`:
```json
{
  "name": "BasicEnemy",
  "components": {
    "Position": { "x": 0, "y": 0 },
    "Velocity": { "x": -100, "y": 0 },
    "Health": { "current": 50, "maximum": 50 },
    "Enemy": { "type": "BASIC", "scoreValue": 100 },
    "Sprite": {
      "textureId": "enemy_basic",
      "width": 32,
      "height": 32
    }
  }
}
```

### 2. Define a Level

`assets/levels/level_1.json`:
```json
{
  "name": "Level 1",
  "waves": [
    {
      "waveId": 1,
      "spawnInterval": 2.0,
      "enemyPrefab": "BasicEnemy",
      "count": 10,
      "spawnPattern": "sequential"
    },
    {
      "waveId": 2,
      "spawnInterval": 1.5,
      "enemyPrefab": "FastEnemy",
      "count": 15
    }
  ],
  "boss": {
    "prefab": "BydoBoss",
    "spawnAfterWave": 2
  }
}
```

### 3. Load Level

```cpp
// Register LevelSystem
ecs->registerSystem<GameEngine::LevelSystem>(*ecs);

// Load level
GameEngine::LoadLevelEvent loadEvent("assets/levels/level_1.json");
ecs->publishEvent(loadEvent);
```

---

## Architecture

### Key Components

| Component | Purpose |
|-----------|---------|
| **LevelLoader** | Parses JSON, creates entities |
| **PrefabManager** | Stores/instantiates entity templates |
| **ServerLevelSystem** | Manages wave progression (server-side) |

---

## Prefab System

### Prefab Structure

```json
{
  "name": "PrefabName",
  "components": {
    "ComponentType": { "field": value },
    "AnotherComponent": { ... }
  }
}
```

### Loading Prefabs

```cpp
// Load prefab library
GameEngine::LoadPrefabsEvent loadPrefabs("assets/prefabs/");
ecs->publishEvent(loadPrefabs);
```

### Instantiating Prefabs

```cpp
// Create entity from prefab
GameEngine::SpawnPrefabEvent spawn("BasicEnemy", 800.0F, 300.0F);
ecs->publishEvent(spawn);
```

---

## Level Configuration

### Wave Definition

```json
{
  "waveId": 1,
  "spawnInterval": 2.0,     // Seconds between spawns
  "enemyPrefab": "BasicEnemy",
  "count": 10,              // How many to spawn
  "spawnPattern": "sequential",
  "spawnPositions": [
    {"x": 800, "y": 100},
    {"x": 800, "y": 300},
    {"x": 800, "y": 500}
  ]
}
```

### Boss Configuration

```json
{
  "boss": {
    "prefab": "BydoBoss",
    "spawnAfterWave": 3,    // After wave 3 completes
    "health": 5000,
    "scoreValue": 10000
  }
}
```

---

## Wave Management (Server)

The `ServerLevelSystem` handles wave progression:

```cpp
class ServerLevelSystem : public ISystem {
    void update(EcsManager& ecs, float deltaTime) override {
        // 1. Check if current wave complete
        // 2. Spawn next enemy if interval elapsed
        // 3. Transition to next wave when count reached
        // 4. Spawn boss after final wave
    }
};
```

**Wave States:**
1. `SPAWNING` - Spawning enemies at intervals
2. `WAITING` - All spawned, waiting for clear
3. `COMPLETE` - Wave finished, ready for next
4. `BOSS` - Boss active

---

## Example: Complete Level

`assets/levels/stage_1.json`:
```json
{
  "name": "Stage 1: Asteroid Field",
  "backgroundColor": "#001122",
  "music": "assets/sounds/first_stage.mp3",

  "waves": [
    {
      "waveId": 1,
      "enemyPrefab": "BasicEnemy",
      "count": 8,
      "spawnInterval": 2.5,
      "spawnPattern": "top_to_bottom"
    },
    {
      "waveId": 2,
      "enemyPrefab": "SineWaveEnemy",
      "count": 6,
      "spawnInterval": 3.0,
      "spawnPattern": "random"
    },
    {
      "waveId": 3,
      "enemyPrefab": "TankEnemy",
      "count": 4,
      "spawnInterval": 4.0,
      "spawnPattern": "sequential"
    }
  ],

  "boss": {
    "prefab": "BydoBoss",
    "spawnAfterWave": 3,
    "musicOverride": "assets/sounds/boss.mp3"
  }
}
```

---

## Prefab Examples

### Basic Enemy
```json
{
  "name": "BasicEnemy",
  "components": {
    "Position": { "x": 800, "y": 360 },
    "Velocity": { "x": -120, "y": 0 },
    "Health": { "current": 30, "maximum": 30 },
    "Enemy": {
      "type": "BASIC",
      "scoreValue": 100,
      "shootInterval": 3.0
    },
    "Collider": {
      "width": 32,
      "height": 32,
      "layer": "ENEMY"
    },
    "Sprite": {
      "textureId": "enemy_basic",
      "width": 32,
      "height": 32,
      "layer": 10
    }
  }
}
```

### Boss Enemy
```json
{
  "name": "BydoBoss",
  "components": {
    "Position": { "x": 700, "y": 300 },
    "Velocity": { "x": 0, "y": 0 },
    "Health": { "current": 5000, "maximum": 5000 },
    "Enemy": {
      "type": "BOSS",
      "scoreValue": 50000,
      "shootInterval": 0.5,
      "movementPattern": "hover"
    },
    "Sprite": {
      "textureId": "boss_bydo",
      "width": 128,
      "height": 128,
      "layer": 10
    },
    "Script": {
      "scriptPath": "scripts/boss_ai.lua"
    }
  }
}
```

---

## Dynamic Spawning

### Spawn at Specific Position

```cpp
// Spawn enemy at coordinates
GameEngine::SpawnPrefabEvent spawn("BasicEnemy", 800.0F, 200.0F);
ecs->publishEvent(spawn);
```

### Spawn with Overrides

```cpp
// Spawn with custom properties
SpawnPrefabEvent spawn("BasicEnemy", 800.0F, 200.0F);
spawn.overrides["Health.current"] = 100.0F;  // Tougher enemy
spawn.overrides["Velocity.x"] = -200.0F;     // Faster enemy
ecs->publishEvent(spawn);
```

---

## Integration Example

```cpp
// Server main.cpp
int main() {
    auto ecs = std::make_unique<EcsManager>();

    // Register systems
    ecs->registerSystem<LevelSystem>(*ecs);
    ecs->registerSystem<ServerLevelSystem>(*ecs);
    ecs->registerSystem<EnemySpawnSystem>(*ecs);

    // Load prefabs
    LoadPrefabsEvent loadPrefabs("assets/prefabs/");
    ecs->publishEvent(loadPrefabs);

    // Load level
    LoadLevelEvent loadLevel("assets/levels/level_1.json");
    ecs->publishEvent(loadLevel);

    // Game loop
    while (running) {
        ecs->update(deltaTime);
        // Wave system spawns enemies automatically
    }
}
```

---

## Best Practices

- **Separate prefabs by type** (one file per enemy type)
- **Use descriptive names** (BasicEnemy, not enemy1)
- **Test levels progressively** (easy → hard)
- **Reuse prefabs** across levels
- **Version control** JSON files

---

## Troubleshooting

### Prefab Not Loading
- Check JSON syntax (use validator)
- Verify file path relative to executable
- Ensure component types are registered

### Enemies Not Spawning
- Check `ServerLevelSystem` is registered
- Verify wave `spawnInterval` > 0
- Check enemy count > 0

### Boss Not Appearing
- Ensure all waves completed
- Check `spawnAfterWave` matches wave count
- Verify boss prefab exists

---

**Last Updated:** November 2025

**System Version:** LevelSystem 1.0
