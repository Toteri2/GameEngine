# Level Editor

## Overview

The R-Type Level Editor is an Electron-based desktop application that provides a visual interface for creating and editing game levels. It features a canvas-based editor with camera controls, wave-based enemy spawning, and a comprehensive prefab management system integrated with the game engine's ECS architecture.

## Features

### Core Capabilities

- **Visual Level Design**: Interactive 1920x1080 canvas (16:9 aspect ratio) for placing enemy spawn points
- **Wave-Based Spawning**: Create multiple enemy waves with precise timing and positioning
- **Prefab Management**: Use pre-defined enemies or create custom prefabs with full component editing
- **Camera Controls**: Pan with middle mouse button, zoom with mouse wheel (0.5x to 3x)
- **Dual Edit Modes**: Visual form editor and raw JSON editor for prefabs
- **Cross-Platform**: Builds for Linux (AppImage, .deb) and Windows (.exe)

## Getting Started

### Installation

```bash
cd editor
npm install
```

### Running the Editor

```bash
npm start
```

### Building Distributables

```bash
npm run dist
```

This creates platform-specific installers in the `editor/dist/` directory.

## Architecture

### Application Structure

```
editor/
├── package.json             # NPM configuration and dependencies
├── src/
│   ├── main.js              # Electron main process (backend)
│   ├── preload.js           # IPC bridge between main and renderer
│   └── renderer/
│       ├── index.html       # UI structure
│       ├── renderer.js      # Frontend logic (canvas editor)
│       └── style.css        # Styling
```

### Process Architecture

The editor uses Electron's multi-process architecture:

- **Main Process** (`src/main.js`): Handles window management, file system operations, and IPC handlers
- **Renderer Process** (`src/renderer/`): Manages the UI, canvas rendering, and user interactions
- **Preload Script** (`src/preload.js`): Provides secure IPC communication via `contextBridge`

## User Interface

### Canvas Editor

The main canvas provides a visual representation of your level:

- **Resolution**: 1920x1080 pixels (matches game resolution)
- **Spawn Area Overlay**: Green rectangle showing valid enemy spawn region (default: x:1024, y:0, width:256, height:768)
- **Grid**: Visual reference for positioning

#### Camera Controls

| Input | Action |
|-------|--------|
| Middle Mouse + Drag | Pan camera |
| Mouse Wheel | Zoom in/out (0.5x - 3x) |

### Panels

#### Level Metadata Panel
Configure level information:
- **Name**: Level display name
- **Description**: Level description
- **Author**: Creator name

#### Wave Management Panel
Create and organize enemy waves:
- **Add Wave**: Create a new wave with unique ID and start time
- **Wave List**: View all waves with their spawn counts
- **Delete Wave**: Remove waves from the level

#### Prefab Manager Panel
Manage enemy prefabs:
- **Pre-defined Prefabs**: Use existing enemies (basic, sine_wave, tank, shooter, boss, bydo_boss)
- **Custom Prefabs**: Create new prefabs with custom properties
- **Edit Modes**: Switch between visual form editor and JSON editor

## Level Structure

### Level JSON Format

```json
{
  "meta": {
    "version": 1,
    "name": "Level Name",
    "description": "A challenging level",
    "author": "Author Name"
  },
  "spawn_area": {
    "x": 1024,
    "y": 0,
    "width": 256,
    "height": 768
  },
  "waves": [
    {
      "id": "wave_1",
      "start_time": 0,
      "spawns": [
        {
          "prefab": "enemy_basic",
          "position": { "x": 1200, "y": 400 },
          "delay": 0
        }
      ]
    }
  ]
}
```

### Field Descriptions

#### Meta
- `version`: Format version (currently 1)
- `name`: Level display name
- `description`: Level description
- `author`: Creator name

#### Spawn Area
Defines the valid region for enemy spawning:
- `x`, `y`: Top-left corner position
- `width`, `height`: Area dimensions

#### Waves
Array of enemy waves:
- `id`: Unique wave identifier
- `start_time`: When the wave begins (in seconds from level start)
- `spawns`: Array of spawn points

#### Spawn Points
Individual enemy spawn definitions:
- `prefab`: Reference to prefab ID
- `position`: `{x, y}` coordinates on the level
- `delay`: Spawn delay (in seconds after wave starts)

## Prefab System

### Pre-defined Prefabs

The engine includes several pre-built enemy prefabs in `assets/prefabs/`:

| Prefab ID | Description |
|-----------|-------------|
| `enemy_basic` | Basic horizontal-moving enemy |
| `enemy_sine_wave` | Enemy with sine wave movement pattern |
| `enemy_tank` | Tank enemy with higher health |
| `enemy_shooter` | Enemy that shoots projectiles |
| `enemy_boss` | Boss enemy |
| `enemy_bydo_boss` | Special boss variant |

### Prefab JSON Structure

