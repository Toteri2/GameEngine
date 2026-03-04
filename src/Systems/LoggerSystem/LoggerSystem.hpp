/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** LoggerSystem - ECS integrated logging system
*/

#pragma once

#include <GameEngine/EcsManager.hpp>
#include <GameEngine/Logger.hpp>

namespace GameEngine
{
    namespace Logger
    {
        class LoggerSystem final : public ISystem {
          public:
            LoggerSystem();
            ~LoggerSystem() override = default;

            auto update(EcsManager &ecs, float deltaTime) -> void override;

          private:
            LoggerManager _logger;
        };
    } // namespace Logger
} // namespace GameEngine
