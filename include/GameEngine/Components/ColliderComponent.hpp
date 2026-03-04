/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** ColliderComponent - Physics collider component for ECS
*/

#pragma once

#include <GameEngine/IComponent.hpp>
#include <cstdint>
#include <vector>

namespace GameEngine
{
    enum class ColliderType : std::uint8_t {
        CIRCLE = 0,
        BOX = 1
    };

    struct PhysicsShapeHandle {
        std::int32_t index1 = 0;
        std::uint16_t world0 = 0;
        std::uint16_t generation = 0;
    };

    struct Vector2D {
        float x = 0.0F;
        float y = 0.0F;
    };

    class ColliderComponent final : public IComponent {
      public:
        PhysicsShapeHandle shapeHandle;
        ColliderType colliderType = ColliderType::BOX;

        float density = 1.0F;
        float friction = 0.3F;
        float restitution = 0.0F;
        bool isSensor = false;

        std::uint16_t collisionLayer = 0xFFFF;
        std::uint16_t collidesWith = 0xFFFF;

        float radius = 0.5F;

        float width = 1.0F;
        float height = 1.0F;

        bool isInitialized = false;
    };
} // namespace GameEngine
