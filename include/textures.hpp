#pragma once
#include <CONSTANTS.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
class Texture {
  public:
    // Load a texture from a file
    // @param path The path to the image file
    // @param renderer The renderer to load the texture onto
    // @param texture The texture to load the image onto
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

// Code created by Mouttaki Omar(王明清)