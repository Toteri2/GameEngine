/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** AudioSystem - Public API for ECS Audio subsystem
*/

#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace GameEngine
{
    namespace Audio
    {
        class LoadMusicEvent {
            public:
                const std::string _id;
                const std::string _filepath;

                LoadMusicEvent(const std::string &id, const std::string &filepath) : _id(id), _filepath(filepath) {}
            };
        
        class PlayMusicEvent {
            public:
                const std::string _id;
                bool _loop;

                PlayMusicEvent(const std::string &id, bool loop = true) : _id(id), _loop(loop) {}
            };

        class StopMusicEvent {
            public:
                StopMusicEvent() = default;
            };

        class LoadSoundEvent {
            public:
                const std::string _id;
                const std::string _filepath;

                LoadSoundEvent(const std::string &id, const std::string &filepath) : _id(id), _filepath(filepath) {}
            };
        
        class PlaySoundEvent {
            public:
                const std::string _id;
                float _volume;

                PlaySoundEvent(const std::string &id, float volume = 1.0F) : _id(id), _volume(volume) {}
            };

        class SetMasterVolumeEvent {
            public:
                float _volume;

                SetMasterVolumeEvent(float volume = 1.0F) : _volume(volume) {}
            };

        class SetSFXVolumeEvent {
            public:
                float _volume;

                SetSFXVolumeEvent(float volume = 1.0F) : _volume(volume) {}
            };

        class SetMusicVolumeEvent {
            public:
                float _volume;

                SetMusicVolumeEvent(float volume = 1.0F) : _volume(volume) {}
            };

        class IsMusicPlayingEvent {
            public:
                IsMusicPlayingEvent() = default;
            };

        class GetCurrentMusic {
            public:
                GetCurrentMusic() = default;
            };
    }
} // namespace GameEngine
