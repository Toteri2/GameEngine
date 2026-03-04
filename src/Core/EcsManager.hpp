/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EcsManager
*/

#pragma once
#include "managers/BusEventManager.hpp"
#include "managers/ComponentManager.hpp"
#include "managers/EntityManager.hpp"
#include "managers/SystemManager.hpp"
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <vector>

namespace GameEngine
{
    class EcsManager final {
      public:
        EcsManager()
            : _componentManager(std::make_unique<ComponentManager>()),
              _entityManager(std::make_unique<EntityManager>()),
              _busEventManager(std::make_unique<BusEventManager>()),
              _systemManager(std::make_unique<SystemManager>())
        {}

        ~EcsManager() = default;

        EcsManager(const EcsManager &) = delete;
        auto operator=(const EcsManager &) -> EcsManager & = delete;

        EcsManager(EcsManager &&) noexcept = default;
        auto operator=(EcsManager &&) noexcept -> EcsManager & = default;

        [[nodiscard]] auto createEntity() const -> EntityId
        {
            return this->_entityManager->createEntity();
        }

        auto removeEntity(const EntityId entity) -> void
        {
            this->_componentManager->removeAllComponents(entity);
            this->_entityManager->removeEntity(entity);
        }

        template<typename T> auto registerComponent() const -> void
        {
            this->_componentManager->registerComponent<T>();
        }

        template<typename T> auto addComponent(const EntityId entity, T component) -> void
        {
            this->_componentManager->addComponent<T>(entity, component);
        }

        template<typename T> auto removeComponent(EntityId entity) -> void
        {
            this->_componentManager->removeComponent<T>(entity);
        }

        template<typename T> auto getComponent(const EntityId entity) -> T &
        {
            return this->_componentManager->getComponent<T>(entity);
        }

        template<typename T> [[nodiscard]] auto hasComponent(const EntityId entity) const -> bool
        {
            return this->_componentManager->hasComponent<T>(entity);
        }

        template<typename T> [[nodiscard]] auto getComponentType() const -> uint8_t
        {
            return this->_componentManager->getComponentType<T>();
        }

        template<typename T, typename... Args> auto registerSystem(Args &&...args) -> T *
        {
            return this->_systemManager->registerSystem<T>(std::forward<Args>(args)...);
        }

        template<typename T> auto getSystem() -> T &
        {
            return this->_systemManager->getSystem<T>();
        }

        auto runEngine(const float deltaTime) -> void
        {
            this->_busEventManager->processDelayed(deltaTime);
            this->_busEventManager->processDeferred();
            this->_systemManager->runSystem(*this, deltaTime);
        }

        template<typename... ComponentTypes>
        [[nodiscard]] auto getAllEntitiesWith() const -> std::vector<EntityId>
        {
            constexpr auto numComponents = sizeof...(ComponentTypes);

            if constexpr (numComponents == 0) {
                return {};
            } else if constexpr (numComponents == 1) {
                return getSingleComponentEntities<ComponentTypes...>();
            } else {
                return getIntersectedEntities<ComponentTypes...>();
            }
        }

        template<typename EventType> auto publishEvent(const EventType &event) -> void
        {
            this->_busEventManager->publish<EventType>(event);
        }

        template<typename EventType> auto publishEventDeferred(const EventType &event) -> void
        {
            this->_busEventManager->publishDeferred<EventType>(event);
        }

        template<typename EventType>
        auto publishEventDelayed(const EventType &event, const float delay) -> void
        {
            this->_busEventManager->publishDelayed<EventType>(event, delay);
        }

        template<typename EventType>
        [[nodiscard]] auto subscribeEvent(std::function<void(const EventType &)> callback,
            const HandlerOptions options = {},
            std::function<bool(const EventType &)> filter = nullptr) -> HandlerId
        {
            return this->_busEventManager->subscribe<EventType>(callback, options, filter);
        }

        auto unsubscribeEvent(const HandlerId id) const -> void
        {
            this->_busEventManager->unsubscribe(id);
        }

        auto disableCategoryEvent(const std::string &category) const -> void
        {
            this->_busEventManager->disableCategory(category);
        }

        auto enableCategoryEvent(const std::string &category) const -> void
        {
            this->_busEventManager->enableCategory(category);
        }

        auto clearCategoryEvent(const std::string &category) const -> void
        {
            this->_busEventManager->clearCategory(category);
        }

        auto disableHandlerEvent(const HandlerId id) const -> void
        {
            this->_busEventManager->disableHandler(id);
        }

        auto enableHandlerEvent(const HandlerId id) const -> void
        {
            this->_busEventManager->enableHandler(id);
        }

        auto enableEventHistory(const std::size_t maxSize) const -> void
        {
            this->_busEventManager->enableHistory(maxSize);
        }

        auto disableEventHistory() const -> void
        {
            this->_busEventManager->disableHistory();
        }

        auto clearEventHistory() const -> void
        {
            this->_busEventManager->clearHistory();
        }

        template<typename EventType>
        [[nodiscard]] auto getRecentEvents(const std::size_t count) const -> std::vector<EventType>
        {
            return this->_busEventManager->getRecentEvents<EventType>(count);
        }

        template<typename EventType>
        auto replayEvents(const std::size_t count) const -> void
        {
            this->_busEventManager->replayEvents<EventType>(count);
        }

        [[nodiscard]] auto getAllRecentEvents(const std::size_t count) const -> std::vector<HistoricalEvent>
        {
            return this->_busEventManager->getAllRecentEvents(count);
        }

        auto replayAllEvents(const std::size_t count) const -> void
        {
            this->_busEventManager->replayAllEvents(count);
        }

      private:
        template<typename T>
        [[nodiscard]] auto getSingleComponentEntities() const -> std::vector<EntityId>
        {
            return _componentManager->getEntityList<T>();
        }

        template<typename... ComponentTypes>
        [[nodiscard]] auto getIntersectedEntities() const -> std::vector<EntityId>
        {
            std::vector<std::vector<EntityId>> entityLists = {
                _componentManager->getEntityList<ComponentTypes>()...};
            auto smallestIt = std::min_element(entityLists.begin(), entityLists.end(),
                [](const auto &a, const auto &b) { return a.size() < b.size(); });

            if (smallestIt->empty())
                return {};

            std::vector<std::unordered_set<EntityId>> entitySets;
            entitySets.reserve(entityLists.size() - 1);
            for (const auto &list : entityLists) {
                if (&list != &(*smallestIt)) {
                    entitySets.emplace_back(list.begin(), list.end());
                }
            }
            std::vector<EntityId> result;
            result.reserve(smallestIt->size());

            for (EntityId entity : *smallestIt) {
                bool hasAll = true;
                for (const auto &set : entitySets) {
                    if (!set.contains(entity)) {
                        hasAll = false;
                        break;
                    }
                }
                if (hasAll) {
                    result.push_back(entity);
                }
            }

            return result;
        }

        std::unique_ptr<ComponentManager> _componentManager;
        std::unique_ptr<EntityManager> _entityManager;
        std::unique_ptr<SystemManager> _systemManager;
        std::unique_ptr<BusEventManager> _busEventManager;
    };
} // namespace GameEngine
