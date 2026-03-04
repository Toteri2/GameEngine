/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** RenderSystem implementation
*/

#include "RenderSystem.hpp"
#include "../../Utils/Logger/Logger.hpp"
#include "../../Utils/Rendering/RenderManager.hpp"
#include "../../Utils/Rendering/wrapperSDL3.hpp"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <GameEngine/RenderComponents.hpp>
#include <GameEngine/RenderEventTypes.hpp>
#include <algorithm>
#include <memory>
#include <set>
#include <vector>

namespace GameEngine
{
    RenderSystem::RenderSystem(EcsManager &ecsManager)
        : _ecsManager(ecsManager), _initialized(false)
    {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            Logger::error("RenderSystem: Failed to initialize SDL: " + SDL3Wrapper::getError());
            return;
        }

        if (!TTF_Init()) {
            Logger::error(
                "RenderSystem: Failed to initialize SDL_ttf: " + SDL3Wrapper::getError());
            SDL_Quit();
            return;
        }
        _renderManager = std::make_shared<RenderManager>();

        ecsManager.registerComponent<RendererComponent>();
        ecsManager.registerComponent<WindowComponent>();
        ecsManager.registerComponent<TransformComponent>();
        ecsManager.registerComponent<SpriteComponent>();
        ecsManager.registerComponent<TextComponent>();
        ecsManager.registerComponent<CircleComponent>();
        ecsManager.registerComponent<RectangleComponent>();
        ecsManager.registerComponent<AnimationComponent>();
        ecsManager.registerComponent<CameraComponent>();
        ecsManager.registerComponent<ButtonComponent>();

        ecsManager.subscribeEvent<CreateWindowEvent>(
            [this](const CreateWindowEvent &event) { onCreateWindow(event); });

        ecsManager.subscribeEvent<CreateRendererEvent>(
            [this](const CreateRendererEvent &event) { onCreateRenderer(event); });

        ecsManager.subscribeEvent<SetRenderDrawColorEvent>(
            [this](const SetRenderDrawColorEvent &event) { onSetRenderDrawColor(event); });

        ecsManager.subscribeEvent<LoadTextureEvent>(
            [this](const LoadTextureEvent &event) { onLoadTexture(event); });

        ecsManager.subscribeEvent<UnloadTextureEvent>(
            [this](const UnloadTextureEvent &event) { onUnloadTexture(event); });

        ecsManager.subscribeEvent<LoadFontEvent>(
            [this](const LoadFontEvent &event) { onLoadFont(event); });

        ecsManager.subscribeEvent<UnloadFontEvent>(
            [this](const UnloadFontEvent &event) { onUnloadFont(event); });

        ecsManager.subscribeEvent<CreateTextTextureEvent>(
            [this](const CreateTextTextureEvent &event) { onCreateTextTexture(event); });

