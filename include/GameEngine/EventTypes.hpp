/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** NetworkEvents
*/

#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace GameEngine
{
    enum class DeviceType : std::uint8_t { KEYBOARD = 0, MOUSE = 1, GAMEPAD = 2, TOUCH = 3 };
    enum class InputEventType : std::uint8_t {
        BUTTON_PRESSED = 0,
        BUTTON_RELEASED = 1,
        AXIS_MOTION = 2,
        DEVICE_CONNECTED = 3,
        DEVICE_DISCONNECTED = 4
    };
} // namespace GameEngine

#include "../../src/Systems/InputSystem/InputBindingManager.hpp"

namespace GameEngine
{
    using PacketId = std::uint32_t;
    constexpr PacketId INVALID_PACKET_ID = 0;

    enum class PacketType : std::uint8_t { PUNCTUAL = 0, BURST = 1 };

    class NetworkPacket {
      public:
        PacketId id;
        PacketType type;
        std::vector<std::uint8_t> data;

        std::chrono::steady_clock::time_point lastSent;
        float sendDelay;

        NetworkPacket(PacketId packetId, std::vector<std::uint8_t> packetData)
            : id(packetId), type(PacketType::PUNCTUAL), data(std::move(packetData)), lastSent(),
              sendDelay(0.0F)
        {}

        NetworkPacket(PacketId packetId, std::vector<std::uint8_t> packetData, float delay)
            : id(packetId), type(PacketType::BURST), data(std::move(packetData)), lastSent(),
              sendDelay(delay)
        {}
    };

    class ReceivedPacket {
      public:
        std::vector<std::uint8_t> data;
        std::string senderAddress;
        std::uint16_t senderPort;

        ReceivedPacket(std::vector<std::uint8_t> packetData)
            : data(std::move(packetData)), senderAddress(), senderPort(0)
        {}

        ReceivedPacket(
            std::vector<std::uint8_t> packetData, std::string address, std::uint16_t port)
            : data(std::move(packetData)), senderAddress(std::move(address)), senderPort(port)
        {}
    };

    class SendPunctualPacketEvent {
      public:
        PacketId id;
        std::vector<std::uint8_t> data;
        std::string targetClientId;

        SendPunctualPacketEvent(PacketId packetId, std::vector<std::uint8_t> packetData)
            : id(packetId), data(std::move(packetData)), targetClientId()
        {}

        SendPunctualPacketEvent(
            PacketId packetId, std::vector<std::uint8_t> packetData, std::string clientId)
            : id(packetId), data(std::move(packetData)), targetClientId(std::move(clientId))
        {}
    };

    class StartBurstPacketEvent {
      public:
        PacketId id;
        std::vector<std::uint8_t> data;
        float sendDelay;
        std::string targetClientId;

        StartBurstPacketEvent(PacketId packetId, std::vector<std::uint8_t> packetData, float delay)
            : id(packetId), data(std::move(packetData)), sendDelay(delay), targetClientId()
        {}

        StartBurstPacketEvent(PacketId packetId, std::vector<std::uint8_t> packetData, float delay,
            std::string clientId)
            : id(packetId), data(std::move(packetData)), sendDelay(delay),
              targetClientId(std::move(clientId))
        {}
    };

    class StopBurstPacketEvent {
      public:
        PacketId id;

        explicit StopBurstPacketEvent(PacketId packetId) : id(packetId) {}
    };

    class StartNetworkEvent {
      public:
        std::uint16_t port;
        std::string serverAddress;
        bool isServer;

        explicit StartNetworkEvent(std::uint16_t serverPort)
            : port(serverPort), serverAddress(), isServer(true)
        {}

        StartNetworkEvent(std::uint16_t serverPort, std::string address)
            : port(serverPort), serverAddress(std::move(address)), isServer(false)
        {}
    };

    class StopNetworkEvent {
      public:
        StopNetworkEvent() = default;
    };

    class NetworkErrorEvent {
      public:
        std::string errorMessage;
        bool isFatal;

        explicit NetworkErrorEvent(std::string message, bool fatal = true)
            : errorMessage(std::move(message)), isFatal(fatal)
        {}
    };

    class LowLevelInputEvent {
      public:
        DeviceType deviceType;
        std::uint32_t deviceId;
        InputEventType eventType;
        std::uint32_t code;
        float value;
        std::uint64_t timestamp;

        LowLevelInputEvent(DeviceType devType, std::uint32_t devId, InputEventType evtType,
            std::uint32_t inputCode, float inputValue)
            : deviceType(devType), deviceId(devId), eventType(evtType), code(inputCode),
              value(inputValue), timestamp(0)
        {}

        LowLevelInputEvent(DeviceType devType, std::uint32_t devId, InputEventType evtType,
            std::uint32_t inputCode, float inputValue, std::uint64_t timeStamp)
            : deviceType(devType), deviceId(devId), eventType(evtType), code(inputCode),
              value(inputValue), timestamp(timeStamp)
        {}
    };

