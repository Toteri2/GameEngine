/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** AudioManager implementation
*/

#include "AudioManager.hpp"
#include <iostream>
#include <ranges>

namespace GameEngine
{
    namespace Audio
    {
        AudioManager::AudioManager()
        {
            if (!initialize()) {
                throw ("[AudioManager] Failed to initialize\n");
            }
        }

        AudioManager::~AudioManager()
        {
            cleanup();
        }

        auto AudioManager::initialize() -> bool
        {
            if (ma_engine_init(nullptr, &_engine) != MA_SUCCESS) {
                fprintf(stderr, "[AudioManager] Failed to initialize\n");
                return false;
            }
            _initialized = true;
            return true;
        }

        auto AudioManager::loadSound(const std::string &id, const std::string &filepath) -> void
        {
            if (!_initialized)
                return;

            SoundPool pool;
            pool.filepath = filepath;

            for (size_t i = 0; i < SoundPool::INITIAL_POOL_SIZE; ++i) {
                auto sound = std::make_unique<ma_sound>();
                if (ma_sound_init_from_file(&_engine, filepath.c_str(),
                    MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
                    nullptr, nullptr, sound.get()) != MA_SUCCESS) {
                    fprintf(stderr, "[AudioManager] Failed to load sound '%s'\n", filepath.c_str());
                    return;
                }
                pool.instances.push_back(std::move(sound));
            }

            _sounds[id] = std::move(pool);
        }

        auto AudioManager::loadMusic(const std::string &id, const std::string &filepath) -> void
        {
            if (!_initialized)
                return;

            if (_music.contains(id)) {
                fprintf(stderr, "[AudioManager] Music with ID '%s' is already loaded\n", id.c_str());
                return;
            }

            auto music = std::make_unique<ma_sound>();
            if (ma_sound_init_from_file(&_engine, filepath.c_str(), MA_SOUND_FLAG_STREAM, nullptr, nullptr, music.get()) != MA_SUCCESS) {
                fprintf(stderr, "[AudioManager] Failed to load music '%s'\n", filepath.c_str());
                return;
            }
            ma_sound_set_looping(music.get(), MA_TRUE);
            _music[id] = std::move(music);
        }

        auto AudioManager::playSound(const std::string &id, const float volume) -> void
        {
            if (!_initialized)
                return;

            const auto it = _sounds.find(id);
            if (it == _sounds.end())
                return;

            auto &[filepath, instances, nextInstance] = it->second;
            if (instances.empty())
                return;

            ma_sound *sound = nullptr;
            const size_t startIndex = nextInstance;

            for (size_t i = 0; i < instances.size(); ++i) {
                size_t const index = (startIndex + i) % instances.size();
                ma_sound *candidate = instances[index].get();

                if (!ma_sound_is_playing(candidate)) {
                    sound = candidate;
                    nextInstance = (index + 1) % instances.size();
                    break;
                }
            }

            if (sound == nullptr && instances.size() < SoundPool::MAX_POOL_SIZE) {
                std::lock_guard const lock(_poolMutex);

                const size_t oldSize = instances.size();
                const size_t newSize = std::min(oldSize * 2, SoundPool::MAX_POOL_SIZE);

                for (size_t i = oldSize; i < newSize; ++i) {
                    auto newSound = std::make_unique<ma_sound>();
                    if (ma_sound_init_from_file(&_engine, filepath.c_str(),
                        MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
                        nullptr, nullptr, newSound.get()) == MA_SUCCESS) {
                        instances.push_back(std::move(newSound));
                    } else {
                        fprintf(stderr, "[AudioManager] Failed to grow pool for '%s'\n", id.c_str());
                        break;
                    }
                }

                if (instances.size() > oldSize) {
                    fprintf(stderr, "[AudioManager] Grew sound pool '%s': %zu → %zu instances\n",
                            id.c_str(), oldSize, instances.size());
                    sound = instances[oldSize].get();
                    nextInstance = oldSize + 1;
                }
            }

            if (sound == nullptr) {
                sound = instances[nextInstance].get();
                nextInstance = (nextInstance + 1) % instances.size();
                fprintf(stderr, "[AudioManager] Warning: All %zu instances of '%s' are busy, reusing oldest\n",
                        instances.size(), id.c_str());
            }

            const float finalVolume = volume * _sfxVolume * _masterVolume;
            ma_sound_set_volume(sound, finalVolume);
            ma_sound_seek_to_pcm_frame(sound, 0);
            ma_sound_start(sound);
        }

        auto AudioManager::playMusic(const std::string &id, const bool loop) -> void
        {
            if (_currentMusic == id && ma_sound_is_playing(_music[id].get()) != 0U) {
                return;
            }

            if (!_currentMusic.empty() && _music.contains(_currentMusic)) {
                ma_sound_stop(_music[_currentMusic].get());
            }

            const auto it = _music.find(id);
            if (it != _music.end()) {
                ma_sound_set_looping(it->second.get(), loop ? MA_TRUE : MA_FALSE);
                ma_sound_set_volume(it->second.get(), _musicVolume * _masterVolume);
                ma_sound_seek_to_pcm_frame(it->second.get(), 0);
                ma_sound_start(it->second.get());
                _currentMusic = id;
            }
        }

        auto AudioManager::stopMusic() -> void
        {
            if (_currentMusic.empty() || !_music.contains(_currentMusic))
                return;
            ma_sound_stop(_music[_currentMusic].get());
            _currentMusic.clear();
        }

        auto AudioManager::setMasterVolume(const float volume) -> void
        {
            _masterVolume = volume;
            ma_engine_set_volume(&_engine, volume);
        }

        auto AudioManager::setSFXVolume(const float volume) -> void
        {
            _sfxVolume = volume;
        }

        auto AudioManager::setMusicVolume(const float volume) -> void
        {
            _musicVolume = volume;
            if (!_currentMusic.empty() && _music.contains(_currentMusic)) {
                ma_sound_set_volume(_music[_currentMusic].get(), _masterVolume * _musicVolume);
            }
        }

        auto AudioManager::isMusicPlaying() const -> bool
        {
            if (_currentMusic.empty() || !_music.contains(_currentMusic))
                return false;
            return ma_sound_is_playing(_music.at(_currentMusic).get());
        }

        auto AudioManager::getCurrentMusic() const -> const std::string &
        {
            return _currentMusic;
        }

        auto AudioManager::cleanup() -> void
        {
            if (!_initialized)
                return;

            for (auto& [id, pool] : _sounds) {
                for (auto& sound : pool.instances) {
                    ma_sound_uninit(sound.get());
                }
            }
            _sounds.clear();

            for (auto &music : _music | std::views::values) {
                ma_sound_uninit(music.get());
            }
            _music.clear();

            ma_engine_uninit(&_engine);
            _initialized = false;
        }
    } // namespace Audio
} // namespace GameEngine
