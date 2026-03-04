/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** RenderManager
*/

#include "RenderManager.hpp"
#include "../Logger/Logger.hpp"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>

namespace GameEngine
{
    RenderManager::RenderManager()
    {
        Logger::info("RenderManager: initialized");
    }

    RenderManager::~RenderManager()
    {
        clearAll();
        Logger::info("RenderManager: destroyed");
    }

    SDL_Texture *RenderManager::loadTexture(
        SDL_Renderer *renderer, const std::string &id, const std::string &filename)
    {
        if (renderer == nullptr) {
            Logger::error("RenderManager: renderer is null");
            return nullptr;
        }

        const auto it = _textures.find(id);
        if (it != _textures.end()) {
            Logger::debug("RenderManager: Texture '" + id + "' already loaded");
            return it->second;
        }

        SDL_Texture *texture = IMG_LoadTexture(renderer, filename.c_str());
        if (texture == nullptr) {
            Logger::error("RenderManager: Failed to load texture '" + id + "' from '" + filename
                + "': " + SDL_GetError());
            return nullptr;
        }

        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

        _textures[id] = texture;

        Logger::info("RenderManager: Loaded texture '" + id + "' from '" + filename
            + "' (count: " + std::to_string(_textures.size()) + ")");

        return texture;
    }

    SDL_Texture *RenderManager::getTexture(const std::string &id) const
    {
        const auto it = _textures.find(id);
        if (it != _textures.end()) {
            return it->second;
        }
        return nullptr;
    }

    bool RenderManager::hasTexture(const std::string &id) const
    {
        return _textures.contains(id);
    }

    void RenderManager::removeTexture(const std::string &id)
    {
        const auto it = _textures.find(id);
        if (it != _textures.end()) {
            SDL_DestroyTexture(it->second);
            _textures.erase(it);
            Logger::debug("RenderManager: Removed texture '" + id
                + "' (count: " + std::to_string(_textures.size()) + ")");
        }
    }

    TTF_Font *RenderManager::loadFont(
        const std::string &id, const std::string &filename, const int fontSize)
    {
        const auto it = _fonts.find(id);
        if (it != _fonts.end()) {
            Logger::debug("RenderManager: Font '" + id + "' already loaded");
            return it->second;
        }

        TTF_Font *font = TTF_OpenFont(filename.c_str(), fontSize);
        if (font == nullptr) {
            Logger::error("RenderManager: Failed to load font '" + id + "' from '" + filename
                + "': " + SDL_GetError());
            return nullptr;
        }

        _fonts[id] = font;

        Logger::info("RenderManager: Loaded font '" + id + "' from '" + filename + "' (size: "
            + std::to_string(fontSize) + ", count: " + std::to_string(_fonts.size()) + ")");

        return font;
    }

    TTF_Font *RenderManager::getFont(const std::string &id) const
    {
        const auto it = _fonts.find(id);
        if (it != _fonts.end()) {
            return it->second;
        }
        return nullptr;
    }

    bool RenderManager::hasFont(const std::string &id) const
    {
        return _fonts.contains(id);
    }

    void RenderManager::removeFont(const std::string &id)
    {
        const auto it = _fonts.find(id);
        if (it != _fonts.end()) {
            TTF_CloseFont(it->second);
            _fonts.erase(it);
            Logger::debug("RenderManager: Removed font '" + id
                + "' (count: " + std::to_string(_fonts.size()) + ")");
        }
    }

    SDL_Texture *RenderManager::getOrCreateTextTexture(SDL_Renderer *renderer,
        const std::string &id, const std::string &fontId, const std::string &text,
        const SDL_Color color)
    {
        if (renderer == nullptr) {
            Logger::error("RenderManager: renderer is null");
            return nullptr;
        }

        const auto it = _textTextures.find(id);
        if (it != _textTextures.end()) {
            return it->second;
        }

        TTF_Font *font = getFont(fontId);
        if (font == nullptr) {
            Logger::error(
                "RenderManager: Font '" + fontId + "' not found for text texture '" + id + "'");
            return nullptr;
        }

        if (text.empty()) {
            return nullptr;
        }
        SDL_Surface *surface = TTF_RenderText_Solid(font, text.c_str(), text.length(), color);
        if (surface == nullptr) {
            Logger::error(
                "RenderManager: Failed to render text '" + text + "': " + SDL_GetError());
            return nullptr;
        }

        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);

        if (texture == nullptr) {
            Logger::error("RenderManager: Failed to create texture from text surface: "
                + std::string(SDL_GetError()));
            return nullptr;
        }

        _textTextures[id] = texture;
        Logger::debug("RenderManager: Created text texture '" + id
            + "' (count: " + std::to_string(_textTextures.size()) + ")");

        return texture;
    }

    void RenderManager::removeTextTexture(const std::string &id)
    {
        const auto it = _textTextures.find(id);
        if (it != _textTextures.end()) {
            SDL_DestroyTexture(it->second);
            _textTextures.erase(it);
            Logger::debug("RenderManager: Removed text texture '" + id
                + "' (count: " + std::to_string(_textTextures.size()) + ")");
        }
    }

    bool RenderManager::hasTextTexture(const std::string &id) const
    {
        return _textTextures.contains(id);
    }

    SDL_Texture *RenderManager::getTextTexture(const std::string &id) const
    {
        auto it = _textTextures.find(id);
        if (it != _textTextures.end()) {
            return it->second;
        }
        return nullptr;
    }

    void RenderManager::clearTextures()
    {
        for (const auto &pair : _textures) {
            if (pair.second != nullptr) {
                SDL_DestroyTexture(pair.second);
            }
        }
        size_t const count = _textures.size();
        _textures.clear();
        Logger::info("RenderManager: Cleared " + std::to_string(count) + " textures");
    }

    void RenderManager::clearFonts()
    {
        for (const auto &pair : _fonts) {
            if (pair.second != nullptr) {
                TTF_CloseFont(pair.second);
            }
        }
        size_t const count = _fonts.size();
        _fonts.clear();
        Logger::info("RenderManager: Cleared " + std::to_string(count) + " fonts");
    }

    void RenderManager::clearTextTextures()
    {
        for (const auto &pair : _textTextures) {
            if (pair.second != nullptr) {
                SDL_DestroyTexture(pair.second);
            }
        }
        size_t const count = _textTextures.size();
        _textTextures.clear();
        Logger::info("RenderManager: Cleared " + std::to_string(count) + " text textures");
    }

    void RenderManager::clearAll()
    {
        clearTextTextures();
        clearTextures();
        clearFonts();
    }
} // namespace GameEngine
