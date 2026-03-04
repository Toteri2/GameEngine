#pragma once

#include "../../managers/EntityManager.hpp"
#include <cstdint>

namespace GameEngine
{
    class IComponentStorageBase {
      public:
        virtual ~IComponentStorageBase() = default;
        virtual auto removeEntityData(EntityId entity) -> void = 0;
    };
    template<typename T> class IComponentStorage : public IComponentStorageBase {
      public:
        ~IComponentStorage() override = default;
        virtual auto insertData(EntityId entity, T component) -> void = 0;
        virtual auto removeData(EntityId entity) -> void = 0;
        virtual auto getData(EntityId entity) const -> const T & = 0;
        [[nodiscard]] virtual auto hasData(EntityId entity) const -> bool = 0;
        [[nodiscard]] virtual auto getEntities() const -> std::vector<EntityId> = 0;
    };
} // namespace GameEngine
