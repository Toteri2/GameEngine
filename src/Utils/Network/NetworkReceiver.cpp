/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** NetworkReceiver
*/

#include "NetworkReceiver.hpp"
#include <boost/asio/ip/v6_only.hpp>
#include <boost/asio/buffer.hpp>
#include <chrono>
#include <exception>
#include <functional>
#include <iostream>

GameEngine::NetworkReceiver::NetworkReceiver(const std::uint16_t port)
    : _isServer(true), _port(port)
{
    _buffer.resize(1024);
    initializeServerSocket(port);
}

GameEngine::NetworkReceiver::NetworkReceiver(
    const std::uint16_t port, const std::string &serverAddress)
    : _isServer(false), _port(port), _serverAddress(serverAddress)
{
    _buffer.resize(1024);
    initializeClientSocket(port, serverAddress);
}

GameEngine::NetworkReceiver::~NetworkReceiver()
{
    stop();
}

void GameEngine::NetworkReceiver::initializeServerSocket(std::uint16_t port)
{
    try {
        _socket =
            std::make_unique<boost::asio::ip::udp::socket>(_ioContext, boost::asio::ip::udp::v6());
        _socket->set_option(boost::asio::ip::v6_only(false));
        _socket->bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v6(), port));

        std::cout << "[NetworkReceiver] Server socket bound to port " << port << '\n';
    } catch (const std::exception &e) {
        std::cerr << "[NetworkReceiver] Failed to create server socket: " << e.what() << '\n';
        throw;
    }
}

void GameEngine::NetworkReceiver::initializeClientSocket(
    const std::uint16_t port, const std::string &serverAddress)
{
    try {
        _socket = std::make_unique<boost::asio::ip::udp::socket>(_ioContext);
        boost::asio::ip::address const address =
            boost::asio::ip::address::from_string(serverAddress);
        const boost::asio::ip::udp::endpoint serverEndpoint(address, port);

        _socket->open(serverEndpoint.protocol());
        _socket->connect(serverEndpoint);

        std::cout << "[NetworkReceiver] Client socket connected to " << serverAddress << ":"
                  << port << '\n';
    } catch (const std::exception &e) {
        std::cerr << "[NetworkReceiver] Failed to create client socket: " << e.what() << '\n';
        throw;
    }
}

void GameEngine::NetworkReceiver::start()
{
    if (_running.load()) {
        return;
    }

    _shouldStop.store(false);
    _running.store(true);
    _networkThread = std::thread(&GameEngine::NetworkReceiver::networkThreadFunction, this);

    std::cout << "[NetworkReceiver] Started in " << (_isServer ? "server" : "client") << " mode"
              << '\n';
}

void GameEngine::NetworkReceiver::stop()
{
    stopNetworkThread();
}

bool GameEngine::NetworkReceiver::isRunning() const
{
    return _running.load();
}

bool GameEngine::NetworkReceiver::isServer() const
{
    return _isServer;
}

void GameEngine::NetworkReceiver::onReceive(
    const boost::system::error_code &error, const std::size_t bytesTransferred)
{
    if (!error && bytesTransferred > 0 && !_shouldStop.load()) {
        {
            std::lock_guard const lock(_queueMutex);
            ReceivedMessage msg;
            msg.data.assign(_buffer.begin(), _buffer.begin() + bytesTransferred);
            msg.sender = _senderEndpoint;
            _messageQueue.push(std::move(msg));
        }

        std::cout << "[NetworkReceiver] Received " << bytesTransferred << " bytes from "
                  << _senderEndpoint.address().to_string() << ":" << _senderEndpoint.port()
                  << '\n';

        if (!_shouldStop.load()) {
            handleMessage();
        }
    } else if (!_shouldStop.load() && error) {
        std::cerr << "[NetworkReceiver] Receive error: " << error.message() << '\n';
        if (!_shouldStop.load()) {
            handleMessage();
        }
    }
}

void GameEngine::NetworkReceiver::handleMessage()
{
    _socket->async_receive_from(boost::asio::buffer(_buffer), _senderEndpoint,
        std::bind(&GameEngine::NetworkReceiver::onReceive, this, std::placeholders::_1,
            std::placeholders::_2));
}

void GameEngine::NetworkReceiver::networkThreadFunction()
{
    try {
        handleMessage();

        while (!_shouldStop.load()) {
            _ioContext.run_for(std::chrono::milliseconds(10));
            _ioContext.restart();
        }
    } catch (const std::exception &e) {
        std::cerr << "[NetworkReceiver] Network thread error: " << e.what() << '\n';
    }
}

void GameEngine::NetworkReceiver::stopNetworkThread()
{
    if (_running.load()) {
        _shouldStop.store(true);

        try {
            if (_socket) {
                _socket->cancel();
            }
            _ioContext.stop();
        } catch (const std::exception &e) {
            std::cerr << "[NetworkReceiver] Error stopping socket: " << e.what() << '\n';
        }

        if (_networkThread.joinable()) {
            _networkThread.join();
        }

        try {
            if (_socket) {
                _socket->close();
            }
        } catch (const std::exception &e) {
            std::cerr << "[NetworkReceiver] Error closing socket: " << e.what() << '\n';
        }

        _running.store(false);
    }
}

bool GameEngine::NetworkReceiver::getNextMessage(ReceivedMessage &outMessage)
{
    std::lock_guard const lock(_queueMutex);

    if (_messageQueue.empty()) {
        return false;
    }

    outMessage = std::move(_messageQueue.front());
    _messageQueue.pop();
    return true;
}

bool GameEngine::NetworkReceiver::hasMessages() const
{
    std::lock_guard const lock(_queueMutex);
    return !_messageQueue.empty();
}

boost::asio::ip::udp::socket &GameEngine::NetworkReceiver::getSocket() const
{
    return *_socket;
}

boost::asio::io_context &GameEngine::NetworkReceiver::getIoContext()
{
    return _ioContext;
}
