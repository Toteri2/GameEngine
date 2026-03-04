/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** LevelLoader
*/

#include "GameEngine/LevelLoader.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

GameEngine::LevelLoader::LevelLoader(PrefabManager &prefabManager)
    : _prefabManager(prefabManager), _lastError("")
{}

auto GameEngine::LevelLoader::loadLevel(const std::string &filePath) -> std::unique_ptr<LevelData>
{
    _lastError.clear();

    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            _lastError = "Failed to open level file: " + filePath;
            std::cerr << "[LevelLoader] " << _lastError << std::endl;
            return nullptr;
        }

        nlohmann::json json;
        file >> json;

        auto levelData = std::make_unique<LevelData>();

        if (json.contains("meta")) {
            levelData->meta = parseLevelMeta(&json["meta"]);
        }

        if (json.contains("spawn_area")) {
            levelData->spawnArea = parseSpawnArea(&json["spawn_area"]);
        }

        if (json.contains("waves")) {
            levelData->waves = parseWaves(&json["waves"]);
        }

        std::cout << "[LevelLoader] Loaded level: " << levelData->meta.name << " ("
                  << levelData->waves.size() << " waves)" << std::endl;

        return levelData;

    } catch (const std::exception &e) {
        _lastError = std::string("Error loading level: ") + e.what();
        std::cerr << "[LevelLoader] " << _lastError << std::endl;
        return nullptr;
    }
}

auto GameEngine::LevelLoader::getLastError() const -> const std::string &
{
    return _lastError;
}

auto GameEngine::LevelLoader::parseLevelMeta(const void *jsonPtr) -> LevelMeta
{
    const auto &json = *static_cast<const nlohmann::json *>(jsonPtr);

    LevelMeta meta;
    meta.version = json.value("version", 1);
    meta.name = json.value("name", "Unnamed Level");

    return meta;
}

auto GameEngine::LevelLoader::parseSpawnArea(const void *jsonPtr) -> SpawnArea
{
    const auto &json = *static_cast<const nlohmann::json *>(jsonPtr);

    SpawnArea area;
    area.x = json.value("x", 0.0F);
    area.y = json.value("y", 0.0F);
    area.width = json.value("width", 1920.0F);
    area.height = json.value("height", 1080.0F);

    return area;
}

auto GameEngine::LevelLoader::parseWaves(const void *jsonPtr) -> std::vector<Wave>
{
    const auto &json = *static_cast<const nlohmann::json *>(jsonPtr);
    std::vector<Wave> waves;

    if (!json.is_array()) {
        return waves;
    }

    for (const auto &waveJson : json) {
        Wave wave;
        wave.id = waveJson.value("id", "wave_" + std::to_string(waves.size()));
        wave.startTime = waveJson.value("start_time", 0.0F);

        if (waveJson.contains("spawns") && waveJson["spawns"].is_array()) {
            for (const auto &spawnJson : waveJson["spawns"]) {
                SpawnPoint spawn = parseSpawnPoint(&spawnJson);

                if (!_prefabManager.hasPrefab(spawn.prefabId)) {
                    std::cerr << "[LevelLoader] Warning: Unknown prefab '" << spawn.prefabId
                              << "' in wave " << wave.id << std::endl;
                    continue;
                }

                wave.spawns.push_back(std::move(spawn));
            }
        }

        waves.push_back(std::move(wave));
    }

    return waves;
}

auto GameEngine::LevelLoader::parseSpawnPoint(const void *jsonPtr) -> SpawnPoint
{
    const auto &json = *static_cast<const nlohmann::json *>(jsonPtr);

    SpawnPoint spawn;
    spawn.prefabId = json.value("prefab", "");
    spawn.delay = json.value("delay", 0.0F);

    if (json.contains("position")) {
        const auto &pos = json["position"];
        spawn.x = pos.value("x", 0.0F);
        spawn.y = pos.value("y", 0.0F);
    }

    if (json.contains("components")) {
        spawn.componentOverrides = parseComponentData(&json["components"]);
    }

    return spawn;
}

auto GameEngine::LevelLoader::parseComponentData(const void *jsonPtr) -> ComponentData
{
    const auto &json = *static_cast<const nlohmann::json *>(jsonPtr);
    ComponentData data;

    if (!json.is_object()) {
        return data;
    }

    for (auto &[key, value] : json.items()) {
        data[key] = value.dump();
    }

    return data;
}