    class KeyboardTouchEvent {
      public:
        std::uint32_t scancode;
        std::uint64_t timestamp;
        std::string textKey;
        InputEventType eventType;

        KeyboardTouchEvent(std::uint32_t code, std::string text, InputEventType evtType,
            std::uint64_t timeStamp = 0)
            : scancode(code), textKey(std::move(text)), eventType(evtType), timestamp(timeStamp)
        {}
    };

    class InputActionEvent {
      public:
        std::string actionName;
        bool pressed;
        std::uint32_t playerId;
        DeviceType sourceDevice;
        std::uint32_t sourceDeviceId;
        std::uint64_t timestamp;

        InputActionEvent(std::string action, bool isPressed, std::uint32_t player = 0)
            : actionName(std::move(action)), pressed(isPressed), playerId(player),
              sourceDevice(DeviceType::KEYBOARD), sourceDeviceId(0), timestamp(0)
        {}
    };

    class InputAxisEvent {
      public:
        std::string axisName;
        float value;
        std::uint32_t playerId;
        DeviceType sourceDevice;
        std::uint32_t sourceDeviceId;
        std::uint64_t timestamp;

        InputAxisEvent(std::string axis, float axisValue, std::uint32_t player = 0)
            : axisName(std::move(axis)), value(axisValue), playerId(player),
              sourceDevice(DeviceType::KEYBOARD), sourceDeviceId(0), timestamp(0)
        {}
    };

    class InputDeviceEvent {
      public:
        DeviceType deviceType;
        std::uint32_t deviceId;
        bool connected;
        std::string deviceName;

        InputDeviceEvent(
            DeviceType devType, std::uint32_t devId, bool isConnected, std::string name = "")
            : deviceType(devType), deviceId(devId), connected(isConnected),
              deviceName(std::move(name))
        {}
    };

    class ChangeBindingEvent {
      public:
        std::string contextName;
        int bindType;
        int deviceType;
        std::uint32_t code;
        std::string actionName;
        int value = 0;
        bool changeBinding;

        ChangeBindingEvent(std::string ctxName, int bType, int devType, std::uint32_t inputCode,
            std::string actName, int val = 0, bool change = true)
            : contextName(std::move(ctxName)), bindType(bType), deviceType(devType),
              code(inputCode), actionName(std::move(actName)), value(val), changeBinding(change)
        {}
    };

    class RecoverBindingsEvent {
      public:
        std::string contextName;

        explicit RecoverBindingsEvent(std::string ctxName) : contextName(std::move(ctxName)) {}
    };

    class SharedBindingsEvent {
      public:
        std::vector<InputBinding> bindings;
        std::string contextName;

        explicit SharedBindingsEvent(std::string ctxName, std::vector<InputBinding> bindList)
            : contextName(std::move(ctxName)), bindings(std::move(bindList))
        {}
    };

    class CreateContextEvent {
      public:
        std::string contextName;
        int priority;

        explicit CreateContextEvent(std::string ctxName, int prio = 0)
            : contextName(std::move(ctxName)), priority(prio)
        {}
    };

    class ExitEvent {};

    class PhysicsCollisionEvent {
      public:
        std::uint32_t entityA;
        std::uint32_t entityB;
        float contactPointX;
        float contactPointY;
        float normalX;
        float normalY;
        float impulse;

        PhysicsCollisionEvent(std::uint32_t entA, std::uint32_t entB, float cpX, float cpY,
            float nX, float nY, float imp)
            : entityA(entA), entityB(entB), contactPointX(cpX), contactPointY(cpY),
              normalX(nX), normalY(nY), impulse(imp)
        {}
    };

    class StartPhysicsEvent {
      public:
        float gravityX;
        float gravityY;

        explicit StartPhysicsEvent(float gx = 0.0F, float gy = 0.0F)
            : gravityX(gx), gravityY(gy)
        {}
    };

    class StopPhysicsEvent {
      public:
        StopPhysicsEvent() = default;
    };

    class SetPhysicsGravityEvent {
      public:
        float gravityX;
        float gravityY;

        SetPhysicsGravityEvent(float gx, float gy) : gravityX(gx), gravityY(gy) {}
    };

    class RegisterConsoleCommandEvent {
      public:
        std::string command;
        std::function<void(const std::vector<std::string> &, std::function<void(const std::string&)>)> function;

        RegisterConsoleCommandEvent(
            std::string cmd, std::function<void(const std::vector<std::string> &, std::function<void(const std::string&)>)> func)
            : command(std::move(cmd)), function(std::move(func))
        {}
    };

    class ToggleConsoleEvent {
      public:
        ToggleConsoleEvent() = default;
    };

    class ExecuteConsoleCommandEvent {
      public:
        std::string commandLine;

        ExecuteConsoleCommandEvent(std::string cmd) : commandLine(std::move(cmd)) {}
    };

    class TextInputEvent {
      public:
        std::string text;
        std::uint64_t timestamp;

        TextInputEvent(std::string inputText, std::uint64_t time)
            : text(std::move(inputText)), timestamp(time)
        {}
    };

} // namespace GameEngine
