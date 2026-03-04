/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** NetworkReceiver
*/

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/system/error_code.hpp>
#include <atomic>
#include <boost/asio/ip/udp.hpp>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace GameEngine
{
    class NetworkReceiver {
      public:
        struct ReceivedMessage {
            std::vector<std::uint8_t> data;
            boost::asio::ip::udp::endpoint sender;

            ReceivedMessage() = default;
            ReceivedMessage(std::vector<std::uint8_t> messageData,
                boost::asio::ip::udp::endpoint senderEndpoint)
                : data(std::move(messageData)), sender(std::move(senderEndpoint))
            {}
        };

        explicit NetworkReceiver(std::uint16_t port);
        NetworkReceiver(std::uint16_t port, const std::string &serverAddress);
        ~NetworkReceiver();

        NetworkReceiver(const NetworkReceiver &) = delete;
        NetworkReceiver &operator=(const NetworkReceiver &) = delete;

        // Message retrieval
        bool getNextMessage(ReceivedMessage &outMessage);
        bool hasMessages() const;

        // Socket access
        boost::asio::ip::udp::socket &getSocket() const;
        boost::asio::io_context &getIoContext();

        // Lifecycle
        void start();
        void stop();
        bool isRunning() const;
        bool isServer() const;

      private:
        void initializeServerSocket(std::uint16_t port);
        void initializeClientSocket(std::uint16_t port, const std::string &serverAddress);

        void onReceive(const boost::system::error_code &error, std::size_t bytesTransferred);
        void handleMessage();
        void networkThreadFunction();
        void stopNetworkThread();

        bool _isServer;
        std::uint16_t _port;
        std::string _serverAddress;

        boost::asio::io_context _ioContext;
        std::unique_ptr<boost::asio::ip::udp::socket> _socket;

        std::thread _networkThread;
        std::atomic<bool> _running{false};
        std::atomic<bool> _shouldStop{false};

        std::vector<std::uint8_t> _buffer;
        boost::asio::ip::udp::endpoint _senderEndpoint;
        std::queue<ReceivedMessage> _messageQueue;
        mutable std::mutex _queueMutex;
    };
} // namespace GameEngine
