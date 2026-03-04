/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** EngineBuilder
*/

#pragma once

#include "../../src/Core/EcsManager.hpp"
#include <memory>
#include <utility>

#ifdef GAMEENGINE_HAS_LOGGER_SYSTEM
    #include "../../src/Systems/LoggerSystem/LoggerSystem.hpp"
#endif

#ifdef GAMEENGINE_HAS_AUTH_SYSTEM
    #include "../../src/Systems/Auth/AuthSystem.hpp"
#endif

#ifdef GAMEENGINE_HAS_AUDIO_SYSTEM
    #include "../../src/Systems/AudioSystem/AudioSystem.hpp"
#endif

#ifdef GAMEENGINE_HAS_NETWORK_SYSTEM
    #include "../../src/Systems/NetworkSystem/NetworkSystem.hpp"
#endif

#ifdef GAMEENGINE_HAS_INPUT_SYSTEM
    #include "../../src/Systems/InputSystem/InputSystem.hpp"
#endif

#ifdef GAMEENGINE_HAS_RENDER_SYSTEM
    #include "../../src/Systems/RenderSystem/RenderSystem.hpp"
#endif

#ifdef GAMEENGINE_HAS_SCRIPTING_SYSTEM
    #include "../../src/Systems/ScriptingSystem/ScriptingSystem.hpp"
#endif

#ifdef GAMEENGINE_HAS_PHYSICS_SYSTEM
    #include "../../src/Systems/PhysicsSystem/PhysicsSystem.hpp"
#endif

#ifdef GAMEENGINE_HAS_CONSOLE_SYSTEM
    #include "../../src/Systems/DeveloperConsoleSystem/DeveloperConsoleSystem.hpp"
#endif

namespace GameEngine
{
    class EngineBuilder {
      public:
        static auto create() -> std::unique_ptr<EcsManager>
        {
            auto ecs = std::make_unique<EcsManager>();

#ifdef GAMEENGINE_HAS_LOGGER_SYSTEM
            ecs->registerSystem<Logger::LoggerSystem>();
#endif

#ifdef GAMEENGINE_HAS_SCRIPTING_SYSTEM
            ecs->registerSystem<Scripting::ScriptingSystem>(*ecs);
#endif

#ifdef GAMEENGINE_HAS_NETWORK_SYSTEM
            ecs->registerSystem<NetworkSystem>();
#endif

#ifdef GAMEENGINE_HAS_INPUT_SYSTEM
            ecs->registerSystem<InputSystem>(*ecs);
#endif

#ifdef GAMEENGINE_HAS_RENDER_SYSTEM
            ecs->registerSystem<RenderSystem>(*ecs);
#endif

#ifdef GAMEENGINE_HAS_AUDIO_SYSTEM
            ecs->registerSystem<Audio::AudioSystem>();
#endif

#ifdef GAMEENGINE_HAS_PHYSICS_SYSTEM
            ecs->registerSystem<PhysicsSystem>();
#endif

#ifdef GAMEENGINE_HAS_CONSOLE_SYSTEM
            ecs->registerSystem<DeveloperConsoleSystem>(*ecs);
#endif

            return ecs;
        }
    };
} // namespace GameEngine
