/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** InputSystem - Implementation
*/

#include "InputSystem.hpp"
#include <iostream>

namespace GameEngine
{
    InputSystem::InputSystem(EcsManager &ecsManager)
        : _inputManager(std::make_unique<InputManager>(ecsManager))
    {
        std::cout << "[GameEngine::InputSystem] Initialized" << std::endl;
        
    }

    auto InputSystem::update(EcsManager & world, float deltaTime) -> void
    {
        _inputManager->update(deltaTime, world);
    }

} // namespace GameEngine
