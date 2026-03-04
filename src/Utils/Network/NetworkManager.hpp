/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** NetworkManager
*/

#pragma once

#include "NetworkReceiver.hpp"
#include <unordered_map>
#include <boost/asio/ip/udp.hpp>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace GameEngine
{
    class NetworkManager {
      public:
        NetworkManager();
        ~NetworkManager();

        NetworkManager(const NetworkManager &) = delete;
        NetworkManager &operator=(const NetworkManager &) = delete;

        using ReceivedMessage = NetworkReceiver::ReceivedMessage;

        struct BurstPacket {
            std::vector<std::uint8_t> data;
            float sendDelay;
            std::chrono::steady_clock::time_point lastSent;
            std::string targetClientId;
        };

        // Network lifecycle
        void startServer(std::uint16_t port);
        void startClient(std::uint16_t port, const std::string &serverAddress);
        void stop();
        bool isRunning() const;
        bool isServer() const;

        // Message retrieval
        bool getNextMessage(ReceivedMessage &outMessage) const;
        bool hasMessages() const;

        // Packet sending
        void sendPacketToClient(const std::vector<std::uint8_t> &data,
            const boost::asio::ip::udp::endpoint &endpoint) const;
        void sendPacketToAll(const std::vector<std::uint8_t> &data);
        void sendPacketToServer(const std::vector<std::uint8_t> &data) const;

        // Client management
        void addKnownClient(const boost::asio::ip::udp::endpoint &endpoint);
        std::string getClientId(const boost::asio::ip::udp::endpoint &endpoint) const;
        boost::asio::ip::udp::endpoint getClientEndpoint(const std::string &clientId) const;
        const std::unordered_map<std::string, boost::asio::ip::udp::endpoint> &getClientEndpoints()
            const;

        // Punctual packet queue management
        void addPunctualPacket(const std::vector<std::uint8_t> &data);
        void addTargetedPunctualPacket(
            const std::string &clientId, const std::vector<std::uint8_t> &data);
        void sendPunctualPackets();

        // Burst packet management
        void addBurstPacket(const std::string &burstId, const std::vector<std::uint8_t> &data,
            float sendDelay, const std::string &targetClientId = "");
        void removeBurstPacket(const std::string &burstId);
        void sendBurstPackets(float deltaTime);

      private:
        std::unique_ptr<NetworkReceiver> _receiver;

        // Client tracking
        std::map<std::string, boost::asio::ip::udp::endpoint> _knownClients;
        std::unordered_map<std::string, boost::asio::ip::udp::endpoint> _clientEndpoints;
        mutable std::mutex _clientsMutex;

        // Packet queues
        std::vector<std::vector<std::uint8_t>> _punctualPackets;
        std::map<std::string, std::vector<std::vector<std::uint8_t>>> _targetedPunctualPackets;
        std::mutex _punctualPacketsMutex;

        std::unordered_map<std::string, BurstPacket> _burstPackets;
        std::mutex _burstPacketsMutex;
    };
} // namespace GameEngine
