/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** PhysicsManager implementation
*/

#include "PhysicsManager.hpp"
#include <GameEngine/Components/ColliderComponent.hpp>
#include <GameEngine/Components/RigidBodyComponent.hpp>
#include <iostream>
#include <stdexcept>

namespace
{
    b2BodyId toBox2DBodyId(const GameEngine::PhysicsBodyHandle &handle)
    {
        b2BodyId id;
        id.index1 = handle.index1;
        id.world0 = handle.world0;
        id.generation = handle.generation;
        return id;
    }

    GameEngine::PhysicsBodyHandle fromBox2DBodyId(b2BodyId id)
    {
        return {id.index1, id.world0, id.generation};
    }

    b2ShapeId toBox2DShapeId(const GameEngine::PhysicsShapeHandle &handle)
    {
        b2ShapeId id;
        id.index1 = handle.index1;
        id.world0 = handle.world0;
        id.generation = handle.generation;
        return id;
    }

    GameEngine::PhysicsShapeHandle fromBox2DShapeId(b2ShapeId id)
    {
        return {id.index1, id.world0, id.generation};
    }
}

namespace GameEngine::Physics
{
    PhysicsManager::PhysicsManager() : _worldId{0}, _initialized(false) {}

    PhysicsManager::~PhysicsManager()
    {
        cleanup();
    }

    bool PhysicsManager::initialize(const b2Vec2 &worldGravity)
    {
        if (_initialized) {
            return true;
        }

        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = worldGravity;
        _worldId = b2CreateWorld(&worldDef);

        if (!b2World_IsValid(_worldId)) {
            std::cerr << "[PhysicsManager] Failed to create Box2D world!" << std::endl;
            return false;
        }

        _initialized = true;
        std::cout << "[PhysicsManager] Initialized with gravity (" << worldGravity.x << ", "
                  << worldGravity.y << ")" << std::endl;
        return true;
    }

    void PhysicsManager::step(float dt, int velocityIterations, int positionIterations)
    {
        if (!_initialized) {
            return;
        }

        _collisions.clear();
        b2World_Step(_worldId, dt, positionIterations);
        processCollisions();
    }

    void PhysicsManager::cleanup()
    {
        if (_initialized) {
            b2DestroyWorld(_worldId);
            _collisions.clear();
            _initialized = false;
            std::cout << "[PhysicsManager] Cleaned up" << std::endl;
        }
    }

    b2BodyId PhysicsManager::createBody(const BodyCreationInfo &info)
    {
        if (!_initialized) {
            throw std::runtime_error("[PhysicsManager] Cannot create body: not initialized");
        }

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = convertBodyType(info.type);
        bodyDef.position = {info.x, info.y};
        bodyDef.rotation = b2MakeRot(info.angle);
        bodyDef.linearDamping = info.linearDamping;
        bodyDef.angularDamping = info.angularDamping;
        bodyDef.gravityScale = info.gravityScale;
        bodyDef.fixedRotation = info.fixedRotation;
        bodyDef.userData = reinterpret_cast<void *>(static_cast<uintptr_t>(info.userData));

        return b2CreateBody(_worldId, &bodyDef);
    }

    void PhysicsManager::destroyBody(b2BodyId bodyId)
    {
        if (b2Body_IsValid(bodyId)) {
            b2DestroyBody(bodyId);
        }
    }

    bool PhysicsManager::isBodyValid(b2BodyId bodyId) const
    {
        return b2Body_IsValid(bodyId);
    }

    b2ShapeId PhysicsManager::attachShape(b2BodyId bodyId, const ShapeCreationInfo &info)
    {
        if (!b2Body_IsValid(bodyId)) {
            throw std::runtime_error("[PhysicsManager] Invalid body ID");
        }

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = info.density;
        shapeDef.isSensor = info.isSensor;

        if (info.isSensor) {
            shapeDef.enableSensorEvents = true;
        } else {
            shapeDef.enableContactEvents = true;
            shapeDef.enableHitEvents = true;
        }
        shapeDef.filter.categoryBits =
            info.collisionLayer;
        shapeDef.filter.maskBits = info.collidesWith;

        b2ShapeId shapeId;
        switch (info.type) {
            case ShapeType::CIRCLE: {
                b2Circle circle = {{0.0F, 0.0F}, info.radius};
                shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
                break;
            }
            case ShapeType::BOX: {
                b2Polygon box = b2MakeBox(info.width / 2.0F, info.height / 2.0F);
                shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
                break;
            }
            default:
                throw std::runtime_error("[PhysicsManager] Unknown shape type");
        }

        if (b2Shape_IsValid(shapeId)) {
            b2Shape_SetFriction(shapeId, info.friction);
            b2Shape_SetRestitution(shapeId, info.restitution);
        }

        return shapeId;
    }

