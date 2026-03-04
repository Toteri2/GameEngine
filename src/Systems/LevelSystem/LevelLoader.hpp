/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** LevelLoader
*/

#pragma once

#include "LevelTypes.hpp"
#include "PrefabManager.hpp"
#include <memory>
#include <string>

namespace GameEngine
{
    class LevelLoader {
      public:
        explicit LevelLoader(PrefabManager &prefabManager);
        ~LevelLoader() = default;

        auto loadLevel(const std::string &filePath) -> std::unique_ptr<LevelData>;
        auto getLastError() const -> const std::string &;

      private:
        PrefabManager &_prefabManager;
        std::string _lastError;

        auto parseLevelMeta(const void *json) -> LevelMeta;
        auto parseSpawnArea(const void *json) -> SpawnArea;
        auto parseWaves(const void *json) -> std::vector<Wave>;
        auto parseSpawnPoint(const void *json) -> SpawnPoint;
        auto parseComponentData(const void *json) -> ComponentData;
    };
} // namespace GameEngine
