/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** LevelTypes
*/

#pragma once

#include <unordered_map>
#include <GameEngine/EcsManager.hpp>
#include <any>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace GameEngine
{
    using ComponentData = std::unordered_map<std::string, std::any>;
    using EntityCreationCallback = std::function<void(EntityId, const ComponentData &)>;

    struct SpawnPoint {
        float x = 0.0F;
        float y = 0.0F;
        float delay = 0.0F;
        std::string prefabId;
        ComponentData componentOverrides;
    };

    struct Wave {
        std::string id;
        float startTime = 0.0F;
        std::vector<SpawnPoint> spawns;
        bool completed = false;
    };

    struct SpawnArea {
        float x = 0.0F;
        float y = 0.0F;
        float width = 1920.0F;
        float height = 1080.0F;
    };

    struct LevelMeta {
        int version = 1;
        std::string name;
    };

    struct LevelData {
        LevelMeta meta;
        SpawnArea spawnArea;
        std::vector<Wave> waves;
    };

    struct Prefab {
        std::string id;
        ComponentData components;
    };

    struct ScheduledSpawn {
        float triggerTime;
        SpawnPoint spawnPoint;
        bool triggered = false;
    };
} // namespace GameEngine