    void PhysicsManager::detachShape(b2ShapeId shapeId)
    {
        if (b2Shape_IsValid(shapeId)) {
            b2DestroyShape(shapeId, true);
        }
    }

    void PhysicsManager::setVelocity(b2BodyId bodyId, const b2Vec2 &velocity)
    {
        if (b2Body_IsValid(bodyId)) {
            b2Body_SetLinearVelocity(bodyId, velocity);
        }
    }

    b2Vec2 PhysicsManager::getVelocity(b2BodyId bodyId) const
    {
        if (b2Body_IsValid(bodyId)) {
            return b2Body_GetLinearVelocity(bodyId);
        }
        return {0.0F, 0.0F};
    }

    void PhysicsManager::setAngularVelocity(b2BodyId bodyId, float omega)
    {
        if (b2Body_IsValid(bodyId)) {
            b2Body_SetAngularVelocity(bodyId, omega);
        }
    }

    float PhysicsManager::getAngularVelocity(b2BodyId bodyId) const
    {
        if (b2Body_IsValid(bodyId)) {
            return b2Body_GetAngularVelocity(bodyId);
        }
        return 0.0F;
    }

    void PhysicsManager::setPosition(b2BodyId bodyId, const b2Vec2 &position)
    {
        if (b2Body_IsValid(bodyId)) {
            b2Rot rotation = b2Body_GetRotation(bodyId);
            b2Body_SetTransform(bodyId, position, rotation);
        }
    }

    b2Vec2 PhysicsManager::getPosition(b2BodyId bodyId) const
    {
        if (b2Body_IsValid(bodyId)) {
            return b2Body_GetPosition(bodyId);
        }
        return {0.0F, 0.0F};
    }

    void PhysicsManager::setRotation(b2BodyId bodyId, float angle)
    {
        if (b2Body_IsValid(bodyId)) {
            b2Vec2 position = b2Body_GetPosition(bodyId);
            b2Body_SetTransform(bodyId, position, b2MakeRot(angle));
        }
    }

    float PhysicsManager::getRotation(b2BodyId bodyId) const
    {
        if (b2Body_IsValid(bodyId)) {
            b2Rot rotation = b2Body_GetRotation(bodyId);
            return b2Rot_GetAngle(rotation);
        }
        return 0.0F;
    }

    void PhysicsManager::setGravityScale(b2BodyId bodyId, float scale)
    {
        if (b2Body_IsValid(bodyId)) {
            b2Body_SetGravityScale(bodyId, scale);
        }
    }

    float PhysicsManager::getGravityScale(b2BodyId bodyId) const
    {
        if (b2Body_IsValid(bodyId)) {
            return b2Body_GetGravityScale(bodyId);
        }
        return 1.0F;
    }

    void PhysicsManager::applyForce(
        b2BodyId bodyId, const b2Vec2 &force, const b2Vec2 &point, bool wake)
    {
        if (b2Body_IsValid(bodyId)) {
            b2Body_ApplyForce(bodyId, force, point, wake);
        }
    }

    void PhysicsManager::applyForceToCenter(b2BodyId bodyId, const b2Vec2 &force, bool wake)
    {
        if (b2Body_IsValid(bodyId)) {
            b2Body_ApplyForceToCenter(bodyId, force, wake);
        }
    }

    void PhysicsManager::applyImpulse(
        b2BodyId bodyId, const b2Vec2 &impulse, const b2Vec2 &point, bool wake)
    {
        if (b2Body_IsValid(bodyId)) {
            b2Body_ApplyLinearImpulse(bodyId, impulse, point, wake);
        }
    }

    void PhysicsManager::applyImpulseToCenter(b2BodyId bodyId, const b2Vec2 &impulse, bool wake)
    {
        if (b2Body_IsValid(bodyId)) {
            b2Body_ApplyLinearImpulseToCenter(bodyId, impulse, wake);
        }
    }

    void PhysicsManager::applyAngularImpulse(b2BodyId bodyId, float impulse, bool wake)
    {
        if (b2Body_IsValid(bodyId)) {
            b2Body_ApplyAngularImpulse(bodyId, impulse, wake);
        }
    }

    std::vector<CollisionInfo> PhysicsManager::getCollisions() const
    {
        return _collisions;
    }

    void PhysicsManager::clearCollisions()
    {
        _collisions.clear();
    }

    void PhysicsManager::setGravity(const b2Vec2 &gravity)
    {
        if (_initialized) {
            b2World_SetGravity(_worldId, gravity);
        }
    }

    b2Vec2 PhysicsManager::getGravity() const
    {
        if (_initialized) {
            return b2World_GetGravity(_worldId);
        }
        return {0.0F, 0.0F};
    }

