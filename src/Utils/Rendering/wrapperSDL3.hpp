/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** wrapperSDL3 - SDL3 abstraction wrapper
*/

#pragma once

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL.h>
#include <cstdint>
#include <string>

namespace GameEngine
{
    // Forward declarations for generic components
    class SpriteComponent;
    class TransformComponent;
    class TextComponent;
    class CircleComponent;
    class RectangleComponent;

    class SDL3Wrapper {
      public:
        // Initialization and cleanup
        static bool init(std::uint32_t flags);
        static void quit();
        static std::string getError();
        static std::uint64_t getTicksNS();
        static void delay(std::uint32_t milliseconds);

        // Window functions
        static SDL_Window *createWindow(const std::string &title, int width, int height,
            std::uint32_t flags);
        static void destroyWindow(SDL_Window *window);

        // Renderer functions
        static SDL_Renderer *createRenderer(SDL_Window *window, const std::string &driver);
        static void destroyRenderer(SDL_Renderer *renderer);
        static bool setRenderDrawColor(SDL_Renderer *renderer, std::uint8_t red,
            std::uint8_t green, std::uint8_t blue, std::uint8_t alpha);
        static bool renderClear(SDL_Renderer *renderer);
        static void renderPresent(SDL_Renderer *renderer);
        static bool renderFillRect(SDL_Renderer *renderer, const SDL_FRect *rect);
        static bool renderTexture(SDL_Renderer *renderer, SDL_Texture *texture,
            const SDL_FRect *srcRect, const SDL_FRect *dstRect);

        // Texture functions
        static SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &filename);
        static void destroyTexture(SDL_Texture *texture);
        static bool setTextureColorMod(SDL_Texture *texture, std::uint8_t red,
            std::uint8_t green, std::uint8_t blue);
        static bool setTextureAlphaMod(SDL_Texture *texture, std::uint8_t alpha);
        static bool getTextureSize(SDL_Texture *texture, float *width, float *height);
        static bool renderTextureRotated(SDL_Renderer *renderer, SDL_Texture *texture,
            const SDL_FRect *srcRect, const SDL_FRect *dstRect, double angle,
            const SDL_FPoint *center, SDL_FlipMode flip);

        // Generic component rendering (using engine components)
        static bool renderSprite(SDL_Renderer *renderer, const SpriteComponent &sprite,
            const TransformComponent &transform);
        static bool renderText(SDL_Renderer *renderer, SDL_Texture *textTexture,
            const TextComponent &text, const TransformComponent &transform);
        static bool renderCircle(SDL_Renderer *renderer, const CircleComponent &circle,
            const TransformComponent &transform);
        static bool renderRectangle(SDL_Renderer *renderer, const RectangleComponent &rectangle,
            const TransformComponent &transform);

        // Event functions
        static bool pollEvent(SDL_Event *event);
        static const bool *getKeyboardState(int *numkeys);

        // Key checking
        static bool isKeyPressed(SDL_Scancode scancode);
    };
} // namespace GameEngine
