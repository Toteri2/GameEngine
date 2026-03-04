/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** PhysicsManager - Physics management with ECS integration
*/

#pragma once

#include <box2d/box2d.h>
#include <GameEngine/EcsManager.hpp>
#include <GameEngine/EventTypes.hpp>
#include <unordered_map>
#include <vector>
#include <functional>
#include <cstdint>
#define COLLIDE_ALL 0xFFFF

namespace GameEngine
{
    namespace Physics
    {
        struct CollisionInfo {
            std::uint32_t bodyA;
            std::uint32_t bodyB;
            b2Vec2 contactPoint;
            b2Vec2 normal;
            float impulse;
        };

        enum class BodyType {
            STATIC,
            DYNAMIC,
            KINEMATIC
        };

        enum class ShapeType {
            CIRCLE,
            BOX,
            POLYGON
        };

        struct BodyCreationInfo {
            BodyType type = BodyType::DYNAMIC;
            float x = 0.0F;
            float y = 0.0F;
            float angle = 0.0F;
            float linearDamping = 0.0F;
            float angularDamping = 0.0F;
            float gravityScale = 1.0F;
            bool fixedRotation = false;
            std::uint32_t userData = 0;
        };

        struct ShapeCreationInfo {
            ShapeType type = ShapeType::BOX;
            float density = 1.0F;
            float friction = 0.3F;
            float restitution = 0.0F;
            bool isSensor = false;

            std::uint16_t collisionLayer = COLLIDE_ALL;
            std::uint16_t collidesWith = COLLIDE_ALL;

            float radius = 0.5F;

            float width = 1.0F;
            float height = 1.0F;
        };

        class PhysicsManager final {
          public:
            PhysicsManager();
            ~PhysicsManager();

            bool initialize(const b2Vec2& worldGravity = {0.0F, 0.0F});
            void step(float dt, int velocityIterations = 8, int positionIterations = 3);
            void cleanup();

            b2BodyId createBody(const BodyCreationInfo& info);
            void destroyBody(b2BodyId bodyId);
            bool isBodyValid(b2BodyId bodyId) const;

            b2ShapeId attachShape(b2BodyId bodyId, const ShapeCreationInfo& info);
            void detachShape(b2ShapeId shapeId);

            void setVelocity(b2BodyId bodyId, const b2Vec2& velocity);
            b2Vec2 getVelocity(b2BodyId bodyId) const;
            void setAngularVelocity(b2BodyId bodyId, float omega);
            float getAngularVelocity(b2BodyId bodyId) const;

            void setPosition(b2BodyId bodyId, const b2Vec2& position);
            b2Vec2 getPosition(b2BodyId bodyId) const;
            void setRotation(b2BodyId bodyId, float angle);
            float getRotation(b2BodyId bodyId) const;

            void setGravityScale(b2BodyId bodyId, float scale);
            float getGravityScale(b2BodyId bodyId) const;

            void applyForce(b2BodyId bodyId, const b2Vec2& force, const b2Vec2& point, bool wake = true);
            void applyForceToCenter(b2BodyId bodyId, const b2Vec2& force, bool wake = true);
            void applyImpulse(b2BodyId bodyId, const b2Vec2& impulse, const b2Vec2& point, bool wake = true);
            void applyImpulseToCenter(b2BodyId bodyId, const b2Vec2& impulse, bool wake = true);
            void applyAngularImpulse(b2BodyId bodyId, float impulse, bool wake = true);

            std::vector<CollisionInfo> getCollisions() const;
            void clearCollisions();

            void setGravity(const b2Vec2& gravity);
            b2Vec2 getGravity() const;

            b2BodyType getBodyType(b2BodyId bodyId) const;

            bool isInitialized() const { return _initialized; }

            void syncComponentsToPhysics(EcsManager &ecs);
            void syncPhysicsToComponents(EcsManager &ecs);
            void processCollisionsAndPublish(EcsManager &ecs);

            void onStartPhysics(const StartPhysicsEvent &event);
            void onStopPhysics(const StopPhysicsEvent &event);
            void onSetGravity(const SetPhysicsGravityEvent &event);

          private:
            void processCollisions();
            static b2BodyType convertBodyType(BodyType type);

            b2WorldId _worldId;
            bool _initialized;
            std::vector<CollisionInfo> _collisions;

            std::unordered_map<EntityId, b2BodyId> _entityToBody;
            std::unordered_map<uint32_t, EntityId> _bodyToEntity;
        };
    };
}
