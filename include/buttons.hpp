#pragma once
#include "textures.hpp"
#include <SDL2/SDL.h>
#include <functional>
class Button {
  private:
    SDL_Texture* texture;
    SDL_Rect rect;
    std::function<void()> callback = nullptr;
    bool isHoveredState = false;

  public:
    Button();
    ~Button();
    void loadFromFile(const char* path, SDL_Renderer* renderer);
    void setCallback(std::function<void()> callback) {
        this->callback = callback;
    }
    void render(SDL_Renderer* renderer);
    void setPosition(int x, int y);
    void setSize(int w, int h);
    bool isHovered(int x, int y);
    void handleEvents(SDL_Event event);
    SDL_Rect getRect() {
        return rect;
    }
};

Button::Button() {
    texture = nullptr;
    rect = {0, 0, 0, 0};
}
Button::~Button() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}
void Button::loadFromFile(const char* path, SDL_Renderer* renderer) {
    // loading the button's texture
    SDL_Surface* loadedSurface = Texture::loadFromFile(path, renderer, texture);
    if (loadedSurface == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image %s! SDL_image Error: %s\n", path, SDL_GetError());
        exit(1);
    }
    rect.x = 0;
    rect.y = 0;
    rect.w = loadedSurface->w;
    rect.h = loadedSurface->h;
    SDL_FreeSurface(loadedSurface);
}
void Button::render(SDL_Renderer* renderer) {
    // if the button is hovered, darken the texture a bit
    if (isHoveredState) {
        SDL_SetTextureColorMod(texture, 200, 200, 200);
    } else {
        SDL_SetTextureColorMod(texture, 255, 255, 255);
    }
    SDL_RenderCopy(renderer, texture, NULL, &rect);
}
void Button::setPosition(int x, int y) {
    rect.x = x;
    rect.y = y;
}
void Button::setSize(int w, int h) {
    rect.w = w;
    rect.h = h;
}
bool Button::isHovered(int x, int y) {
    // check if the mouse is inside the button's rectangle
    if (x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h) {
        return true;
    }
    return false;
}
void Button::handleEvents(SDL_Event event) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (isHovered(x, y)) {
            if (callback != nullptr) {
                callback();
            }
        }
    }
    if (isHovered(x, y)) {
        isHoveredState = true;
    } else {
        isHoveredState = false;
    }
}
// Code created by Mouttaki Omar(王明清)