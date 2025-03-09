#pragma once
#include "SDL2/SDL_render.h"
#include "sprite.hpp"
#include <SDL2/SDL.h>


class Enemy : public Sprite {
public:
  Enemy(SDL_Renderer* renderer);
  ~Enemy();
};

Enemy::Enemy(SDL_Renderer* renderer) {
  loadFromFile("assets/enemy/enemy.png", renderer);
  setPosition(0, 0);
}