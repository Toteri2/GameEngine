/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** LevelSystem
*/

#include "GameEngine/LevelSystem.hpp"
#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>

GameEngine::LevelSystem::LevelSystem(PrefabManager &prefabManager)
    : _prefabManager(prefabManager), _currentLevel(nullptr), _entityCreationCallback(nullptr),
      _currentTime(0.0F), _levelActive(false), _completedWaves(0)
{}

auto GameEngine::LevelSystem::startLevel(std::unique_ptr<LevelData> levelData) -> bool
{
    if (!levelData) {
        std::cerr << "[LevelSystem] Cannot start null level" << std::endl;
        return false;
    }

    stopLevel();

    _currentLevel = std::move(levelData);
    _currentTime = 0.0F;
    _completedWaves = 0;
    _levelActive = true;

    scheduleSpawns();

    std::cout << "[LevelSystem] Started level: " << _currentLevel->meta.name << " ("
              << _scheduledSpawns.size() << " spawns scheduled)" << std::endl;

    return true;
}

auto GameEngine::LevelSystem::stopLevel() -> void
{
    _currentLevel = nullptr;
    _scheduledSpawns.clear();
    _currentTime = 0.0F;
    _levelActive = false;
    _completedWaves = 0;
}

auto GameEngine::LevelSystem::update(EcsManager &ecs, float deltaTime) -> void
{
    if (!_levelActive || !_currentLevel) {
        return;
    }

    _currentTime += deltaTime;
    processSpawns(ecs, deltaTime);

    size_t completedCount = 0;
    for (auto &wave : _currentLevel->waves) {
        if (wave.completed) {
            completedCount++;
            continue;
        }

        if (wave.startTime > _currentTime) {
            continue;
        }

        bool allSpawned = true;
        for (const auto &spawn : wave.spawns) {
            float spawnTime = wave.startTime + spawn.delay;
            if (spawnTime > _currentTime) {
                allSpawned = false;
                break;
            }
        }
        if (allSpawned) {
            wave.completed = true;
            completedCount++;
        }
    }
    _completedWaves = completedCount;
}

auto GameEngine::LevelSystem::setEntityCreationCallback(EntityCreationCallback callback) -> void
{
    _entityCreationCallback = callback;
}

auto GameEngine::LevelSystem::isLevelActive() const -> bool
{
    return _levelActive;
}

auto GameEngine::LevelSystem::getCurrentTime() const -> float
{
    return _currentTime;
}

auto GameEngine::LevelSystem::getLevelProgress() const -> float
{
    if (!_currentLevel || _currentLevel->waves.empty()) {
        return 0.0F;
    }

    return static_cast<float>(_completedWaves) / static_cast<float>(_currentLevel->waves.size());
}

auto GameEngine::LevelSystem::isLevelComplete() const -> bool
{
    if (!_currentLevel) {
        return false;
    }

    return _completedWaves >= _currentLevel->waves.size() && _scheduledSpawns.empty();
}

auto GameEngine::LevelSystem::scheduleSpawns() -> void
{
    _scheduledSpawns.clear();

    if (!_currentLevel) {
        return;
    }

    for (const auto &wave : _currentLevel->waves) {
        for (const auto &spawn : wave.spawns) {
            ScheduledSpawn scheduled;
            scheduled.triggerTime = wave.startTime + spawn.delay;
            scheduled.spawnPoint = spawn;
            scheduled.triggered = false;
            _scheduledSpawns.push_back(scheduled);
        }
    }

    std::sort(_scheduledSpawns.begin(), _scheduledSpawns.end(),
        [](const ScheduledSpawn &a, const ScheduledSpawn &b) {
            return a.triggerTime < b.triggerTime;
        });
}

auto GameEngine::LevelSystem::processSpawns(EcsManager &ecs, float deltaTime) -> void
{
    auto it = _scheduledSpawns.begin();
    while (it != _scheduledSpawns.end()) {
        if (!it->triggered && it->triggerTime <= _currentTime) {
            createEntityFromSpawn(ecs, it->spawnPoint);
            it->triggered = true;
            it = _scheduledSpawns.erase(it);
        } else {
            ++it;
        }
    }
}

auto GameEngine::LevelSystem::createEntityFromSpawn(EcsManager &ecs, const SpawnPoint &spawn)
    -> EntityId
{
    const Prefab *prefab = _prefabManager.getPrefab(spawn.prefabId);
    if (!prefab) {
        std::cerr << "[LevelSystem] Failed to find prefab: " << spawn.prefabId << std::endl;
        return 0;
    }

    EntityId entity = ecs.createEntity();
    ComponentData mergedData = mergePrefabWithOverrides(prefab, spawn.componentOverrides);

    nlohmann::json posData;
    posData["x"] = spawn.x;
    posData["y"] = spawn.y;
    mergedData["_spawn_position"] = posData.dump();

    if (_entityCreationCallback) {
        _entityCreationCallback(entity, mergedData);
    }

    std::cout << "[LevelSystem] Spawned entity " << entity << " from prefab " << spawn.prefabId
              << " at (" << spawn.x << ", " << spawn.y << ")" << std::endl;

    return entity;
}

auto GameEngine::LevelSystem::mergePrefabWithOverrides(
    const Prefab *prefab, const ComponentData &overrides) -> ComponentData
{
    if (!prefab) {
        return overrides;
    }

    ComponentData merged = prefab->components;

    for (const auto &[key, value] : overrides) {
        try {
            std::string prefabJson = "";
            if (merged.find(key) != merged.end()) {
                prefabJson = std::any_cast<std::string>(merged[key]);
            }

            std::string overrideJson = std::any_cast<std::string>(value);

            if (prefabJson.empty()) {
                merged[key] = overrideJson;
            } else {
                nlohmann::json prefabData = nlohmann::json::parse(prefabJson);
                nlohmann::json overrideData = nlohmann::json::parse(overrideJson);
                prefabData.update(overrideData);
                merged[key] = prefabData.dump();
            }
        } catch (const std::exception &e) {
            std::cerr << "[LevelSystem] Error merging component " << key << ": " << e.what()
                      << std::endl;
        }
    }

    return merged;
}
