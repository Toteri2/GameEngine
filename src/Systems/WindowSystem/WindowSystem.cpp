/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** WindowSystem implementation
*/

#include "WindowSystem.hpp"
#include "../../Utils/Logger/Logger.hpp"
#include "../../Utils/Rendering/wrapperSDL3.hpp"
#include <GameEngine/RenderComponents.hpp>
#include <GameEngine/RenderEventTypes.hpp>
#include <SDL3/SDL_video.h>

namespace GameEngine
{
    WindowSystem::WindowSystem(EcsManager &ecsManager)
        : _ecsManager(ecsManager), _initialized(false)
    {
        // Register components
        ecsManager.registerComponent<WindowComponent>();
        ecsManager.registerComponent<RendererComponent>();

        // Subscribe to events
        ecsManager.subscribeEvent<CreateWindowEvent>(
            [this](const CreateWindowEvent &event) { onCreateWindow(event); });

        ecsManager.subscribeEvent<DestroyWindowEvent>(
            [this](const DestroyWindowEvent &event) { onDestroyWindow(event); });

        ecsManager.subscribeEvent<SetWindowTitleEvent>(
            [this](const SetWindowTitleEvent &event) { onSetWindowTitle(event); });

        ecsManager.subscribeEvent<ResizeWindowEvent>(
            [this](const ResizeWindowEvent &event) { onResizeWindow(event); });

        Logger::info("WindowSystem initialized");
    }

    WindowSystem::~WindowSystem()
    {
        cleanup();
        Logger::info("WindowSystem destroyed");
    }

    void WindowSystem::update(EcsManager &world, float deltaTime)
    {
        (void)world;
        (void)deltaTime;
    }

    void WindowSystem::onCreateWindow(const CreateWindowEvent &event)
    {
        if (_initialized) {
            Logger::warning("WindowSystem: Window already exists, ignoring create request");
            return;
        }

        if (!SDL3Wrapper::init(SDL_INIT_VIDEO)) {
            Logger::error("WindowSystem: Failed to initialize SDL: " + SDL3Wrapper::getError());
            return;
        }

        const auto windowEntityId = _ecsManager.createEntity();

        WindowComponent windowComp;
        windowComp.title = event.title;
        windowComp.width = event.width;
        windowComp.height = event.height;
        windowComp.flags = event.flags;
        windowComp.window = SDL3Wrapper::createWindow(event.title, event.width, event.height,
            event.flags);

        if (windowComp.window == nullptr) {
            Logger::error("WindowSystem: Window creation failed: " + SDL3Wrapper::getError());
            return;
        }

        _ecsManager.addComponent(windowEntityId, windowComp);
        Logger::info("WindowSystem: Window created successfully (" + std::to_string(event.width) +
                     "x" + std::to_string(event.height) + ")");

        _initialized = true;
    }

    void WindowSystem::onDestroyWindow(const DestroyWindowEvent &event)
    {
        (void)event;
        cleanup();
    }

    void WindowSystem::onSetWindowTitle(const SetWindowTitleEvent &event)
    {
        const auto entities = _ecsManager.getAllEntitiesWith<WindowComponent>();
        if (entities.empty()) {
            Logger::warning("WindowSystem: No window to set title");
            return;
        }

        for (const auto entity : entities) {
            auto &window = _ecsManager.getComponent<WindowComponent>(entity);
            if (window.window != nullptr) {
                SDL_SetWindowTitle(static_cast<SDL_Window *>(window.window), event.title.c_str());
                window.title = event.title;
                Logger::info("WindowSystem: Window title set to '" + event.title + "'");
            }
        }
    }

    void WindowSystem::onResizeWindow(const ResizeWindowEvent &event)
    {
        const auto entities = _ecsManager.getAllEntitiesWith<WindowComponent>();
        if (entities.empty()) {
            Logger::warning("WindowSystem: No window to resize");
            return;
        }

        for (const auto entity : entities) {
            auto &window = _ecsManager.getComponent<WindowComponent>(entity);
            if (window.window != nullptr) {
                SDL_SetWindowSize(static_cast<SDL_Window *>(window.window), event.width,
                    event.height);
                window.width = event.width;
                window.height = event.height;
                Logger::info("WindowSystem: Window resized to " + std::to_string(event.width) +
                             "x" + std::to_string(event.height));
            }
        }
    }

    void WindowSystem::cleanup()
    {
        const auto entities = _ecsManager.getAllEntitiesWith<WindowComponent>();

        for (const auto entity : entities) {
            if (_ecsManager.hasComponent<WindowComponent>(entity)) {
                auto &window = _ecsManager.getComponent<WindowComponent>(entity);
                if (window.window != nullptr) {
                    SDL3Wrapper::destroyWindow(static_cast<SDL_Window *>(window.window));
                    window.window = nullptr;
                    Logger::info("WindowSystem: Window destroyed");
                }
            }
        }

        if (_initialized) {
            SDL3Wrapper::quit();
            _initialized = false;
        }
    }
} // namespace GameEngine
