/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** AudioSystem - ECS integrated audio system
*/

#pragma once

#include <GameEngine/EcsManager.hpp>
#include <GameEngine/AudioEventTypes.hpp>
#include "../../Utils/Audio/AudioManager.hpp"
#include <miniaudio.h>
#include <unordered_map>
#include <memory>
#include <string>

namespace GameEngine
{
    namespace Audio
    {
        class AudioSystem final : public ISystem {
            public:
                AudioSystem();
                ~AudioSystem() override;

                auto update(EcsManager &ecs, float deltaTime) -> void override;

            private:
                bool _isInitialized;
                AudioManager _audioManager;
        };
    };
};
