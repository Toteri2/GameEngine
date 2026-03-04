/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** PrefabManager
*/

#pragma once

#include "LevelTypes.hpp"
#include <unordered_map>
#include <memory>
#include <optional>
#include <string>

namespace GameEngine
{
    class PrefabManager {
      public:
        PrefabManager() = default;
        ~PrefabManager() = default;

        auto loadPrefab(const std::string &filePath) -> bool;
        auto loadPrefabsFromFolder(const std::string &folderPath) -> int;
        auto getPrefab(const std::string &prefabId) const -> const Prefab *;
        auto hasPrefab(const std::string &prefabId) const -> bool;
        auto getAllPrefabIds() const -> std::vector<std::string>;
        auto clear() -> void;

        auto getComponentData(const std::string &prefabId, const std::string &componentName) const
            -> std::optional<std::string>;

      private:
        std::unordered_map<std::string, Prefab> _prefabs;
    };
} // namespace GameEngine
