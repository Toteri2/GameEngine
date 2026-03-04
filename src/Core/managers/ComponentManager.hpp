#pragma once
#include "../component/ComponentStorage/ComponentStorage.hpp"
#include "../component/ComponentStorage/IComponentStorage.hpp"
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <typeindex>

namespace GameEngine
{
    class ComponentManager {
      public:
        template<typename T> auto registerComponent() -> void
        {
            const auto type = std::type_index(typeid(T));

            if (_componentStorage.contains(type)) {
                return;
            }

            _componentTypes[type] = _nextComponentType;
            _componentStorage[type] = std::make_unique<ComponentStorage<T>>();
            ++_nextComponentType;
        }

        template<typename T> auto getComponentType() -> std::uint8_t
        {
            const auto type = std::type_index(typeid(T));
            const auto it = _componentTypes.find(type);

            if (it == _componentTypes.end()) {
                throw std::runtime_error("Component type not registered");
            }

            return it->second;
        }

        template<typename T> auto addComponent(EntityId entity, T component) -> void
        {
            getComponentArray<T>().insertData(entity, component);
        }

        template<typename T> auto removeComponent(EntityId entity) -> void
        {
            getComponentArray<T>().removeData(entity);
        }

        template<typename T> auto getComponent(EntityId entity) -> T &
        {
            return const_cast<T &>(getComponentArray<T>().getData(entity));
        }

        template<typename T> auto hasComponent(EntityId entity) -> bool
        {
            return getComponentArray<T>().hasData(entity);
        }

        template<typename T> auto getEntityList() -> std::vector<EntityId>
        {
            return getComponentArray<T>().getEntities();
        }

        auto removeAllComponents(EntityId entity) -> void
        {
            for (auto &[type, storage] : _componentStorage) {
                storage->removeEntityData(entity);
            }
        }

      private:
        std::unordered_map<std::type_index, std::unique_ptr<IComponentStorageBase>>
            _componentStorage;
        std::unordered_map<std::type_index, uint8_t> _componentTypes;
        uint8_t _nextComponentType = 0;

        template<typename T> auto getComponentArray() -> IComponentStorage<T> &
        {
            const auto type = std::type_index(typeid(T));
            const auto it = _componentStorage.find(type);

            if (it == _componentStorage.end()) {
                throw std::runtime_error("Component type not registered");
            }

            return *static_cast<IComponentStorage<T> *>(it->second.get());
        }
    };
} // namespace GameEngine