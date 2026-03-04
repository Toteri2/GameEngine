/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** RigidBodyComponent - Physics body component for ECS
*/

#pragma once

#include <GameEngine/IComponent.hpp>
#include <cstdint>

namespace GameEngine
{
    enum class PhysicsBodyType : std::uint8_t {
        STATIC = 0,
        DYNAMIC = 1,
        KINEMATIC = 2
    };

    struct PhysicsBodyHandle {
        std::int32_t index1 = 0;
        std::uint16_t world0 = 0;
        std::uint16_t generation = 0;
    };

    class RigidBodyComponent final : public IComponent {
      public:
        PhysicsBodyHandle bodyHandle;
        PhysicsBodyType bodyType = PhysicsBodyType::DYNAMIC;

        float x = 0.0F;
        float y = 0.0F;
        float angle = 0.0F;

        float vx = 0.0F;
        float vy = 0.0F;
        float angularVelocity = 0.0F;

        float gravityScale = 1.0F;
        float linearDamping = 0.0F;
        float angularDamping = 0.0F;
        bool fixedRotation = false;

        bool isInitialized = false;
    };
} // namespace GameEngine
