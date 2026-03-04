/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** WindowSystem - Manages window creation and lifecycle via events
*/

#pragma once

#include <GameEngine/EcsManager.hpp>
#include <GameEngine/ISystem.hpp>

namespace GameEngine
{
    class WindowSystem final : public ISystem {
      public:
        explicit WindowSystem(EcsManager &ecsManager);
        ~WindowSystem() override;

        void update(EcsManager &world, float deltaTime) override;

      private:
        EcsManager &_ecsManager;
        bool _initialized;

        // Event handlers
        void onCreateWindow(const struct CreateWindowEvent &event);
        void onDestroyWindow(const struct DestroyWindowEvent &event);
        void onSetWindowTitle(const struct SetWindowTitleEvent &event);
        void onResizeWindow(const struct ResizeWindowEvent &event);

        void cleanup();
    };
} // namespace GameEngine
