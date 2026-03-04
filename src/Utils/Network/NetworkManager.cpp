/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** NetworkManager
*/

#include "NetworkManager.hpp"
#include <boost/asio/buffer.hpp>
#include <chrono>
#include <exception>
#include <iostream>

GameEngine::NetworkManager::NetworkManager() : _receiver(nullptr)
{
    std::cout << "[NetworkManager] Initialized" << '\n';
}

GameEngine::NetworkManager::~NetworkManager()
{
    stop();
    std::cout << "[NetworkManager] Destroyed" << '\n';
}

void GameEngine::NetworkManager::startServer(std::uint16_t port)
{
    if (_receiver && _receiver->isRunning()) {
        std::cout << "[NetworkManager] Already running, stopping previous instance" << '\n';
        stop();
    }

    try {
        _receiver = std::make_unique<NetworkReceiver>(port);
        _receiver->start();

        std::cout << "[NetworkManager] Server started on port " << port << '\n';
    } catch (const std::exception &e) {
        std::cerr << "[NetworkManager] Failed to start server: " << e.what() << '\n';
        _receiver = nullptr;
        throw;
    }
}

void GameEngine::NetworkManager::startClient(std::uint16_t port, const std::string &serverAddress)
{
    if (_receiver && _receiver->isRunning()) {
        std::cout << "[NetworkManager] Already running, stopping previous instance" << '\n';
        stop();
    }

    try {
        _receiver = std::make_unique<NetworkReceiver>(port, serverAddress);
        _receiver->start();

        std::cout << "[NetworkManager] Client started, connecting to " << serverAddress << ":"
                  << port << '\n';
    } catch (const std::exception &e) {
        std::cerr << "[NetworkManager] Failed to start client: " << e.what() << '\n';
        _receiver = nullptr;
        throw;
    }
}

void GameEngine::NetworkManager::stop()
{
    if (_receiver) {
        _receiver->stop();
        _receiver = nullptr;
    }

    {
        std::lock_guard const lock(_clientsMutex);
        _knownClients.clear();
        _clientEndpoints.clear();
    }
    {
        std::lock_guard const lock(_punctualPacketsMutex);
        _punctualPackets.clear();
        _targetedPunctualPackets.clear();
    }
    {
        std::lock_guard const lock(_burstPacketsMutex);
        _burstPackets.clear();
    }

    std::cout << "[NetworkManager] Stopped" << '\n';
}

bool GameEngine::NetworkManager::isRunning() const
{
    return _receiver && _receiver->isRunning();
}

bool GameEngine::NetworkManager::isServer() const
{
    return _receiver && _receiver->isServer();
}

bool GameEngine::NetworkManager::getNextMessage(ReceivedMessage &outMessage) const
{
    if (!_receiver) {
        return false;
    }
    return _receiver->getNextMessage(outMessage);
}

bool GameEngine::NetworkManager::hasMessages() const
{
    if (!_receiver) {
        return false;
    }
    return _receiver->hasMessages();
}

void GameEngine::NetworkManager::sendPacketToClient(
    const std::vector<std::uint8_t> &data, const boost::asio::ip::udp::endpoint &endpoint) const
{
    if (!_receiver || !_receiver->isRunning()) {
        return;
    }

    try {
        _receiver->getSocket().send_to(boost::asio::buffer(data), endpoint);
    } catch (const std::exception &e) {
        std::cerr << "[NetworkManager] Error sending packet to client: " << e.what() << '\n';
    }
}

void GameEngine::NetworkManager::sendPacketToAll(const std::vector<std::uint8_t> &data)
{
    if (!_receiver || !_receiver->isRunning() || !_receiver->isServer()) {
        return;
    }

    std::lock_guard const lock(_clientsMutex);
    for (const auto &[clientId, endpoint] : _clientEndpoints) {
        sendPacketToClient(data, endpoint);
    }
}

void GameEngine::NetworkManager::sendPacketToServer(const std::vector<std::uint8_t> &data) const
{
    if (!_receiver || !_receiver->isRunning() || _receiver->isServer()) {
        return;
    }

    try {
        _receiver->getSocket().send(boost::asio::buffer(data));
    } catch (const std::exception &e) {
        std::cerr << "[NetworkManager] Error sending packet to server: " << e.what() << '\n';
    }
}

