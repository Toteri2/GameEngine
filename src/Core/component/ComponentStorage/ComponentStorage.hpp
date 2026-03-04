#pragma once
#include "IComponentStorage.hpp"
#include <unordered_map>
#include <ranges>
#include <stdexcept>
#include <vector>

namespace GameEngine
{
    template<typename T> class ComponentStorage final : public IComponentStorage<T> {
      public:
        auto insertData(const EntityId entityId, T component) -> void override
        {
            if (hasData(entityId)) {
                _components[_entityToIndex[entityId]] = component;
                return;
            }

            _entityToIndex[entityId] = _size;
            _indexToEntity[_size] = entityId;
            _components.push_back(component);
            ++_size;
        }

        auto removeData(const EntityId entity) -> void override
        {
            if (!hasData(entity)) {
                return;
            }

            std::size_t const indexToRemove = _entityToIndex[entity];
            std::size_t const lastIndex = _size - 1;

            if (indexToRemove != lastIndex) {
                const EntityId lastEntity = _indexToEntity[lastIndex];
                _components[indexToRemove] = _components[lastIndex];
                _entityToIndex[lastEntity] = indexToRemove;
                _indexToEntity[indexToRemove] = lastEntity;
            }

            _entityToIndex.erase(entity);
            _indexToEntity.erase(lastIndex);
            _components.pop_back();
            --_size;
        }

        auto getData(const EntityId entity) const -> const T & override
        {
            if (!hasData(entity)) {
                throw std::runtime_error("Entity does not have a component");
            }
            return _components[_entityToIndex.at(entity)];
        }

        [[nodiscard]] auto hasData(const EntityId entity) const -> bool override
        {
            return _entityToIndex.contains(entity);
        }

        [[nodiscard]] auto getEntities() const -> std::vector<EntityId> override
        {
            std::vector<EntityId> entities;
            entities.reserve(_entityToIndex.size());

            for (const auto &entity : _entityToIndex | std::views::keys) {
                entities.push_back(entity);
            }

            return entities;
        }

        auto removeEntityData(EntityId entity) -> void override { removeData(entity); }

      private:
        std::vector<T> _components;
        std::unordered_map<EntityId, std::size_t> _entityToIndex;
        std::unordered_map<std::size_t, EntityId> _indexToEntity;
        std::size_t _size = 0;
    };
} // namespace GameEngine