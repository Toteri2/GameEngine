/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** ScriptingSystem - ECS integration for scripting
*/

#pragma once

#include "../../Core/ISystem.hpp"
#include "../../Scripting/ScriptComponent.hpp"
#include "../../Scripting/ScriptManager.hpp"
#include <unordered_set>

namespace GameEngine::Scripting
{

    class ScriptingSystem final : public ISystem {
      public:
        explicit ScriptingSystem(EcsManager &ecs);
        ~ScriptingSystem() override;

        auto update(EcsManager &ecs, float deltaTime) -> void override;

        auto getScriptManager() -> ScriptManager &;
        auto getScriptManager() const -> const ScriptManager &;

      private:
        std::unique_ptr<ScriptManager> _scriptManager;
        std::unordered_set<EntityId> _trackedEntities;
    };

} // namespace GameEngine::Scripting
