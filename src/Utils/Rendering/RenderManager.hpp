/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** RenderManager
*/

#pragma once

#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace GameEngine
{
    class RenderManager {
      public:
        RenderManager();
        ~RenderManager();

        RenderManager(const RenderManager &) = delete;
        RenderManager &operator=(const RenderManager &) = delete;
        RenderManager(RenderManager &&) = delete;
        RenderManager &operator=(RenderManager &&) = delete;

        SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &id, const std::string &filename);
        SDL_Texture *getTexture(const std::string &id) const;
        bool hasTexture(const std::string &id) const;
        void removeTexture(const std::string &id);

        TTF_Font *loadFont(const std::string &id, const std::string &filename, int fontSize);
        TTF_Font *getFont(const std::string &id) const;
        bool hasFont(const std::string &id) const;
        void removeFont(const std::string &id);

        SDL_Texture *getOrCreateTextTexture(SDL_Renderer *renderer, const std::string &id,
            const std::string &fontId, const std::string &text, SDL_Color color);
        SDL_Texture *getTextTexture(const std::string &id) const;
        void removeTextTexture(const std::string &id);
        bool hasTextTexture(const std::string &id) const;

        void clearTextures();
        void clearFonts();
        void clearTextTextures();
        void clearAll();

        size_t getTextureCount() const { return _textures.size(); }
        size_t getFontCount() const { return _fonts.size(); }
        size_t getTextTextureCount() const { return _textTextures.size(); }

      private:
        std::unordered_map<std::string, SDL_Texture *> _textures;
        std::unordered_map<std::string, TTF_Font *> _fonts;
        std::unordered_map<std::string, SDL_Texture *> _textTextures;
    };
} // namespace GameEngine
