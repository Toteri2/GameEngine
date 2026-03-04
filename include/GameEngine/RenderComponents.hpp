/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** RenderComponents
*/

#pragma once

#include <GameEngine/IComponent.hpp>
#include <cstdint>
#include <string>

namespace GameEngine
{
    class TransformComponent final : public IComponent {
      public:
        float x = 0.0F;
        float y = 0.0F;
        float rotation = 0.0F;
        float scaleX = 1.0F;
        float scaleY = 1.0F;

        TransformComponent() = default;
        TransformComponent(float posX, float posY, float rot = 0.0F, float sX = 1.0F,
            float sY = 1.0F)
            : x(posX), y(posY), rotation(rot), scaleX(sX), scaleY(sY)
        {}
    };

    class WindowComponent final : public IComponent {
      public:
        void *window = nullptr;
        int width = 1920;
        int height = 1080;
        int baseWidth = 1280;
        int baseHeight = 720;
        std::string title = "Game Window";
        std::uint32_t flags = 0;
    };

    class RendererComponent final : public IComponent {
      public:
        void *renderer = nullptr;
        void *renderManager = nullptr;
        std::uint8_t clearColorR = 0;
        std::uint8_t clearColorG = 0;
        std::uint8_t clearColorB = 0;
        std::uint8_t clearColorA = 255;
    };

    class SpriteComponent final : public IComponent {
      public:
        void *texture = nullptr;
        std::string textureId = "";
        std::string cachedTextureId = "";

        // Source rectangle (from texture)
        float srcX = 0.0F;
        float srcY = 0.0F;
        float srcW = 0.0F;
        float srcH = 0.0F;

        // Destination rectangle (on screen)
        float dstX = 0.0F;
        float dstY = 0.0F;
        float dstW = 0.0F;
        float dstH = 0.0F;

        enum class FlipMode { NONE = 0, HORIZONTAL = 1, VERTICAL = 2 };
        FlipMode flip = FlipMode::NONE;

        std::uint8_t colorR = 255;
        std::uint8_t colorG = 255;
        std::uint8_t colorB = 255;
        std::uint8_t alpha = 255;
        int layer = 0;
    };

    class TextComponent final : public IComponent {
      public:
        std::string fontId = "";
        std::string text = "";
        std::string textTextureId = "";
        std::uint8_t colorR = 255;
        std::uint8_t colorG = 255;
        std::uint8_t colorB = 255;
        std::uint8_t alpha = 255;
        bool needsUpdate = true;
        int layer = 10;
        std::string cachedText = "";
    };

    class CircleComponent final : public IComponent {
      public:
        float radius = 10.0F;
        std::uint8_t colorR = 255;
        std::uint8_t colorG = 255;
        std::uint8_t colorB = 255;
        std::uint8_t alpha = 255;
        bool filled = true;
        int layer = 5;
    };

    class RectangleComponent final : public IComponent {
      public:
        float width = 32.0F;
        float height = 32.0F;
        std::uint8_t colorR = 255;
        std::uint8_t colorG = 255;
        std::uint8_t colorB = 255;
        std::uint8_t alpha = 255;
        bool filled = true;
        int layer = 5;
    };

    class AnimationComponent final : public IComponent {
      public:
        int currentFrame = 0;
        int targetFrame = 0;
        float frameTransitionSpeed = 2.0F;
        float frameTimer = 0.0F;

        enum class AnimationType {
            SPRITE_SHEET,
            ROTATION,
            SCALE_PULSE,
            TRAIL_EFFECT
        } type = AnimationType::SPRITE_SHEET;

        float rotationSpeed = 0.0F;
        float currentRotation = 0.0F;

        bool isPlaying = false;
        bool loop = false;
        float scaleSpeed = 1.0F;
        float minScale = 0.8F;
        float maxScale = 1.2F;
        float currentScale = 1.0F;
        bool scalingUp = true;
    };

    class CameraComponent final : public IComponent {
      public:
        float x = 0.0F;
        float y = 0.0F;
        float viewportWidth = 1280.0F;
        float viewportHeight = 720.0F;
        float zoom = 1.0F;
        int followTargetId = -1;
    };

    class ButtonComponent final : public IComponent {
      public:
        float boundsX = 0.0F;
        float boundsY = 0.0F;
        float boundsW = 0.0F;
        float boundsH = 0.0F;

        enum class State { NORMAL, HOVER, PRESSED };
        State currentState = State::NORMAL;
        int actionId = -1;
    };

} // namespace GameEngine