void GameEngine::NetworkManager::addKnownClient(const boost::asio::ip::udp::endpoint &endpoint)
{
    std::lock_guard const lock(_clientsMutex);
    std::string const clientId = getClientId(endpoint);

    if (!_clientEndpoints.contains(clientId)) {
        _clientEndpoints[clientId] = endpoint;
        _knownClients[clientId] = endpoint;

        std::cout << "[NetworkManager] New client connected: " << clientId << " ("
                  << endpoint.address().to_string() << ":" << endpoint.port() << ")" << '\n';
    }
}

std::string GameEngine::NetworkManager::getClientId(
    const boost::asio::ip::udp::endpoint &endpoint) const
{
    return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
}

boost::asio::ip::udp::endpoint GameEngine::NetworkManager::getClientEndpoint(
    const std::string &clientId) const
{
    std::lock_guard const lock(_clientsMutex);
    const auto it = _clientEndpoints.find(clientId);
    if (it != _clientEndpoints.end()) {
        return it->second;
    }
    return {};
}

const std::unordered_map<std::string, boost::asio::ip::udp::endpoint> &GameEngine::NetworkManager::
    getClientEndpoints() const
{
    return _clientEndpoints;
}

void GameEngine::NetworkManager::addPunctualPacket(const std::vector<std::uint8_t> &data)
{
    std::lock_guard const lock(_punctualPacketsMutex);
    _punctualPackets.push_back(data);
}

void GameEngine::NetworkManager::addTargetedPunctualPacket(
    const std::string &clientId, const std::vector<std::uint8_t> &data)
{
    std::lock_guard const lock(_punctualPacketsMutex);
    _targetedPunctualPackets[clientId].push_back(data);
}

void GameEngine::NetworkManager::sendPunctualPackets()
{
    std::lock_guard const lock(_punctualPacketsMutex);

    for (const auto &packet : _punctualPackets) {
        if (isServer()) {
            sendPacketToAll(packet);
        } else {
            sendPacketToServer(packet);
        }
    }
    _punctualPackets.clear();

    for (const auto &[clientId, packets] : _targetedPunctualPackets) {
        if (isServer()) {
            auto endpoint = getClientEndpoint(clientId);
            if (endpoint != boost::asio::ip::udp::endpoint()) {
                for (const auto &packet : packets) {
                    sendPacketToClient(packet, endpoint);
                }
            }
        }
    }
    _targetedPunctualPackets.clear();
}

void GameEngine::NetworkManager::addBurstPacket(const std::string &burstId,
    const std::vector<std::uint8_t> &data, const float sendDelay,
    const std::string &targetClientId)
{
    std::lock_guard const lock(_burstPacketsMutex);

    BurstPacket burstPacket;
    burstPacket.data = data;
    burstPacket.sendDelay = sendDelay;
    burstPacket.lastSent = std::chrono::steady_clock::now();
    burstPacket.targetClientId = targetClientId;

    _burstPackets[burstId] = std::move(burstPacket);
}

void GameEngine::NetworkManager::removeBurstPacket(const std::string &burstId)
{
    std::lock_guard const lock(_burstPacketsMutex);
    _burstPackets.erase(burstId);
}

void GameEngine::NetworkManager::sendBurstPackets(float /*deltaTime*/)
{
    std::lock_guard const lock(_burstPacketsMutex);
    const auto now = std::chrono::steady_clock::now();

    for (auto &[burstId, burstPacket] : _burstPackets) {
        const auto timeSinceLastSent =
            std::chrono::duration_cast<std::chrono::duration<float>>(now - burstPacket.lastSent)
                .count();

        if (timeSinceLastSent >= burstPacket.sendDelay) {
            if (burstPacket.targetClientId.empty()) {
                if (isServer()) {
                    sendPacketToAll(burstPacket.data);
                } else {
                    sendPacketToServer(burstPacket.data);
                }
            } else {
                if (isServer()) {
                    auto endpoint = getClientEndpoint(burstPacket.targetClientId);
                    if (endpoint != boost::asio::ip::udp::endpoint()) {
                        sendPacketToClient(burstPacket.data, endpoint);
                    }
                }
            }

            burstPacket.lastSent = now;
        }
    }
}
