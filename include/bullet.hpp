#pragma once
#include "SDL2/SDL.h"
#include "textures.hpp"
#include <list>
#include <memory>

class Bullet {
public:
  Bullet(SDL_Renderer *renderer, float x, float y, float angle, float speed);
  ~Bullet();
  void update();
  void render(SDL_Renderer *renderer);
  bool isOutOfBounds(int screenWidth, int screenHeight);
  bool isColliding(int targetX, int targetY, int targetWidth, int targetHeight);
  float getX() const { return x; }
  float getY() const { return y; }

private:
  SDL_Texture *texture = nullptr;
  float x, y;
  float angle;
  float speed;
  int width = 8;
  int height = 4;
};

Bullet::Bullet(SDL_Renderer *renderer, float x, float y, float angle,
               float speed)
    : x(x), y(y), angle(angle), speed(speed) {
  SDL_Surface *surface =
      Texture::loadFromFile("assets/gun/laser_bullet.png", renderer, texture);
  if (surface) {
    width = surface->w;
    height = surface->h;
    SDL_FreeSurface(surface);
  }
}

Bullet::~Bullet() {
  if (texture) {
    SDL_DestroyTexture(texture);
  }
}

void Bullet::update() {
  // Move bullet based on angle and speed
  x += cos(angle * M_PI / 180) * speed;
  y += sin(angle * M_PI / 180) * speed;
}

void Bullet::render(SDL_Renderer *renderer) {
  if (texture) {
    SDL_Rect destRect = {static_cast<int>(x - width / 2),
                         static_cast<int>(y - height / 2), width, height};
    SDL_RenderCopyEx(renderer, texture, nullptr, &destRect, angle, nullptr,
                     SDL_FLIP_NONE);
  }
}

bool Bullet::isOutOfBounds(int screenWidth, int screenHeight) {
  return (x < -width || x > screenWidth + width || y < -height ||
          y > screenHeight + height);
}

bool Bullet::isColliding(int targetX, int targetY, int targetWidth,
                         int targetHeight) {
  // Get bullet's bounding box
  int bulletLeft = static_cast<int>(x - width / 2);
  int bulletRight = static_cast<int>(x + width / 2);
  int bulletTop = static_cast<int>(y - height / 2);
  int bulletBottom = static_cast<int>(y + height / 2);

  // Check for collision with target's bounding box
  return (bulletRight >= targetX && bulletLeft <= targetX + targetWidth &&
          bulletBottom >= targetY && bulletTop <= targetY + targetHeight);
}