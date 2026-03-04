/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** PhysicsSystem - ECS integrated physics system
*/

#pragma once

#include "../../Utils/Physics/PhysicsManager.hpp"
#include <GameEngine/EcsManager.hpp>

namespace GameEngine
{
    class PhysicsSystem final : public ISystem {
      public:
        PhysicsSystem();
        ~PhysicsSystem() override;

        auto update(EcsManager &ecs, float deltaTime) -> void override;

      private:
        Physics::PhysicsManager _physicsManager;
    };
} // namespace GameEngine
