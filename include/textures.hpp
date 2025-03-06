#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <CONSTANTS.hpp>
class Texture {
  public:
    static SDL_Surface* loadFromFile(const char* path, SDL_Renderer* renderer, SDL_Texture*& texture) {
        if (texture != nullptr) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }

        SDL_Surface* loadedSurface = IMG_Load(path);
        if (loadedSurface == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image %s! SDL_image Error: %s\n", path, SDL_GetError());
            return nullptr;
        }

        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if (texture == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
            return nullptr;
        }

        return loadedSurface;
    }
};