```json
{
  "id": "enemy_basic",
  "components": {
    "Position": { "x": 0, "y": 0 },
    "Velocity": { "vx": -100, "vy": 0 },
    "Health": { "health": 30 },
    "CollideRect": {
      "width": 55,
      "height": 55,
      "offsetX": 20,
      "offsetY": 27.5
    },
    "Enemy": {
      "enemyType": 0,
      "aiTimer": 0,
      "shootCooldown": 0
    },
    "Weapon": {
      "fireRate": 0,
      "bulletSpeed": 0,
      "damage": 0,
      "canShoot": false,
      "bulletRadius": 0
    },
    "Sprite": {
      "spriteId": 1,
      "texturePath": "assets/Enemies/basic.png",
      "frameCount": 8,
      "frameWidth": 33,
      "frameHeight": 34,
      "animationSpeed": 8.0
    },
    "MovementPattern": {
      "patternType": 3,
      "frequency": 2.0,
      "amplitude": 80,
      "baseVelocityX": -100
    }
  }
}
```

### Component Types

#### Position
- `x`: X-coordinate
- `y`: Y-coordinate

#### Velocity
- `vx`: Horizontal velocity
- `vy`: Vertical velocity

#### Health
- `health`: Hit points

#### CollideRect
Collision box dimensions:
- `width`, `height`: Collision box size
- `offsetX`, `offsetY`: Offset from position

#### Enemy
AI-related properties:
- `enemyType`: Type identifier
- `aiTimer`: AI update timer
- `shootCooldown`: Time between shots

#### Weapon
Shooting capabilities:
- `fireRate`: Shots per second
- `bulletSpeed`: Projectile velocity
- `damage`: Damage per shot
- `canShoot`: Enable shooting
- `bulletRadius`: Projectile collision radius

#### Sprite
Visual representation:
- `spriteId`: Unique sprite identifier
- `texturePath`: Path to texture file (relative to project root)
- `frameCount`: Number of animation frames
- `frameWidth`, `frameHeight`: Frame dimensions in pixels
- `animationSpeed`: Frames per second

#### MovementPattern
Movement behavior:
- `patternType`: Pattern identifier (0=none, 3=sine wave, etc.)
- `frequency`: Oscillation frequency
- `amplitude`: Movement amplitude
- `baseVelocityX`: Base horizontal velocity

### Creating Custom Prefabs

1. Click "Create Custom Prefab" in the Prefab Manager panel
2. Enter a unique ID and choose a color (for visual identification)
3. Edit components using either:
   - **Visual Editor**: Form with inputs for each field
   - **JSON Editor**: Direct JSON editing for advanced users
4. Click "Save Prefab" to save to `assets/prefabs/`

## Editing Workflow

### Creating a New Level

1. Click "New Level"
2. Fill in metadata (name, description, author)
3. Create waves with "Add Wave"
4. Select a prefab from the dropdown
5. Click on the canvas to place spawn points
6. Adjust spawn point properties in the details panel
7. Export the level with "Export Level"

### Editing Spawn Points

#### Adding Spawn Points
1. Select a wave from the wave list
2. Choose a prefab from the dropdown
3. Click on the canvas to place the spawn point

#### Moving Spawn Points
1. Click and drag a spawn point to reposition
2. Release to set the new position

#### Editing Properties
1. Click on a spawn point to select it
2. Edit properties in the spawn point details panel:
   - Prefab reference
   - Position (x, y)
   - Spawn delay

#### Deleting Spawn Points
1. Select a spawn point by clicking on it
2. Press the **Delete** key

### Importing Existing Levels

1. Click "Import Level"
2. Navigate to `assets/levels/` and select a level JSON file
3. The editor loads all waves, spawn points, and metadata

## Engine Integration

### Data Flow

```
Editor (Electron App)
    - exports
Level JSON (assets/levels/*.json)
    - loaded by
LevelLoader (C++ Engine)
    - parses into
LevelData Structure
    - references
Prefab JSON (assets/prefabs/*.json)
    - loaded by
PrefabManager (C++ Engine)
    - instantiates
Game Entities (ECS Components)
```

### PrefabManager

Located in `engine/src/Systems/LevelSystem/PrefabManager.cpp`

#### Key Methods

```cpp
// Load a single prefab from file
void loadPrefab(const std::string& filePath);

// Load all prefabs from a directory
void loadPrefabsFromFolder(const std::string& folderPath);

// Retrieve a prefab by ID
const Prefab& getPrefab(const std::string& prefabId) const;

// Check if a prefab exists
bool hasPrefab(const std::string& prefabId) const;
```

The PrefabManager:
- Loads all `.json` files from `assets/prefabs/`
- Parses component data into C++ structures
- Provides fast lookup by prefab ID
- Stores components as JSON for flexible deserialization

### LevelLoader

Located in `engine/src/Systems/LevelSystem/LevelLoader.cpp`

#### Key Methods

