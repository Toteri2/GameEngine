/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** AudioManager - ECS integrated audio system using miniaudio
*/

#pragma once

#include <GameEngine/EcsManager.hpp>
#include <miniaudio.h>
#include <unordered_map>
#include <memory>
#include <string>
#include <mutex>

namespace GameEngine
{
    namespace Audio
    {
        struct SoundPool {
            std::string filepath;
            std::vector<std::unique_ptr<ma_sound>> instances;
            size_t nextInstance = 0;
            static constexpr size_t INITIAL_POOL_SIZE = 4;
            static constexpr size_t MAX_POOL_SIZE = 64;
        };

        class AudioManager final {
          public:
            AudioManager();
            ~AudioManager();

            AudioManager(const AudioManager &) = delete;
            auto operator=(const AudioManager &) -> AudioManager & = delete;

            auto initialize() -> bool;
            auto loadSound(const std::string &id, const std::string &filepath) -> void;
            auto loadMusic(const std::string &id, const std::string &filepath) -> void;
            auto playSound(const std::string &id, float volume = 1.0F) -> void;
            auto playMusic(const std::string &id, bool loop = true) -> void;
            auto stopMusic() -> void;
            auto setMasterVolume(float volume) -> void;
            auto setSFXVolume(float volume) -> void;
            auto setMusicVolume(float volume) -> void;
            auto isMusicPlaying() const -> bool;
            auto getCurrentMusic() const -> const std::string &;

          private:
            auto cleanup() -> void;

            ma_engine _engine{};
            std::unordered_map<std::string, SoundPool> _sounds;
            std::unordered_map<std::string, std::unique_ptr<ma_sound>> _music;
            std::string _currentMusic;
            float _masterVolume = 1.0F;
            float _sfxVolume = 1.0F;
            float _musicVolume = 0.7F;
            bool _initialized = false;
            std::mutex _poolMutex;
        };
    } // namespace Audio
} // namespace GameEngine
