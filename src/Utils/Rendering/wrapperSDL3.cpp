/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** wrapperSDL3 implementation
*/

#include "wrapperSDL3.hpp"
#include <GameEngine/RenderComponents.hpp>
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3_gfxPrimitives.h>
#include <SDL3_image/SDL_image.h>
#include <cstdint>
#include <string>

namespace GameEngine
{
    bool SDL3Wrapper::init(const std::uint32_t flags)
    {
        return SDL_Init(flags);
    }

    void SDL3Wrapper::quit()
    {
        SDL_Quit();
    }

    std::string SDL3Wrapper::getError()
    {
        return SDL_GetError();
    }

    std::uint64_t SDL3Wrapper::getTicksNS()
    {
        return SDL_GetTicksNS();
    }

    void SDL3Wrapper::delay(const std::uint32_t milliseconds)
    {
        SDL_Delay(milliseconds);
    }

    // Window functions

    SDL_Window *SDL3Wrapper::createWindow(const std::string &title, const int width,
        const int height, const std::uint32_t flags)
    {
        return SDL_CreateWindow(title.c_str(), width, height, flags);
    }

    void SDL3Wrapper::destroyWindow(SDL_Window *window)
    {
        SDL_DestroyWindow(window);
    }

    // Renderer functions

    SDL_Renderer *SDL3Wrapper::createRenderer(SDL_Window *window, const std::string &driver)
    {
        return SDL_CreateRenderer(window, driver.c_str());
    }

    void SDL3Wrapper::destroyRenderer(SDL_Renderer *renderer)
    {
        SDL_DestroyRenderer(renderer);
    }

    bool SDL3Wrapper::setRenderDrawColor(SDL_Renderer *renderer, const std::uint8_t red,
        const std::uint8_t green, const std::uint8_t blue, const std::uint8_t alpha)
    {
        return SDL_SetRenderDrawColor(renderer, red, green, blue, alpha);
    }

    bool SDL3Wrapper::renderClear(SDL_Renderer *renderer)
    {
        return SDL_RenderClear(renderer);
    }

    void SDL3Wrapper::renderPresent(SDL_Renderer *renderer)
    {
        SDL_RenderPresent(renderer);
    }

    bool SDL3Wrapper::renderFillRect(SDL_Renderer *renderer, const SDL_FRect *rect)
    {
        return SDL_RenderFillRect(renderer, rect);
    }

    bool SDL3Wrapper::renderTexture(SDL_Renderer *renderer, SDL_Texture *texture,
        const SDL_FRect *srcRect, const SDL_FRect *dstRect)
    {
        return SDL_RenderTexture(renderer, texture, srcRect, dstRect);
    }

    // Texture functions

    SDL_Texture *SDL3Wrapper::loadTexture(SDL_Renderer *renderer, const std::string &filename)
    {
        return IMG_LoadTexture(renderer, filename.c_str());
    }

    void SDL3Wrapper::destroyTexture(SDL_Texture *texture)
    {
        SDL_DestroyTexture(texture);
    }

    bool SDL3Wrapper::setTextureColorMod(SDL_Texture *texture, const std::uint8_t red,
        const std::uint8_t green, const std::uint8_t blue)
    {
        return SDL_SetTextureColorMod(texture, red, green, blue);
    }

    bool SDL3Wrapper::setTextureAlphaMod(SDL_Texture *texture, const std::uint8_t alpha)
    {
        return SDL_SetTextureAlphaMod(texture, alpha);
    }

    bool SDL3Wrapper::getTextureSize(SDL_Texture *texture, float *width, float *height)
    {
        return SDL_GetTextureSize(texture, width, height);
    }

    bool SDL3Wrapper::renderTextureRotated(SDL_Renderer *renderer, SDL_Texture *texture,
        const SDL_FRect *srcRect, const SDL_FRect *dstRect, const double angle,
        const SDL_FPoint *center, const SDL_FlipMode flip)
    {
        return SDL_RenderTextureRotated(renderer, texture, srcRect, dstRect, angle, center, flip);
    }

    // Event functions

    bool SDL3Wrapper::pollEvent(SDL_Event *event)
    {
        return SDL_PollEvent(event);
    }

    const bool *SDL3Wrapper::getKeyboardState(int *numkeys)
    {
        return SDL_GetKeyboardState(numkeys);
    }

    // Key checking

    bool SDL3Wrapper::isKeyPressed(const SDL_Scancode scancode)
    {
        const bool *keystate = SDL_GetKeyboardState(nullptr);
        return (keystate != nullptr) ? keystate[scancode] : false;
    }

    // Component-based rendering functions

