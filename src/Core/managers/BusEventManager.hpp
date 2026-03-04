/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BusEventManager
*/

#pragma once

#include <unordered_map>
#include <algorithm>
#include <any>
#include <cstddef>
#include <deque>
#include <functional>
#include <ranges>
#include <string>
#include <vector>
#include <iostream>

namespace GameEngine
{
    using HandlerId = std::size_t;

    struct HandlerOptions {
        int priority = 0;
        std::string category;
    };

    struct Handler {
        std::function<void(const void *)> callback;
        std::function<bool(const void *)> filter;
        HandlerId id = 0;
        int priority = 0;
        bool enabled = true;
        std::string category;
    };

    struct DelayedEvent {
        std::function<void()> dispatcher;
        float remainingTime = 0.0F;
    };

    struct HistoricalEvent {
        std::any eventData;
        std::size_t typeId = 0;
        std::function<void()> dispatcher;
    };

    class BusEventManager {
      public:
        template<typename EventType> void publish(const EventType &event)
        {
            auto typeId = getTypeId<EventType>();

            if (_historyEnabled) {
                HistoricalEvent histEvent;
                histEvent.eventData = event;
                histEvent.typeId = typeId;
                histEvent.dispatcher = [this, event]() {
                    this->publish(event);
                };

                _eventHistory.push_back(histEvent);
                if (_eventHistory.size() > _historyMaxSize) {
                    _eventHistory.pop_front();
                }
            }

            const auto it = _handlers.find(typeId);
            if (it == _handlers.end()) {
                return;
            }

            for (auto &handler : it->second) {
                if (!handler.enabled) {
                    continue;
                }

                if (handler.filter && !handler.filter(&event)) {
                    continue;
                }

                handler.callback(&event);
            }
        }

        template<typename EventType>
        [[nodiscard]] HandlerId subscribe(std::function<void(const EventType &)> callback,
            const HandlerOptions &options = {},
            std::function<bool(const EventType &)> filter = nullptr)
        {
            auto typeId = getTypeId<EventType>();

            Handler handler;
            handler.callback = [callback](const void *data) {
                callback(*static_cast<const EventType *>(data));
            };
            if (filter) {
                handler.filter = [filter](const void *data) {
                    return filter(*static_cast<const EventType *>(data));
                };
            }
            handler.id = _nextHandlerId++;
            handler.priority = options.priority;
            handler.enabled = true;
            handler.category = options.category;

            auto &handlerList = _handlers[typeId];
            auto insertPos = std::lower_bound(handlerList.begin(), handlerList.end(), handler,
                [](const Handler &a, const Handler &b) { return a.priority > b.priority; });
            handlerList.insert(insertPos, handler);

            _handlerTypeMap[handler.id] = typeId;

            return handler.id;
        }

        void unsubscribe(HandlerId id)
        {
            const auto typeIt = _handlerTypeMap.find(id);
            if (typeIt == _handlerTypeMap.end()) {
                return;
            }

            const auto typeId = typeIt->second;

            const auto handlersIt = _handlers.find(typeId);
            if (handlersIt != _handlers.end()) {
                auto &handlerList = handlersIt->second;
                std::erase_if(handlerList, [id](const Handler &h) { return h.id == id; });
            }

            _handlerTypeMap.erase(typeIt);
        }

        template<typename EventType> void publishDeferred(const EventType &event)
        {
            _deferredEvents.push_back([this, event]() { this->publish(event); });
        }

        void processDeferred()
        {
            const auto events = std::move(_deferredEvents);
            _deferredEvents.clear();

            for (const auto &dispatcher : events) {
                dispatcher();
            }
        }

        template<typename EventType> void publishDelayed(const EventType &event, const float delay)
        {
            DelayedEvent delayedEvent;
            delayedEvent.dispatcher = [this, event]() {
                this->publish(event);
            };
            delayedEvent.remainingTime = delay;
            _delayedEvents.push_back(delayedEvent);
        }

        void processDelayed(const float deltaTime)
        {
            for (auto &[dispatcher, remainingTime] : _delayedEvents) {
                remainingTime -= deltaTime;
            }

            std::erase_if(_delayedEvents, [](const DelayedEvent &event) {
                if (event.remainingTime <= 0.0F) {
                    event.dispatcher();
                    return true;
                }
                return false;
            });
        }

