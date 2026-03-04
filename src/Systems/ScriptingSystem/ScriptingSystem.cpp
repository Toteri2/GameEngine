/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** ScriptingSystem
*/

#include "ScriptingSystem.hpp"

namespace GameEngine::Scripting
{

    ScriptingSystem::ScriptingSystem(EcsManager &ecs)
    {
        _scriptManager = std::make_unique<ScriptManager>(ecs);
    }

    ScriptingSystem::~ScriptingSystem() {}

    auto ScriptingSystem::update(EcsManager &ecs, float deltaTime) -> void
    {
        const auto entities = ecs.getAllEntitiesWith<ScriptComponent>();

        for (EntityId const entity : entities) {
            auto &scriptComp = ecs.getComponent<ScriptComponent>(entity);

            if (scriptComp.path.has_value()) {
                ScriptID const loadedId = _scriptManager->loadScript(*scriptComp.path);

                if (loadedId == INVALID_SCRIPT_ID) {
                    fprintf(stderr, "[ScriptingSystem] Failed to load script '%s' for entity %u\n",
                        scriptComp.path->c_str(), entity);
                    scriptComp.path.reset();
                } else {
                    scriptComp.scriptId = loadedId;
                    scriptComp.path.reset();
                }
            }

            if (!_trackedEntities.contains(entity) && scriptComp.scriptId != INVALID_SCRIPT_ID) {
                if (!_scriptManager->attachScript(entity, scriptComp.scriptId)) {
                    fprintf(stderr, "[ScriptingSystem] Failed to attach script %u to entity %u\n",
                        scriptComp.scriptId, entity);
                } else {
                    if (scriptComp.autoInit) {
                        _scriptManager->initializeEntity(entity);
                    }
                    _trackedEntities.insert(entity);
                }
            }
        }

        std::vector<EntityId> toRemove;
        for (EntityId const tracked : _trackedEntities) {
            if (!ecs.hasComponent<ScriptComponent>(tracked)) {
                _scriptManager->destroyEntity(tracked);
                _scriptManager->detachScript(tracked);
                toRemove.push_back(tracked);
            }
        }
        for (EntityId const entity : toRemove) {
            _trackedEntities.erase(entity);
        }
        _scriptManager->updateAll(deltaTime);
    }

    auto ScriptingSystem::getScriptManager() -> ScriptManager &
    {
        return *_scriptManager;
    }

    auto ScriptingSystem::getScriptManager() const -> const ScriptManager &
    {
        return *_scriptManager;
    }

} // namespace GameEngine::Scripting