    b2BodyType PhysicsManager::getBodyType(b2BodyId bodyId) const
    {
        if (b2Body_IsValid(bodyId)) {
            return b2Body_GetType(bodyId);
        }
        return b2_staticBody;
    }

    void PhysicsManager::processCollisions()
    {
        if (!_initialized) {
            return;
        }
        b2ContactEvents events = b2World_GetContactEvents(_worldId);
        b2SensorEvents sensorEvents = b2World_GetSensorEvents(_worldId);

        for (int i = 0; i < events.beginCount; ++i) {
            const b2ContactBeginTouchEvent &event = events.beginEvents[i];

            b2ShapeId shapeA = event.shapeIdA;
            b2ShapeId shapeB = event.shapeIdB;

            if (!b2Shape_IsValid(shapeA) || !b2Shape_IsValid(shapeB)) {
                continue;
            }

            b2BodyId bodyA = b2Shape_GetBody(shapeA);
            b2BodyId bodyB = b2Shape_GetBody(shapeB);

            if (!b2Body_IsValid(bodyA) || !b2Body_IsValid(bodyB)) {
                continue;
            }

            CollisionInfo collision;
            collision.bodyA = bodyA.index1;
            collision.bodyB = bodyB.index1;
            collision.contactPoint = {0.0F, 0.0F};
            collision.normal = {0.0F, 0.0F};
            collision.impulse = 0.0F;

            _collisions.push_back(collision);
        }

        for (int i = 0; i < events.hitCount; ++i) {
            const b2ContactHitEvent &event = events.hitEvents[i];

            b2ShapeId shapeA = event.shapeIdA;
            b2ShapeId shapeB = event.shapeIdB;

            if (!b2Shape_IsValid(shapeA) || !b2Shape_IsValid(shapeB)) {
                continue;
            }

            b2BodyId bodyA = b2Shape_GetBody(shapeA);
            b2BodyId bodyB = b2Shape_GetBody(shapeB);

            if (!b2Body_IsValid(bodyA) || !b2Body_IsValid(bodyB)) {
                continue;
            }

            CollisionInfo collision;
            collision.bodyA = bodyA.index1;
            collision.bodyB = bodyB.index1;
            collision.contactPoint = event.point;
            collision.normal = event.normal;
            collision.impulse = event.approachSpeed;

            _collisions.push_back(collision);
        }

        for (int i = 0; i < sensorEvents.beginCount; ++i) {
            const b2SensorBeginTouchEvent &event = sensorEvents.beginEvents[i];

            b2ShapeId sensorShape = event.sensorShapeId;
            b2ShapeId visitorShape = event.visitorShapeId;

            if (!b2Shape_IsValid(sensorShape) || !b2Shape_IsValid(visitorShape)) {
                continue;
            }

            b2BodyId bodyA = b2Shape_GetBody(sensorShape);
            b2BodyId bodyB = b2Shape_GetBody(visitorShape);

            if (!b2Body_IsValid(bodyA) || !b2Body_IsValid(bodyB)) {
                continue;
            }

            CollisionInfo collision;
            collision.bodyA = bodyA.index1;
            collision.bodyB = bodyB.index1;
            collision.contactPoint = {0.0F, 0.0F};
            collision.normal = {0.0F, 0.0F};
            collision.impulse = 0.0F;

            _collisions.push_back(collision);
        }
    }

    b2BodyType PhysicsManager::convertBodyType(BodyType type)
    {
        switch (type) {
            case BodyType::STATIC:
                return b2_staticBody;
            case BodyType::DYNAMIC:
                return b2_dynamicBody;
            case BodyType::KINEMATIC:
                return b2_kinematicBody;
            default:
                return b2_dynamicBody;
        }
    }