```cpp
// Load a complete level from file
LevelData loadLevel(const std::string& filePath);

// Parse individual sections
void parseLevelMeta(const json& j);
void parseSpawnArea(const json& j);
void parseWaves(const json& j);
void parseSpawnPoint(const json& spawnJson);
```

The LevelLoader:
- Loads level JSON files
- Validates prefab references against PrefabManager
- Parses waves and spawn points
- Creates `LevelData` structure for runtime use

### Data Structures

Defined in `engine/src/Systems/LevelSystem/LevelTypes.hpp`

```cpp
struct SpawnPoint {
    float x, y;              // Position
    float delay;             // Spawn delay in seconds
    std::string prefabId;    // Reference to prefab
    ComponentData componentOverrides;  // Optional overrides
};

struct Wave {
    std::string id;                    // Wave identifier
    float startTime;                   // Start time in seconds
    std::vector<SpawnPoint> spawns;    // Spawn points
    bool completed;                    // Runtime completion flag
};

struct LevelData {
    LevelMeta meta;              // Level metadata
    SpawnArea spawnArea;         // Valid spawn region
    std::vector<Wave> waves;     // All waves
};

struct Prefab {
    std::string id;              // Prefab identifier
    ComponentData components;    // ECS components
};
```

## Build Configuration

### package.json

```json
{
  "name": "level-editor",
  "version": "1.0.0",
  "description": "R-Type editor",
  "author": "Axel BATTIGELLI",
  "main": "src/main.js",
  "scripts": {
    "start": "electron .",
    "dist": "electron-builder"
  },
  "devDependencies": {
    "electron": "^33.0.0",
    "electron-builder": "^26.0.12"
  },
  "build": {
    "appId": "com.rtype.rtypeeditor",
    "productName": "RType Editor",
    "linux": {
      "target": ["AppImage", "deb"],
      "category": "Utility"
    },
    "win": {
      "target": ["nsis"]
    }
  }
}
```

### CI/CD Pipeline

The editor has an automated build pipeline in `.github/workflows/build_editor.yml`:

- **Platforms**: Linux (AppImage, .deb) and Windows (.exe)
- **Container**: Ubuntu with Wine for cross-compilation
- **Artifacts**: Uploaded as GitHub Actions artifacts

#### Build Steps
1. Install Node.js 20+
2. Install npm dependencies
3. Run `electron-builder` with Wine for Windows
4. Upload AppImage and .exe artifacts

## File Locations

### Editor Application
- Main entry: `editor/src/main.js`
- Renderer: `editor/src/renderer/renderer.js`
- UI: `editor/src/renderer/index.html`

### Data Files
- Levels: `assets/levels/*.json`
- Prefabs: `assets/prefabs/*.json`

### Engine Integration
- PrefabManager: `engine/src/Systems/LevelSystem/PrefabManager.cpp`
- LevelLoader: `engine/src/Systems/LevelSystem/LevelLoader.cpp`
- Type definitions: `engine/src/Systems/LevelSystem/LevelTypes.hpp`

## Tips and Best Practices

### Level Design

1. **Start Simple**: Begin with a single wave and test in-game before adding complexity
2. **Use Spawn Delays**: Stagger enemy spawns within a wave for more interesting patterns
3. **Visual Balance**: Use the canvas to ensure spawn points are well-distributed
4. **Test Spawn Area**: Keep enemies within the green spawn area overlay for proper spawning

### Prefab Creation

1. **Start from Existing**: Duplicate and modify existing prefabs rather than starting from scratch
2. **Test Movement Patterns**: Pattern type 3 (sine wave) requires proper frequency and amplitude values
3. **Match Sprite Dimensions**: Ensure `CollideRect` matches your sprite's visual size
4. **Use Descriptive IDs**: Name prefabs clearly (e.g., `enemy_fast_shooter` instead of `enemy1`)

### Performance Considerations

1. **Limit Spawn Density**: Too many enemies at once can impact performance
2. **Optimize Sprites**: Use appropriate texture sizes and frame counts
3. **Balance Wave Timing**: Space out waves to avoid overwhelming the system

## Troubleshooting

### Common Issues

**Issue**: Prefab not appearing in dropdown
- **Solution**: Ensure the prefab JSON is in `assets/prefabs/` and has a valid `id` field

**Issue**: Spawn points not visible on canvas
- **Solution**: Check that the selected wave is active and the prefab has a valid color

**Issue**: Level fails to load in game
- **Solution**: Validate JSON structure and ensure all referenced prefabs exist

**Issue**: Exported level file is empty
- **Solution**: Ensure you have at least one wave with spawn points before exporting

### Validation Checklist

Before exporting a level, verify:
- [ ] All waves have unique IDs
- [ ] All prefab references exist in `assets/prefabs/`
- [ ] Spawn points are within the spawn area (if using strict validation)
- [ ] Wave start times are in ascending order
- [ ] No negative spawn delays
- [ ] Level metadata is filled in

---

**Last Updated:** November 2025

**Version:** Editor 1.0
