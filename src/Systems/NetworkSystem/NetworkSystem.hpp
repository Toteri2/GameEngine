/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** NetworkSystem
*/

#pragma once

#include "../../Utils/Network/NetworkManager.hpp"
#include <GameEngine/EcsManager.hpp>
#include <GameEngine/EventTypes.hpp>

namespace GameEngine
{
    class NetworkSystem final : public ISystem {
      public:
        NetworkSystem();
        ~NetworkSystem() override;

        auto update(EcsManager &world, float deltaTime) -> void override;

      private:
        void onStartNetwork(const StartNetworkEvent &event, EcsManager &world);
        void onStopNetwork(const StopNetworkEvent &event);
        void onSendPunctualPacket(const SendPunctualPacketEvent &event);
        void onStartBurstPacket(const StartBurstPacketEvent &event);
        void onStopBurstPacket(const StopBurstPacketEvent &event);

        void processReceivedPackets(EcsManager &world);

        NetworkManager _networkManager;
    };
} // namespace GameEngine
