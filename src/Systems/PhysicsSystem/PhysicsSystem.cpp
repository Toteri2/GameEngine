/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** PhysicsSystem
*/

#include "PhysicsSystem.hpp"
#include <iostream>

GameEngine::PhysicsSystem::PhysicsSystem()
{
    std::cout << "[PhysicsSystem] Initialized" << std::endl;
}

GameEngine::PhysicsSystem::~PhysicsSystem()
{
    std::cout << "[PhysicsSystem] Destroyed" << std::endl;
}

auto GameEngine::PhysicsSystem::update(EcsManager &ecs, float deltaTime) -> void
{
    static bool eventsSubscribed = false;

    if (!eventsSubscribed) {
        ecs.subscribeEvent<StartPhysicsEvent>(
            [this](const StartPhysicsEvent &event) {
                _physicsManager.onStartPhysics(event);
            });
        ecs.subscribeEvent<StopPhysicsEvent>(
            [this](const StopPhysicsEvent &event) {
                _physicsManager.onStopPhysics(event);
            });
        ecs.subscribeEvent<SetPhysicsGravityEvent>(
            [this](const SetPhysicsGravityEvent &event) {
                _physicsManager.onSetGravity(event);
            });
        eventsSubscribed = true;
    }

    if (!_physicsManager.isInitialized()) {
        return;
    }

    _physicsManager.syncComponentsToPhysics(ecs);

    _physicsManager.step(deltaTime);

    _physicsManager.syncPhysicsToComponents(ecs);

    _physicsManager.processCollisionsAndPublish(ecs);
}
