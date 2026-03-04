/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** RenderEventTypes
*/

#pragma once

#include <cstdint>
#include <string>
#include <array>

namespace GameEngine
{
    struct CreateWindowEvent {
        std::string title;
        int width;
        int height;
        std::uint32_t flags;

        CreateWindowEvent(std::string windowTitle, int w, int h, std::uint32_t windowFlags = 0)
            : title(std::move(windowTitle)), width(w), height(h), flags(windowFlags)
        {}
    };

    struct DestroyWindowEvent {
        DestroyWindowEvent() = default;
    };

    struct SetWindowTitleEvent {
        std::string title;

        explicit SetWindowTitleEvent(std::string windowTitle) : title(std::move(windowTitle)) {}
    };

    struct ResizeWindowEvent {
        int width;
        int height;

        ResizeWindowEvent(int w, int h) : width(w), height(h) {}
    };

    struct CreateRendererEvent {
        std::string driver;

        CreateRendererEvent() : driver() {}
        explicit CreateRendererEvent(std::string rendererDriver)
            : driver(std::move(rendererDriver))
        {}
    };

    struct SetRenderDrawColorEvent {
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
        std::uint8_t a;

        SetRenderDrawColorEvent(std::uint8_t red, std::uint8_t green, std::uint8_t blue,
            std::uint8_t alpha = 255)
            : r(red), g(green), b(blue), a(alpha)
        {}
    };

    struct ClearRendererEvent {
        ClearRendererEvent() = default;
    };

    struct PresentRendererEvent {
        PresentRendererEvent() = default;
    };

    struct LoadTextureEvent {
        std::string textureId;
        std::string filename;

        LoadTextureEvent(std::string id, std::string file)
            : textureId(std::move(id)), filename(std::move(file))
        {}
    };

    struct UnloadTextureEvent {
        std::string textureId;

        explicit UnloadTextureEvent(std::string id) : textureId(std::move(id)) {}
    };

    struct SetTextureColorModEvent {
        std::string textureId;
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;

        SetTextureColorModEvent(std::string id, std::uint8_t red, std::uint8_t green,
            std::uint8_t blue)
            : textureId(std::move(id)), r(red), g(green), b(blue)
        {}
    };

    struct SetTextureAlphaModEvent {
        std::string textureId;
        std::uint8_t alpha;

        SetTextureAlphaModEvent(std::string id, std::uint8_t a)
            : textureId(std::move(id)), alpha(a)
        {}
    };

    struct LoadFontEvent {
        std::string fontId;
        std::string filename;
        int fontSize;

        LoadFontEvent(std::string id, std::string file, int size)
            : fontId(std::move(id)), filename(std::move(file)), fontSize(size)
        {}
    };

    struct UnloadFontEvent {
        std::string fontId;

        explicit UnloadFontEvent(std::string id) : fontId(std::move(id)) {}
    };

    struct CreateTextTextureEvent {
        std::string textTextureId;
        std::string fontId;
        std::string text;
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
        std::uint8_t a;

        CreateTextTextureEvent(std::string textId, std::string font, std::string textContent,
            std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255)
            : textTextureId(std::move(textId)), fontId(std::move(font)),
              text(std::move(textContent)), r(red), g(green), b(blue), a(alpha)
        {}
    };

    struct RenderTextureEvent {
        std::string textureId;
        float srcX;
        float srcY;
        float srcW;
        float srcH;
        float dstX;
        float dstY;
        float dstW;
        float dstH;
        float rotation;
        int layer;

        RenderTextureEvent(std::string id, float srcRectX, float srcRectY, float srcRectW,
            float srcRectH, float dstRectX, float dstRectY, float dstRectW, float dstRectH,
            float rot = 0.0F, int renderLayer = 0)
            : textureId(std::move(id)), srcX(srcRectX), srcY(srcRectY), srcW(srcRectW),
              srcH(srcRectH), dstX(dstRectX), dstY(dstRectY), dstW(dstRectW), dstH(dstRectH),
              rotation(rot), layer(renderLayer)
        {}
    };

    struct RenderRectEvent {
        float x;
        float y;
        float width;
        float height;
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
        std::uint8_t a;
        bool filled;
        int layer;

        RenderRectEvent(float rectX, float rectY, float w, float h, std::uint8_t red,
            std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255, bool isFilled = true,
            int renderLayer = 0)
            : x(rectX), y(rectY), width(w), height(h), r(red), g(green), b(blue), a(alpha),
              filled(isFilled), layer(renderLayer)
        {}
    };

    struct RenderCircleEvent {
        float x;
        float y;
        float radius;
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
        std::uint8_t a;
        bool filled;
        int layer;

        RenderCircleEvent(float centerX, float centerY, float rad, std::uint8_t red,
            std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255, bool isFilled = true,
            int renderLayer = 0)
            : x(centerX), y(centerY), radius(rad), r(red), g(green), b(blue), a(alpha),
              filled(isFilled), layer(renderLayer)
        {}
    };

    struct RenderTextEvent {
        std::string textTextureId;
        float x;
        float y;
        float scaleX;
        float scaleY;
        float rotation;
        int layer;

        RenderTextEvent(std::string textId, float posX, float posY, float sX = 1.0F,
            float sY = 1.0F, float rot = 0.0F, int renderLayer = 10)
            : textTextureId(std::move(textId)), x(posX), y(posY), scaleX(sX), scaleY(sY),
              rotation(rot), layer(renderLayer)
        {}
    };

} // namespace GameEngine
