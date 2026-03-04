#pragma once
#include "IComponent.hpp"

namespace GameEngine
{
    class ExampleComponents final : public IComponent {
      public:
        int x = 0;
        int y = 0;
    };
} // namespace GameEngine