        _initialized = true;
        Logger::info("RenderSystem initialized successfully");
    }

    RenderSystem::~RenderSystem()
    {
        cleanup();
        TTF_Quit();
        SDL_Quit();
        Logger::info("RenderSystem destroyed");
    }

    void RenderSystem::update(EcsManager &world, const float deltaTime)
    {
        (void) deltaTime;
        renderFrame(world);
    }

    void RenderSystem::onCreateWindow(const CreateWindowEvent &event) const
    {
        const auto windowEntities = _ecsManager.getAllEntitiesWith<WindowComponent>();
        if (!windowEntities.empty()) {
            Logger::warning("RenderSystem: Window already exists");
            return;
        }

        void *window =
            SDL3Wrapper::createWindow(event.title, event.width, event.height, event.flags);
        if (window == nullptr) {
            Logger::error("RenderSystem: Failed to create window: " + SDL3Wrapper::getError());
            return;
        }

        const EntityId windowEntity = _ecsManager.createEntity();
        WindowComponent windowComp;
        windowComp.window = window;
        windowComp.baseWidth = event.width;
        windowComp.baseHeight = event.height;
        windowComp.title = event.title;

        _ecsManager.addComponent(windowEntity, windowComp);
        Logger::info("RenderSystem: Window created successfully");
    }

    void RenderSystem::onCreateRenderer(const CreateRendererEvent &event)
    {
        const auto windowEntities = _ecsManager.getAllEntitiesWith<WindowComponent>();
        if (windowEntities.empty()) {
            Logger::error("RenderSystem: No window found to create renderer");
            return;
        }

        const auto &windowComp = _ecsManager.getComponent<WindowComponent>(windowEntities[0]);
        if (windowComp.window == nullptr) {
            Logger::error("RenderSystem: Window is null, cannot create renderer");
            return;
        }

        RendererComponent rendererComp;
        rendererComp.renderer = SDL3Wrapper::createRenderer(
            static_cast<SDL_Window *>(windowComp.window), event.driver);

        if (rendererComp.renderer == nullptr) {
            Logger::error("RenderSystem: Failed to create renderer: " + SDL3Wrapper::getError());
            return;
        }

        rendererComp.renderManager = _renderManager.get();
        SDL_SetRenderLogicalPresentation(static_cast<SDL_Renderer *>(rendererComp.renderer),
            windowComp.baseWidth, windowComp.baseHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);

        _ecsManager.addComponent(windowEntities[0], rendererComp);
        _cachedRenderer = rendererComp.renderer;

        Logger::info("RenderSystem: Renderer created successfully");
    }

    void RenderSystem::onSetRenderDrawColor(const SetRenderDrawColorEvent &event) const
    {
        const auto rendererEntities = _ecsManager.getAllEntitiesWith<RendererComponent>();
        if (rendererEntities.empty()) {
            return;
        }

        auto &rendererComp = _ecsManager.getComponent<RendererComponent>(rendererEntities[0]);
        rendererComp.clearColorR = event.r;
        rendererComp.clearColorG = event.g;
        rendererComp.clearColorB = event.b;
        rendererComp.clearColorA = event.a;
    }

    void RenderSystem::onLoadTexture(const LoadTextureEvent &event) const
    {
        const auto rendererEntities = _ecsManager.getAllEntitiesWith<RendererComponent>();
        if (rendererEntities.empty()) {
            Logger::error("RenderSystem: No renderer to load texture");
            return;
        }

        const auto &rendererComp =
            _ecsManager.getComponent<RendererComponent>(rendererEntities[0]);
        auto *renderManager = static_cast<RenderManager *>(rendererComp.renderManager);
        if (renderManager == nullptr) {
            Logger::error("RenderSystem: RenderManager is null");
            return;
        }

        renderManager->loadTexture(
            static_cast<SDL_Renderer *>(rendererComp.renderer), event.textureId, event.filename);
    }

    void RenderSystem::onUnloadTexture(const UnloadTextureEvent &event) const
    {
        const auto rendererEntities = _ecsManager.getAllEntitiesWith<RendererComponent>();
        if (rendererEntities.empty()) {
            return;
        }

        const auto &rendererComp =
            _ecsManager.getComponent<RendererComponent>(rendererEntities[0]);
        auto *renderManager = static_cast<RenderManager *>(rendererComp.renderManager);
        if (renderManager != nullptr) {
            renderManager->removeTexture(event.textureId);
        }
    }

    void RenderSystem::onLoadFont(const LoadFontEvent &event) const
    {
        const auto rendererEntities = _ecsManager.getAllEntitiesWith<RendererComponent>();
        if (rendererEntities.empty()) {
            Logger::error("RenderSystem: No renderer to load font");
            return;
        }

        const auto &rendererComp =
            _ecsManager.getComponent<RendererComponent>(rendererEntities[0]);
        auto *renderManager = static_cast<RenderManager *>(rendererComp.renderManager);
        if (renderManager == nullptr) {
            Logger::error("RenderSystem: RenderManager is null");
            return;
        }

        renderManager->loadFont(event.fontId, event.filename, event.fontSize);
    }

    void RenderSystem::onUnloadFont(const UnloadFontEvent &event) const
    {
        const auto rendererEntities = _ecsManager.getAllEntitiesWith<RendererComponent>();
        if (rendererEntities.empty()) {
            return;
        }

        const auto &rendererComp =
            _ecsManager.getComponent<RendererComponent>(rendererEntities[0]);
        auto *renderManager = static_cast<RenderManager *>(rendererComp.renderManager);
        if (renderManager != nullptr) {
            renderManager->removeFont(event.fontId);
        }
    }

    void RenderSystem::onCreateTextTexture(const CreateTextTextureEvent &event) const
    {
        const auto rendererEntities = _ecsManager.getAllEntitiesWith<RendererComponent>();
        if (rendererEntities.empty()) {
            Logger::error("RenderSystem: No renderer to create text texture");
            return;
        }

        const auto &rendererComp =
            _ecsManager.getComponent<RendererComponent>(rendererEntities[0]);
        auto *renderManager = static_cast<RenderManager *>(rendererComp.renderManager);
        if (renderManager == nullptr) {
            Logger::error("RenderSystem: RenderManager is null");
            return;
        }

        const SDL_Color color = {event.r, event.g, event.b, event.a};
        renderManager->getOrCreateTextTexture(static_cast<SDL_Renderer *>(rendererComp.renderer),
            event.textTextureId, event.fontId, event.text, color);
    }

    void RenderSystem::renderFrame(EcsManager &world)
    {
        const auto rendererEntities = world.getAllEntitiesWith<RendererComponent>();
        if (rendererEntities.empty()) {
            return;
        }

        const auto &rendererComp = world.getComponent<RendererComponent>(rendererEntities[0]);
        auto *renderer = static_cast<SDL_Renderer *>(rendererComp.renderer);

        if (renderer == nullptr) {
            return;
        }

        SDL3Wrapper::setRenderDrawColor(renderer, rendererComp.clearColorR,
            rendererComp.clearColorG, rendererComp.clearColorB, rendererComp.clearColorA);
        SDL3Wrapper::renderClear(renderer);

        renderAllByLayers(world, renderer);
        SDL3Wrapper::renderPresent(renderer);
    }

    void RenderSystem::renderAllByLayers(EcsManager &world, void *renderer)
    {
        auto *renderManager = _renderManager.get();

        struct RenderableEntity {
            EntityId id;
            int layer;
            enum Type { SPRITE, TEXT, CIRCLE, RECTANGLE } type;
        };

        std::vector<RenderableEntity> renderables;

        const auto spriteEntities =
            world.getAllEntitiesWith<SpriteComponent, TransformComponent>();
        for (const auto entity : spriteEntities) {
            const auto &sprite = world.getComponent<SpriteComponent>(entity);
            renderables.push_back({entity, sprite.layer, RenderableEntity::SPRITE});
        }

        const auto textEntities = world.getAllEntitiesWith<TextComponent, TransformComponent>();
        for (const auto entity : textEntities) {
            const auto &text = world.getComponent<TextComponent>(entity);
            renderables.push_back({entity, text.layer, RenderableEntity::TEXT});
        }

        const auto circleEntities =
            world.getAllEntitiesWith<CircleComponent, TransformComponent>();
        for (const auto entity : circleEntities) {
            const auto &circle = world.getComponent<CircleComponent>(entity);
            renderables.push_back({entity, circle.layer, RenderableEntity::CIRCLE});
        }

        const auto rectangleEntities =
            world.getAllEntitiesWith<RectangleComponent, TransformComponent>();
        for (const auto entity : rectangleEntities) {
            const auto &rectangle = world.getComponent<RectangleComponent>(entity);
            renderables.push_back({entity, rectangle.layer, RenderableEntity::RECTANGLE});
        }

        std::sort(renderables.begin(), renderables.end(),
            [](const RenderableEntity &a, const RenderableEntity &b) {
                return a.layer < b.layer;
            });

        for (const auto &renderable : renderables) {
            switch (renderable.type) {
                case RenderableEntity::SPRITE: {
                    auto &sprite = world.getComponent<SpriteComponent>(renderable.id);
                    const auto &transform = world.getComponent<TransformComponent>(renderable.id);

                    if (!sprite.textureId.empty() && renderManager != nullptr) {
                        if (sprite.textureId != sprite.cachedTextureId) {
                            sprite.texture = renderManager->getTexture(sprite.textureId);
                            sprite.cachedTextureId = sprite.textureId;
                        }
                    }

                    if (sprite.texture != nullptr) {
                        SDL3Wrapper::renderSprite(
                            static_cast<SDL_Renderer *>(renderer), sprite, transform);
                    }
                    break;
                }

                case RenderableEntity::TEXT: {
                    auto &text = world.getComponent<TextComponent>(renderable.id);
                    const auto &transform = world.getComponent<TransformComponent>(renderable.id);

                    void *textTexture =
                        getOrCreateTextTexture(renderable.id, text, renderer, renderManager);
                    if (textTexture != nullptr) {
                        SDL3Wrapper::renderText(static_cast<SDL_Renderer *>(renderer),
                            static_cast<SDL_Texture *>(textTexture), text, transform);
                    }
                    break;
                }

                case RenderableEntity::CIRCLE: {
                    const auto &circle = world.getComponent<CircleComponent>(renderable.id);
                    const auto &transform = world.getComponent<TransformComponent>(renderable.id);
                    SDL3Wrapper::renderCircle(
                        static_cast<SDL_Renderer *>(renderer), circle, transform);
                    break;
                }

                case RenderableEntity::RECTANGLE: {
                    const auto &rectangle = world.getComponent<RectangleComponent>(renderable.id);
                    const auto &transform = world.getComponent<TransformComponent>(renderable.id);
                    SDL3Wrapper::renderRectangle(
                        static_cast<SDL_Renderer *>(renderer), rectangle, transform);
                    break;
                }
            }
        }
    }

    void *RenderSystem::getOrCreateTextTexture(
        const EntityId entity, TextComponent &text, void *renderer, RenderManager *renderManager)
    {
        if (renderManager == nullptr || text.fontId.empty()) {
            return nullptr;
        }

        if (text.textTextureId.empty()) {
            text.textTextureId = "text_" + std::to_string(entity);
        }

        const bool textureExists = renderManager->hasTextTexture(text.textTextureId);
        const bool textContentChanged = (text.text != text.cachedText);

        if (!textureExists || (text.needsUpdate && textContentChanged)) {
            if (textureExists) {
                renderManager->removeTextTexture(text.textTextureId);
            }

            constexpr SDL_Color whiteColor = {255, 255, 255, 255};
            void *newTexture =
                renderManager->getOrCreateTextTexture(static_cast<SDL_Renderer *>(renderer),
                    text.textTextureId, text.fontId, text.text, whiteColor);

            if (newTexture != nullptr) {
                text.cachedText = text.text;
                text.needsUpdate = false;
            }
            return newTexture;
        } else {
            if (text.needsUpdate) {
                text.needsUpdate = false;
            }
            return renderManager->getTextTexture(text.textTextureId);
        }
    }

    void RenderSystem::cleanup()
    {
        const auto rendererEntities = _ecsManager.getAllEntitiesWith<RendererComponent>();

        for (const auto entity : rendererEntities) {
            auto &renderer = _ecsManager.getComponent<RendererComponent>(entity);

            renderer.renderManager = nullptr;

            if (renderer.renderer != nullptr) {
                SDL3Wrapper::destroyRenderer(static_cast<SDL_Renderer *>(renderer.renderer));
                renderer.renderer = nullptr;
                Logger::info("RenderSystem: Renderer destroyed");
            }
        }

        _renderManager.reset();
        Logger::info("RenderSystem: RenderManager destroyed");
    }
} // namespace GameEngine
