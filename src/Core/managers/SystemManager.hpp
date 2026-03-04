#pragma once
#include "../ISystem.hpp"
#include <unordered_map>
#include <cstdio>
#include <exception>
#include <memory>
#include <ranges>
#include <typeindex>

namespace GameEngine
{
    class EcsManager;
    class SystemManager {
      public:
        template<typename T, typename... Args> auto registerSystem(Args &&...args) -> T *
        {
            const std::type_index typeIndex(typeid(T));
            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            T *systemPtr = system.get();
            _systems[typeIndex] = std::move(system);
            return systemPtr;
        }

        auto runSystem(EcsManager &world, const float deltaTime) -> void
        {
            for (const auto &[typeIndex, system] : _systems) {
                try {
                    system->update(world, deltaTime);
                } catch (const std::exception &e) {
                    fprintf(stderr, "Exception in system '%s': %s\n", typeIndex.name(), e.what());
                } catch (...) {
                    fprintf(stderr, "Unknown exception in system '%s'\n", typeIndex.name());
                }
            }
        }

        template<typename T> auto getSystem() -> T &
        {
            const std::type_index typeIndex(typeid(T));
            const auto it = _systems.find(typeIndex);
            if (it == _systems.end()) {
                throw std::runtime_error("System not found");
            }
            return *static_cast<T *>(it->second.get());
        }

      private:
        std::unordered_map<std::type_index, std::unique_ptr<ISystem>> _systems;
    };
} // namespace GameEngine
