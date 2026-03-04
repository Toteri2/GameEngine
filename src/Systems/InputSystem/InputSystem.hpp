/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** InputSystem - ECS integrated input system
*/

#pragma once

#include "InputManager.hpp"
#include <GameEngine/EcsManager.hpp>
#include <memory>

namespace GameEngine
{
    class InputSystem final : public ISystem {
      public:
        explicit InputSystem(EcsManager &ecsManager);
        ~InputSystem() override = default;

        auto update(EcsManager &world, float deltaTime) -> void override;

      private:
        std::unique_ptr<InputManager> _inputManager;
        bool _bindingsSetup = false;
    };

} // namespace GameEngine
