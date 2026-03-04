/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** RenderSystem
*/

#pragma once

#include <GameEngine/EcsManager.hpp>
#include <GameEngine/ISystem.hpp>
#include <memory>

namespace GameEngine
{
    class RenderManager;
    class TextComponent;

    class RenderSystem final : public ISystem {
      public:
        explicit RenderSystem(EcsManager &ecsManager);
        ~RenderSystem() override;

        void update(EcsManager &world, float deltaTime) override;

      private:
        EcsManager &_ecsManager;
        bool _initialized;
        std::shared_ptr<RenderManager> _renderManager;
        void *_cachedRenderer = nullptr;

        void onCreateWindow(const struct CreateWindowEvent &event) const;
        void onCreateRenderer(const struct CreateRendererEvent &event);
        void onSetRenderDrawColor(const struct SetRenderDrawColorEvent &event) const;
        void onLoadTexture(const struct LoadTextureEvent &event) const;
        void onUnloadTexture(const struct UnloadTextureEvent &event) const;
        void onLoadFont(const struct LoadFontEvent &event) const;
        void onUnloadFont(const struct UnloadFontEvent &event) const;
        void onCreateTextTexture(const struct CreateTextTextureEvent &event) const;

        void renderFrame(EcsManager &world);
        void renderAllByLayers(EcsManager &world, void *renderer);

        void cleanup();

        void *getOrCreateTextTexture(EntityId entity, TextComponent &text, void *renderer, RenderManager *renderManager);
    };
} // namespace GameEngine
