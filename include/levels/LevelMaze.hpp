#pragma once
#include "Level.hpp"
#include <SDL2/SDL.h>
#include <music.hpp>
class LevelMaze : public Level {
public:
  LevelMaze(SDL_Renderer *renderer);
};
LevelMaze::LevelMaze(SDL_Renderer *renderer) : Level(renderer) {
  readLevel("levels/maze.txt", renderer);
  loadLevelBackground("assets/backgrounds/maze.jpg", renderer);
  MUSIC.playMusic("amicitia");
  
  // Reduce gravity for maze level
  world->SetGravity(b2Vec2(0.0f, 0.7f));
}