        void disableCategory(const std::string &category)
        {
            for (auto &[typeId, handlerList] : _handlers) {
                for (auto &handler : handlerList
                        | std::views::filter(
                            [&category](const Handler &h) { return h.category == category; })) {
                    handler.enabled = false;
                }
            }
        }

        void enableCategory(const std::string &category)
        {
            for (auto &[typeId, handlerList] : _handlers) {
                for (auto &handler : handlerList
                        | std::views::filter(
                            [&category](const Handler &h) { return h.category == category; })) {
                    handler.enabled = true;
                }
            }
        }

        void clearCategory(const std::string &category)
        {
            for (auto &[typeId, handlerList] : _handlers) {
                std::erase_if(handlerList, [&category, this](const Handler &h) {
                    const bool matches = (h.category == category);
                    if (matches) {
                        _handlerTypeMap.erase(h.id);
                    }
                    return matches;
                });
            }
        }

        void disableHandler(const HandlerId id)
        {
            const auto typeIt = _handlerTypeMap.find(id);
            if (typeIt == _handlerTypeMap.end()) {
                return;
            }

            const auto handlersIt = _handlers.find(typeIt->second);
            if (handlersIt == _handlers.end()) {
                return;
            }

            const auto handlerIt = std::ranges::find_if(
                handlersIt->second, [id](const Handler &h) { return h.id == id; });

            if (handlerIt != handlersIt->second.end()) {
                handlerIt->enabled = false;
            }
        }

        void enableHandler(const HandlerId id)
        {
            const auto typeIt = _handlerTypeMap.find(id);
            if (typeIt == _handlerTypeMap.end()) {
                return;
            }

            const auto handlersIt = _handlers.find(typeIt->second);
            if (handlersIt == _handlers.end()) {
                return;
            }

            const auto handlerIt = std::ranges::find_if(
                handlersIt->second, [id](const Handler &h) { return h.id == id; });

            if (handlerIt != handlersIt->second.end()) {
                handlerIt->enabled = true;
            }
        }

        void enableHistory(const std::size_t maxSize)
        {
            _historyEnabled = true;
            _historyMaxSize = maxSize;
            _eventHistory.clear();
        }

        void disableHistory()
        {
            _historyEnabled = false;
            _eventHistory.clear();
        }

        void clearHistory()
        {
            _eventHistory.clear();
        }

        template<typename EventType>
        [[nodiscard]] std::vector<EventType> getRecentEvents(const std::size_t count) const
        {
            auto typeId = getTypeId<EventType>();
            std::vector<EventType> result;
            result.reserve(count);

            for (auto it = _eventHistory.rbegin(); it != _eventHistory.rend() && result.size() < count; ++it) {
                if (it->typeId == typeId) {
                    result.push_back(std::any_cast<EventType>(it->eventData));
                }
            }

            return result;
        }

        template<typename EventType>
        void replayEvents(const std::size_t count)
        {
            auto typeId = getTypeId<EventType>();
            std::size_t replayed = 0;

            for (auto it = _eventHistory.rbegin(); it != _eventHistory.rend() && replayed < count; ++it) {
                if (it->typeId == typeId) {
                    it->dispatcher();
                    ++replayed;
                }
            }
        }

        [[nodiscard]] std::vector<HistoricalEvent> getAllRecentEvents(const std::size_t count) const
        {
            std::vector<HistoricalEvent> result;
            result.reserve(std::min(count, _eventHistory.size()));

            auto it = _eventHistory.rbegin();
            for (std::size_t i = 0; i < count && it != _eventHistory.rend(); ++i, ++it) {
                result.push_back(*it);
            }

            return result;
        }

        void replayAllEvents(const std::size_t count)
        {
            std::size_t replayed = 0;

            for (auto it = _eventHistory.rbegin(); it != _eventHistory.rend() && replayed < count; ++it, ++replayed) {
                it->dispatcher();
            }
        }

      private:
        template<typename T> static std::size_t getTypeId()
        {
            static std::size_t const id = _nextTypeId++;
            return id;
        }

        static inline std::size_t _nextTypeId = 0;
        HandlerId _nextHandlerId = 1;

        std::unordered_map<std::size_t, std::vector<Handler>> _handlers;
        std::unordered_map<HandlerId, std::size_t> _handlerTypeMap;

        std::vector<std::function<void()>> _deferredEvents;
        std::vector<DelayedEvent> _delayedEvents;

        bool _historyEnabled = false;
        std::size_t _historyMaxSize = 0;
        std::deque<HistoricalEvent> _eventHistory;
    };

} // namespace GameEngine
