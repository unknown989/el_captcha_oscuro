#pragma once
#include "textures.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
class Sprite {
  public:
    Sprite();
    ~Sprite();

    int getX() const { return posX; }
    int getY() const { return posY; }
    SDL_Rect getRect() const { return destRect; }
    bool loadFromFile(const char* path, SDL_Renderer* renderer);
    void setTexture(SDL_Texture* texture) { this->texture = texture; }
    void render(SDL_Renderer* renderer, int x, int y);
    void setPosition(int x, int y);
    void setSize(int w, int h);
    void setIsHidden(bool isHidden) { this->isHidden = isHidden; }
    bool getIsHidden() const { return isHidden; }

  private:
    SDL_Texture* texture;
    SDL_Rect srcRect;
    SDL_Rect destRect;
    int posX, posY;
    bool isHidden = false;
};

Sprite::Sprite() {
    posX = 0;
    posY = 0;
    texture = nullptr;
    srcRect = {0, 0, 0, 0};
    destRect = {0, 0, 0, 0};
}

Sprite::~Sprite() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

bool Sprite::loadFromFile(const char* path, SDL_Renderer* renderer) {
    // Load image at specified path
    SDL_Surface* loadedSurface = Texture::loadFromFile(path, renderer, texture);
    if (loadedSurface == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image %s! SDL_image Error: %s\n", path, SDL_GetError());
        exit(1);
    }
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = loadedSurface->w;
    srcRect.h = loadedSurface->h;
    destRect.x = 0;
    destRect.y = 0;
    destRect.w = loadedSurface->w;
    destRect.h = loadedSurface->h;

    SDL_FreeSurface(loadedSurface);
    return true;
}

void Sprite::render(SDL_Renderer* renderer, int x, int y) {
    // Render to screen, if not hidden
    if(isHidden) return;
    destRect.x = x;
    destRect.y = y;
    SDL_RenderCopy(renderer, texture, &srcRect, &destRect);
}

void Sprite::setPosition(int x, int y) {
    posX = x;
    posY = y;
    destRect.x = x;
    destRect.y = y;
}

void Sprite::setSize(int w, int h) {
    destRect.w = w;
    destRect.h = h;
}
// Code created by Mouttaki Omar(王明清)