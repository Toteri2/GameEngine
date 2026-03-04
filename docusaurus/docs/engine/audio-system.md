---
sidebar_position: 6
---

# Audio System

Cross-platform audio via miniaudio with sound effect pooling and music playback.

## Overview

**Key Features:**
-  miniaudio backend (cross-platform)
-  Sound effect pooling (multiple simultaneous sounds)
-  Music playback with looping
-  Volume control
-  Event-driven API
-  MP3, WAV, OGG support

**Location:** `engine/src/Systems/AudioSystem/`

---

## Quick Start

### 1. Setup Audio System

```cpp
// Register AudioSystem
ecs->registerSystem<GameEngine::AudioSystem>(*ecs);
```

### 2. Load Audio

```cpp
// Load sound effect
GameEngine::Audio::LoadSoundEvent loadSound("laser", "assets/sounds/laser.wav");
ecs->publishEvent(loadSound);

// Load music
GameEngine::Audio::LoadMusicEvent loadMusic("menu", "assets/sounds/menu.mp3");
ecs->publishEvent(loadMusic);
```

### 3. Play Audio

```cpp
// Play sound effect
GameEngine::Audio::PlaySoundEvent playSound("laser", 1.0F);  // volume 1.0
ecs->publishEvent(playSound);

// Play music (looping)
GameEngine::Audio::PlayMusicEvent playMusic("menu", true);  // loop = true
ecs->publishEvent(playMusic);
```

---

## Audio Events

### LoadSoundEvent
```cpp
struct LoadSoundEvent {
    std::string soundId;    // Identifier
    std::string filePath;   // Path to audio file
};

// Example
LoadSoundEvent load("explosion", "assets/sounds/explosion.wav");
ecs->publishEvent(load);
```

### PlaySoundEvent
```cpp
struct PlaySoundEvent {
    std::string soundId;
    float volume = 1.0F;    // 0.0 - 1.0
};

// Example
PlaySoundEvent play("explosion", 0.8F);  // 80% volume
ecs->publishEvent(play);
```

### LoadMusicEvent
```cpp
struct LoadMusicEvent {
    std::string musicId;
    std::string filePath;
};

// Example
LoadMusicEvent load("boss", "assets/sounds/boss.mp3");
ecs->publishEvent(load);
```

### PlayMusicEvent
```cpp
struct PlayMusicEvent {
    std::string musicId;
    bool loop = true;
};

// Example
PlayMusicEvent play("boss", true);  // Loop
ecs->publishEvent(play);
```

### StopMusicEvent
```cpp
struct StopMusicEvent {};

// Example
StopMusicEvent stop;
ecs->publishEvent(stop);  // Stop current music
```

### SetMusicVolumeEvent
```cpp
struct SetMusicVolumeEvent {
    float volume;  // 0.0 - 1.0
};

// Example
SetMusicVolumeEvent setVol(0.5F);  // 50% volume
ecs->publishEvent(setVol);
```

---

## Sound Pooling

The AudioSystem maintains **instance pools** for sound effects, allowing multiple simultaneous plays:

```cpp
// Fire laser 3 times rapidly
for (int i = 0; i < 3; i++) {
    PlaySoundEvent play("laser", 1.0F);
    ecs->publishEvent(play);
}
// All 3 sounds play simultaneously!
```

**Pool Size:** Configurable per sound (default: 5 instances)

---

## Example: Complete Audio Setup

```cpp
// Game initialization
void setupAudio(EcsManager* ecs) {
    // Load UI sounds
    ecs->publishEvent(LoadSoundEvent("click", "assets/sounds/click.wav"));
    ecs->publishEvent(LoadSoundEvent("hover", "assets/sounds/hover.wav"));

    // Load game sounds
    ecs->publishEvent(LoadSoundEvent("laser", "assets/sounds/laser.wav"));
    ecs->publishEvent(LoadSoundEvent("explosion", "assets/sounds/explosion.wav"));
    ecs->publishEvent(LoadSoundEvent("powerup", "assets/sounds/powerup.wav"));

    // Load music tracks
    ecs->publishEvent(LoadMusicEvent("menu", "assets/sounds/main_menu.mp3"));
    ecs->publishEvent(LoadMusicEvent("game", "assets/sounds/first_stage.mp3"));
    ecs->publishEvent(LoadMusicEvent("boss", "assets/sounds/boss.mp3"));

    // Start menu music
    ecs->publishEvent(PlayMusicEvent("menu", true));
}
```

---

## Music Management

### Playing Music

```cpp
// Play menu music
PlayMusicEvent play("menu", true);  // Loop
ecs->publishEvent(play);
```

### Stopping Music

```cpp
// Stop current music
StopMusicEvent stop;
ecs->publishEvent(stop);
```

### Changing Tracks

```cpp
// Stop current, play new
ecs->publishEvent(StopMusicEvent());
ecs->publishEvent(PlayMusicEvent("game", true));
```

### Volume Control

```cpp
// Set volume to 70%
SetMusicVolumeEvent setVol(0.7F);
ecs->publishEvent(setVol);

// Mute
SetMusicVolumeEvent mute(0.0F);
ecs->publishEvent(mute);
```

---

## Game Integration Examples

### Menu System

```cpp
// Button click
if (buttonClicked) {
    PlaySoundEvent click("click", 1.0F);
    ecs->publishEvent(click);
}

// Transition to game
StopMusicEvent stopMenu;
ecs->publishEvent(stopMenu);

LoadMusicEvent loadGame("game", "assets/sounds/first_stage.mp3");
ecs->publishEvent(loadGame);

PlayMusicEvent playGame("game", true);
ecs->publishEvent(playGame);
```

### Weapon System

```cpp
// Player shoots
void onPlayerShoot() {
    PlaySoundEvent fire("laser", 0.8F);
    ecs->publishEvent(fire);
}

// Enemy destroyed
void onEnemyDeath() {
    PlaySoundEvent boom("explosion", 1.0F);
    ecs->publishEvent(boom);
}
```

### Pause System

```cpp
// Game paused
void onPause() {
    SetMusicVolumeEvent fadeOut(0.3F);  // Reduce volume
    ecs->publishEvent(fadeOut);
}

// Game resumed
void onResume() {
    SetMusicVolumeEvent fadeIn(1.0F);   // Restore volume
    ecs->publishEvent(fadeIn);
}
```

---

## Supported Formats

| Format | Extension | Use Case |
|--------|-----------|----------|
| **WAV** | .wav | Sound effects (uncompressed, best for short sounds) |
| **MP3** | .mp3 | Music (compressed, good for long tracks) |
| **OGG** | .ogg | Music (compressed, open format) |

**Recommendation:**
- Sound effects: `.wav` (fast loading)
- Music: `.mp3` or `.ogg` (smaller file size)

---

## Performance Tips

- **Preload audio** during loading screens
- **Use short loops** for music (reduces memory)
- **Limit pool size** (don't need 100 laser instances)
- **Compress music** (MP3/OGG)
- **Keep sounds under 1s** for effects

---

## Troubleshooting

### Sound Not Playing
- Check file path relative to executable
- Verify sound loaded with `LoadSoundEvent` first
- Check volume > 0
- Verify audio file format supported

### Music Not Looping
- Ensure `loop = true` in `PlayMusicEvent`
- Check music file is valid

### Audio Crackling
- Reduce pool size
- Check system audio settings
- Use lower sample rate audio files

---

**Last Updated:** November 2025

**System Version:** AudioSystem 1.0