    void PhysicsManager::syncComponentsToPhysics(EcsManager &ecs)
    {
        auto entities = ecs.getAllEntitiesWith<RigidBodyComponent>();

        for (auto entity : entities) {
            auto &rigidBody = ecs.getComponent<RigidBodyComponent>(entity);

            if (!rigidBody.isInitialized) {
                BodyCreationInfo bodyInfo;
                bodyInfo.type = static_cast<BodyType>(rigidBody.bodyType);
                bodyInfo.x = rigidBody.x;
                bodyInfo.y = rigidBody.y;
                bodyInfo.angle = rigidBody.angle;
                bodyInfo.linearDamping = rigidBody.linearDamping;
                bodyInfo.angularDamping = rigidBody.angularDamping;
                bodyInfo.gravityScale = rigidBody.gravityScale;
                bodyInfo.fixedRotation = rigidBody.fixedRotation;
                bodyInfo.userData = entity;

                b2BodyId bodyId = createBody(bodyInfo);
                rigidBody.bodyHandle = fromBox2DBodyId(bodyId);
                rigidBody.isInitialized = true;

                _entityToBody[entity] = bodyId;
                _bodyToEntity[bodyId.index1] = entity;

                if (ecs.hasComponent<ColliderComponent>(entity)) {
                    auto &collider = ecs.getComponent<ColliderComponent>(entity);

                    ShapeCreationInfo shapeInfo;
                    shapeInfo.type = static_cast<ShapeType>(collider.colliderType);
                    shapeInfo.density = collider.density;
                    shapeInfo.friction = collider.friction;
                    shapeInfo.restitution = collider.restitution;
                    shapeInfo.isSensor = collider.isSensor;
                    shapeInfo.collisionLayer = collider.collisionLayer;
                    shapeInfo.collidesWith = collider.collidesWith;
                    shapeInfo.radius = collider.radius;
                    shapeInfo.width = collider.width;
                    shapeInfo.height = collider.height;

                    b2ShapeId shapeId = attachShape(bodyId, shapeInfo);
                    collider.shapeHandle = fromBox2DShapeId(shapeId);
                    collider.isInitialized = true;
                }

                if (rigidBody.bodyType == PhysicsBodyType::DYNAMIC) {
                    setVelocity(bodyId, {rigidBody.vx, rigidBody.vy});
                    setAngularVelocity(bodyId, rigidBody.angularVelocity);
                }
            } else {
                b2BodyId bodyId = toBox2DBodyId(rigidBody.bodyHandle);

                b2BodyType bodyType = getBodyType(bodyId);
                if (bodyType == b2_kinematicBody) {
                    setPosition(bodyId, {rigidBody.x, rigidBody.y});
                    setRotation(bodyId, rigidBody.angle);
                } else {
                    setVelocity(bodyId, {rigidBody.vx, rigidBody.vy});
                    setAngularVelocity(bodyId, rigidBody.angularVelocity);
                }
            }
        }
    }

    void PhysicsManager::syncPhysicsToComponents(EcsManager &ecs)
    {
        auto entities = ecs.getAllEntitiesWith<RigidBodyComponent>();

        for (auto entity : entities) {
            auto &rigidBody = ecs.getComponent<RigidBodyComponent>(entity);

            if (rigidBody.isInitialized) {
                b2BodyId bodyId = toBox2DBodyId(rigidBody.bodyHandle);
                if (isBodyValid(bodyId)) {
                    b2Vec2 position = getPosition(bodyId);
                    rigidBody.x = position.x;
                    rigidBody.y = position.y;
                    rigidBody.angle = getRotation(bodyId);

                    b2Vec2 velocity = getVelocity(bodyId);
                    rigidBody.vx = velocity.x;
                    rigidBody.vy = velocity.y;
                    rigidBody.angularVelocity = getAngularVelocity(bodyId);
                }
            }
        }
    }

    void PhysicsManager::processCollisionsAndPublish(EcsManager &ecs)
    {
        auto collisions = getCollisions();

        for (const auto &collision : collisions) {
            auto itA = _bodyToEntity.find(collision.bodyA);
            auto itB = _bodyToEntity.find(collision.bodyB);

            if (itA != _bodyToEntity.end() && itB != _bodyToEntity.end()) {
                EntityId entityA = itA->second;
                EntityId entityB = itB->second;

                std::cout << "[PhysicsManager] Collision between entity " << entityA << " and entity "
                          << entityB << std::endl;

                PhysicsCollisionEvent event(entityA, entityB, collision.contactPoint.x,
                    collision.contactPoint.y, collision.normal.x, collision.normal.y,
                    collision.impulse);

                ecs.publishEvent(event);
            }
        }

        clearCollisions();
    }

    void PhysicsManager::onStartPhysics(const StartPhysicsEvent &event)
    {
        if (!_initialized) {
            b2Vec2 gravity = {event.gravityX, event.gravityY};
            if (initialize(gravity)) {
                std::cout << "[PhysicsManager] Started with gravity (" << event.gravityX << ", "
                          << event.gravityY << ")" << std::endl;
            } else {
                std::cerr << "[PhysicsManager] Failed to start physics" << std::endl;
            }
        }
    }

    void PhysicsManager::onStopPhysics(const StopPhysicsEvent & /*event*/)
    {
        cleanup();
        _entityToBody.clear();
        _bodyToEntity.clear();
        std::cout << "[PhysicsManager] Stopped" << std::endl;
    }

    void PhysicsManager::onSetGravity(const SetPhysicsGravityEvent &event)
    {
        if (_initialized) {
            setGravity({event.gravityX, event.gravityY});
            std::cout << "[PhysicsManager] Gravity set to (" << event.gravityX << ", " << event.gravityY
                      << ")" << std::endl;
        }
    }
} // namespace GameEngine::Physics