    bool SDL3Wrapper::renderSprite(SDL_Renderer *renderer, const SpriteComponent &sprite,
        const TransformComponent &transform)
    {
        if (renderer == nullptr) {
            SDL_Log("renderSprite: renderer is null");
            return false;
        }

        if (sprite.texture == nullptr) {
            SDL_Log("renderSprite: sprite.texture is null");
            return false;
        }

        auto *texture = static_cast<SDL_Texture *>(sprite.texture);

        float texW = 0;
        float texH = 0;
        SDL_GetTextureSize(texture, &texW, &texH);

        float const srcW = (sprite.srcW == 0.0F || sprite.srcW > texW) ? texW : sprite.srcW;
        float const srcH = (sprite.srcH == 0.0F || sprite.srcH > texH) ? texH : sprite.srcH;

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        SDL_FRect const srcRect = {sprite.srcX, sprite.srcY, srcW, srcH};
        SDL_FRect const dstRect = {transform.x, transform.y, sprite.dstW * transform.scaleX,
            sprite.dstH * transform.scaleY};

        if (!setTextureColorMod(texture, sprite.colorR, sprite.colorG, sprite.colorB)) {
            SDL_Log("renderSprite: setTextureColorMod failed: %s", SDL_GetError());
            return false;
        }

        if (!setTextureAlphaMod(texture, sprite.alpha)) {
            SDL_Log("renderSprite: setTextureAlphaMod failed: %s", SDL_GetError());
            return false;
        }

        SDL_FlipMode flip = SDL_FLIP_NONE;
        if (sprite.flip == SpriteComponent::FlipMode::HORIZONTAL) {
            flip = SDL_FLIP_HORIZONTAL;
        } else if (sprite.flip == SpriteComponent::FlipMode::VERTICAL) {
            flip = SDL_FLIP_VERTICAL;
        }

        const bool result = renderTextureRotated(renderer, texture, &srcRect, &dstRect, transform.rotation,
            nullptr, flip);

        if (!result) {
            SDL_Log("renderSprite: renderTextureRotated failed: %s", SDL_GetError());
        }

        return result;
    }

    bool SDL3Wrapper::renderText(SDL_Renderer *renderer, SDL_Texture *textTexture,
        const TextComponent &text, const TransformComponent &transform)
    {
        if (renderer == nullptr || textTexture == nullptr) {
            return false;
        }

        float width = 0.0F;
        float height = 0.0F;
        getTextureSize(textTexture, &width, &height);

        SDL_FRect const dstRect = {transform.x, transform.y, width * transform.scaleX,
            height * transform.scaleY};

        if (!setTextureColorMod(textTexture, text.colorR, text.colorG, text.colorB)) {
            return false;
        }

        if (!setTextureAlphaMod(textTexture, text.alpha)) {
            return false;
        }

        return renderTextureRotated(renderer, textTexture, nullptr, &dstRect, transform.rotation,
            nullptr, SDL_FLIP_NONE);
    }

    bool SDL3Wrapper::renderCircle(SDL_Renderer *renderer, const CircleComponent &circle,
        const TransformComponent &transform)
    {
        if (renderer == nullptr) {
            return false;
        }

        const auto xPos = static_cast<std::int16_t>(transform.x);
        const auto yPos = static_cast<std::int16_t>(transform.y);
        const auto radius = static_cast<std::int16_t>(circle.radius * transform.scaleX);

        if (circle.filled) {
            return filledCircleRGBA(renderer, xPos, yPos, radius, circle.colorR, circle.colorG,
                circle.colorB, circle.alpha);
        }
        return circleRGBA(renderer, xPos, yPos, radius, circle.colorR, circle.colorG,
            circle.colorB, circle.alpha);
    }

    bool SDL3Wrapper::renderRectangle(SDL_Renderer *renderer, const RectangleComponent &rectangle,
        const TransformComponent &transform)
    {
        if (renderer == nullptr) {
            return false;
        }

        const float scaledWidth = rectangle.width * transform.scaleX;
        const float scaledHeight = rectangle.height * transform.scaleY;

        SDL_FRect const rect = {transform.x - (scaledWidth / 2.0F),
            transform.y - (scaledHeight / 2.0F), scaledWidth, scaledHeight};

        SDL_BlendMode previousBlendMode;
        SDL_GetRenderDrawBlendMode(renderer, &previousBlendMode);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        setRenderDrawColor(renderer, rectangle.colorR, rectangle.colorG, rectangle.colorB,
            rectangle.alpha);

        bool result;
        if (rectangle.filled) {
            result = SDL_RenderFillRect(renderer, &rect);
        } else {
            result = SDL_RenderRect(renderer, &rect);
        }

        SDL_SetRenderDrawBlendMode(renderer, previousBlendMode);
        return result;
    }
} // namespace GameEngine
