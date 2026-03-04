#pragma once
#include <cstdint>
#include <queue>

namespace GameEngine
{
    using EntityId = std::uint32_t;
    constexpr EntityId INVALID_ENTITY = 0;

    class EntityManager {
      public:
        EntityManager() = default;

        auto createEntity() -> EntityId
        {
            EntityId const entity = this->_nextEntity++;
            this->_entities.push(entity);
            this->_entityCount++;
            return entity;
        };

        auto removeEntity(const EntityId entity) -> void
        {
            std::queue<EntityId> updatedQueue;
            bool found = false;

            while (!this->_entities.empty()) {
                EntityId const current = this->_entities.front();
                this->_entities.pop();
                if (current != entity) {
                    updatedQueue.push(current);
                } else {
                    found = true;
                }
            }

            this->_entities = std::move(updatedQueue);
            if (found) {
                this->_entityCount--;
            }
        }

        [[nodiscard]] auto getNextEntityId() const -> EntityId { return this->_nextEntity; }

      private:
        std::queue<EntityId> _entities;
        EntityId _nextEntity = 1;
        EntityId _entityCount = 0;
    };
} // namespace GameEngine
