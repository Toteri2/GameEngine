/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** LevelSystem
*/

#pragma once

#include <GameEngine/EcsManager.hpp>
#include "LevelTypes.hpp"
#include "PrefabManager.hpp"
#include <memory>
#include <vector>

namespace GameEngine
{
    class LevelSystem {
      public:
        explicit LevelSystem(PrefabManager &prefabManager);
        ~LevelSystem() = default;

        auto update(EcsManager &ecs, float deltaTime) -> void;
        auto startLevel(std::unique_ptr<LevelData> levelData) -> bool;
        auto stopLevel() -> void;

        auto setEntityCreationCallback(EntityCreationCallback callback) -> void;
        auto isLevelActive() const -> bool;
        auto getCurrentTime() const -> float;
        auto getLevelProgress() const -> float;
        auto isLevelComplete() const -> bool;

      private:
        PrefabManager &_prefabManager;
        std::unique_ptr<LevelData> _currentLevel;
        std::vector<ScheduledSpawn> _scheduledSpawns;
        EntityCreationCallback _entityCreationCallback;

        float _currentTime;
        bool _levelActive;
        size_t _completedWaves;

        auto scheduleSpawns() -> void;
        auto processSpawns(EcsManager &ecs, float deltaTime) -> void;
        auto createEntityFromSpawn(EcsManager &ecs, const SpawnPoint &spawn) -> EntityId;
        auto mergePrefabWithOverrides(const Prefab *prefab, const ComponentData &overrides)
            -> ComponentData;
    };
} // namespace GameEngine
