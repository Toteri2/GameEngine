/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** LoggerSystem implementation
*/

#include "LoggerSystem.hpp"

namespace GameEngine
{
    namespace Logger
    {
        LoggerSystem::LoggerSystem()
        {
            _logger.info("LoggerSystem initialized");
        }

        auto LoggerSystem::update(EcsManager &ecs, const float deltaTime) -> void
        {
            (void) ecs;
            (void) deltaTime;
        }

    } // namespace Logger
} // namespace GameEngine
