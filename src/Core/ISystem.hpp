#pragma once

namespace GameEngine
{
    class EcsManager;

    class ISystem {
      public:
        virtual ~ISystem() = default;
        virtual auto update(EcsManager &world, float deltaTime) -> void = 0;
    };
} // namespace GameEngine