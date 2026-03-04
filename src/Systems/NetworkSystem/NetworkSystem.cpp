/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** NetworkSystem
*/

#include "NetworkSystem.hpp"
#include <iostream>

GameEngine::NetworkSystem::NetworkSystem()
{
    std::cout << "[NetworkSystem] Initialized" << '\n';
}

GameEngine::NetworkSystem::~NetworkSystem()
{
    std::cout << "[NetworkSystem] Destroyed" << '\n';
}

auto GameEngine::NetworkSystem::update(EcsManager &world, float deltaTime) -> void
{
    static bool eventsSubscribed = false;

    if (!eventsSubscribed) {
        world.subscribeEvent<StartNetworkEvent>(
            [this, &world](const StartNetworkEvent &event) { onStartNetwork(event, world); });
        world.subscribeEvent<StopNetworkEvent>(
            [this](const StopNetworkEvent &event) { onStopNetwork(event); });
        world.subscribeEvent<SendPunctualPacketEvent>(
            [this](const SendPunctualPacketEvent &event) { onSendPunctualPacket(event); });
        world.subscribeEvent<StartBurstPacketEvent>(
            [this](const StartBurstPacketEvent &event) { onStartBurstPacket(event); });
        world.subscribeEvent<StopBurstPacketEvent>(
            [this](const StopBurstPacketEvent &event) { onStopBurstPacket(event); });
        eventsSubscribed = true;
    }

    if (!_networkManager.isRunning()) {
        return;
    }

    processReceivedPackets(world);
    _networkManager.sendPunctualPackets();
    _networkManager.sendBurstPackets(deltaTime);
}

void GameEngine::NetworkSystem::onStartNetwork(const StartNetworkEvent &event, EcsManager &world)
{
    try {
        if (event.isServer) {
            _networkManager.startServer(event.port);
            std::cout << "[NetworkSystem] Started server on port " << event.port << '\n';
        } else {
            _networkManager.startClient(event.port, event.serverAddress);
            std::cout << "[NetworkSystem] Started client connecting to " << event.serverAddress
                      << ":" << event.port << '\n';
        }
    } catch (const std::exception &e) {
        std::cerr << "[NetworkSystem] Failed to start network: " << e.what() << '\n';
        std::string errorMsg = "Failed to start network: ";
        errorMsg += e.what();
        NetworkErrorEvent errorEvent(errorMsg, true);
        world.publishEvent(errorEvent);
    }
}

void GameEngine::NetworkSystem::onStopNetwork(const StopNetworkEvent & /*event*/)
{
    _networkManager.stop();
    std::cout << "[NetworkSystem] Network stopped" << '\n';
}

void GameEngine::NetworkSystem::onSendPunctualPacket(const SendPunctualPacketEvent &event)
{
    if (!_networkManager.isRunning()) {
        return;
    }

    if (event.targetClientId.empty()) {
        _networkManager.addPunctualPacket(event.data);
    } else {
        _networkManager.addTargetedPunctualPacket(event.targetClientId, event.data);
    }
}

void GameEngine::NetworkSystem::onStartBurstPacket(const StartBurstPacketEvent &event)
{
    if (!_networkManager.isRunning()) {
        return;
    }

    const std::string burstId = std::to_string(event.id);
    _networkManager.addBurstPacket(burstId, event.data, event.sendDelay, event.targetClientId);
}

void GameEngine::NetworkSystem::onStopBurstPacket(const StopBurstPacketEvent &event)
{
    std::string const burstId = std::to_string(event.id);
    _networkManager.removeBurstPacket(burstId);
}

void GameEngine::NetworkSystem::processReceivedPackets(EcsManager &world)
{
    NetworkManager::ReceivedMessage message;

    while (_networkManager.getNextMessage(message)) {
        if (_networkManager.isServer()) {
            _networkManager.addKnownClient(message.sender);
        }

        std::string const senderAddress = message.sender.address().to_string();
        std::uint16_t const senderPort = message.sender.port();

        ReceivedPacket const receivedPacket(message.data, senderAddress, senderPort);
        world.publishEvent(receivedPacket);

        std::cout << "[NetworkSystem] Processed packet from " << senderAddress << ":" << senderPort
                  << " (" << message.data.size() << " bytes)" << '\n';
    }
}
