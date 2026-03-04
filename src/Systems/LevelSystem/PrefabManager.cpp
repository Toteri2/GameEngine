/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** PrefabManager
*/

#include "GameEngine/PrefabManager.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

auto GameEngine::PrefabManager::loadPrefab(const std::string &filePath) -> bool
{
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "[PrefabManager] Failed to open prefab file: " << filePath << std::endl;
            return false;
        }

        nlohmann::json json;
        file >> json;

        if (!json.contains("id")) {
            std::cerr << "[PrefabManager] Prefab missing 'id' field: " << filePath << std::endl;
            return false;
        }

        Prefab prefab;
        prefab.id = json["id"].get<std::string>();

        if (json.contains("components") && json["components"].is_object()) {
            for (auto &[key, value] : json["components"].items()) {
                prefab.components[key] = value.dump();
            }
        }

        _prefabs[prefab.id] = std::move(prefab);
        std::cout << "[PrefabManager] Loaded prefab: " << prefab.id << std::endl;
        return true;

    } catch (const std::exception &e) {
        std::cerr << "[PrefabManager] Error loading prefab " << filePath << ": " << e.what()
                  << std::endl;
        return false;
    }
}

auto GameEngine::PrefabManager::loadPrefabsFromFolder(const std::string &folderPath) -> int
{
    int loadedCount = 0;

    if (!std::filesystem::exists(folderPath)) {
        std::cerr << "[PrefabManager] Folder does not exist: " << folderPath << std::endl;
        return 0;
    }

    if (!std::filesystem::is_directory(folderPath)) {
        std::cerr << "[PrefabManager] Path is not a directory: " << folderPath << std::endl;
        return 0;
    }

    for (const auto &entry : std::filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            if (loadPrefab(entry.path().string())) {
                loadedCount++;
            }
        }
    }

    std::cout << "[PrefabManager] Loaded " << loadedCount << " prefabs from " << folderPath
              << std::endl;
    return loadedCount;
}

auto GameEngine::PrefabManager::getPrefab(const std::string &prefabId) const -> const Prefab *
{
    auto it = _prefabs.find(prefabId);
    if (it != _prefabs.end()) {
        return &it->second;
    }
    return nullptr;
}

auto GameEngine::PrefabManager::hasPrefab(const std::string &prefabId) const -> bool
{
    return _prefabs.find(prefabId) != _prefabs.end();
}

auto GameEngine::PrefabManager::getAllPrefabIds() const -> std::vector<std::string>
{
    std::vector<std::string> ids;
    ids.reserve(_prefabs.size());
    for (const auto &[id, prefab] : _prefabs) {
        ids.push_back(id);
    }
    return ids;
}

auto GameEngine::PrefabManager::clear() -> void
{
    _prefabs.clear();
}

auto GameEngine::PrefabManager::getComponentData(const std::string &prefabId,
    const std::string &componentName) const -> std::optional<std::string>
{
    const auto *prefab = getPrefab(prefabId);
    if (!prefab) {
        return std::nullopt;
    }

    auto it = prefab->components.find(componentName);
    if (it == prefab->components.end()) {
        return std::nullopt;
    }

    try {
        return std::any_cast<std::string>(it->second);
    } catch (const std::bad_any_cast &) {
        return std::nullopt;
    }
}
