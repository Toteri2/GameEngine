/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** AudioSystem
*/

#include "AudioSystem.hpp"
#include <iostream>

GameEngine::Audio::AudioSystem::AudioSystem() : _isInitialized(true)
{
    std::cout << "[AudioSystem] Initialized" << '\n';
}

GameEngine::Audio::AudioSystem::~AudioSystem()
{
    std::cout << "[AudioSystem] Destroyed" << '\n';
}

auto GameEngine::Audio::AudioSystem::update(EcsManager &ecs, float  /*deltaTime*/) -> void
{
    static bool eventsSubscribed = false;

    if (!eventsSubscribed) {
        ecs.subscribeEvent<LoadMusicEvent>(
            [this](const LoadMusicEvent &event) {
                _audioManager.loadMusic(event._id, event._filepath);
            });
        ecs.subscribeEvent<PlayMusicEvent>(
            [this](const PlayMusicEvent &event) {
                _audioManager.playMusic(event._id, event._loop);
            });
        ecs.subscribeEvent<StopMusicEvent>(
            [this](const StopMusicEvent &event) {
                _audioManager.stopMusic();
            });
        ecs.subscribeEvent<LoadSoundEvent>(
            [this](const LoadSoundEvent &event) {
                _audioManager.loadSound(event._id, event._filepath);
            });
        ecs.subscribeEvent<PlaySoundEvent>(
            [this](const PlaySoundEvent &event) {
                _audioManager.playSound(event._id, event._volume);
            });
        ecs.subscribeEvent<SetMasterVolumeEvent>(
            [this](const SetMasterVolumeEvent &event) {
                _audioManager.setMasterVolume(event._volume);
            });
        ecs.subscribeEvent<SetSFXVolumeEvent>(
            [this](const SetSFXVolumeEvent &event) {
                _audioManager.setSFXVolume(event._volume);
            });
        ecs.subscribeEvent<SetMusicVolumeEvent>(
            [this](const SetMusicVolumeEvent &event) {
                _audioManager.setMusicVolume(event._volume);
            });
        ecs.subscribeEvent<IsMusicPlayingEvent>(
            [this](const IsMusicPlayingEvent &event) {
                _audioManager.isMusicPlaying();
            });
        ecs.subscribeEvent<GetCurrentMusic>(
            [this](const GetCurrentMusic &event) {
                _audioManager.getCurrentMusic();
            });
        eventsSubscribed = true;
    }

    if (!_isInitialized) {
        return;
    }
}
