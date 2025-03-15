#pragma once
#include "SDL2/SDL_mixer.h"
#include "enemy.hpp"
#include <CONSTANTS.hpp>
#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <cstdio>
#include <player.hpp>
#include <sprite.hpp>
#include <vector>

// A block is a simple sprite with a type
class Block : public Sprite {
public:
  Block(SDL_Renderer *renderer, const char *texturePath) {
    loadFromFile(texturePath, renderer);
    setSize(W_SPRITESIZE, W_SPRITESIZE);
  }
  char type;
};
// The base class for all levels
// It contains the Box2D world, the player, the blocks, and the background

class Level {
public:
  Level(SDL_Renderer *renderer);
  virtual ~Level();
  virtual void render(SDL_Renderer *renderer);
  virtual void handleEvents(SDL_Event *event, SDL_Renderer *renderer);
  void readLevel(const char *path, SDL_Renderer *renderer);
  void loadLevelBackground(const char *path, SDL_Renderer *renderer);
  bool isLevelLoaded() { return isLoaded; }
  void loadLevel() {
    isLoaded = true;
  }
  void unloadLevel() { isLoaded = false; }
  void update();
  bool isLevelOver() { return this->over; };

protected:
  std::vector<Block *> blocks;
  SDL_Texture *background = nullptr;
  Player *player;
  b2Vec2 gravity;
  b2World *world;
  Mix_Music *level_music;
  bool isLoaded = false;
  bool isPlayingMusic = false;
  Enemy *enemy;
  bool over = false;
};

Level::Level(SDL_Renderer *renderer) : gravity(0.0f, 9.8f) {
  // box2d setup
  world = new b2World(gravity);
  world->SetAllowSleeping(false);

  // Add linear damping to prevent infinite sliding
  b2BodyDef bodyDef;
  bodyDef.linearDamping = 0.5f;
  bodyDef.fixedRotation = true; // Prevent rotation

  background = nullptr;
  player = nullptr;
}

// Add cleanup in destructor
Level::~Level() {
  for (Block *block : blocks) {
    delete block;
  }
  blocks.clear();

  delete world; // Clean up Box2D world
  if (player) {
    delete player;
  }
  if (background) {
    SDL_DestroyTexture(background);
  }
}

void Level::loadLevelBackground(const char *path, SDL_Renderer *renderer) {
  // Load the background image
  SDL_Surface *loadedSurface =
      Texture::loadFromFile(path, renderer, background);
  if (loadedSurface == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Unable to load image %s! SDL_image Error: %s\n", path,
                 SDL_GetError());
    exit(1);
  }
  SDL_FreeSurface(loadedSurface);
}
// Read the level from a file
// The file format is simple:
// check levels/info.txt for reference
void Level::readLevel(const char *path, SDL_Renderer *renderer) {
  // Open the file
  FILE *file = fopen(path, "r");
  if (file == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not open file %s", path);
    return;
  }
  int row = 0;
  int col = 0;
  char blockType;
  // the level file format is 30 blocks by 17 blocks
  while ((blockType = fgetc(file)) != EOF && row < 30) {
    if (blockType == '\n') {
      row++;
      col = 0;
      continue;
    }
    Block *block = nullptr;
    // Loading blocks according to the type
    switch (blockType) {
      // Dirt block + physics
    case 'D': {
      block = new Block(renderer, "assets/blocks/dirt.png");
      b2BodyDef blockBodyDef;
      blockBodyDef.type = b2_staticBody;
      blockBodyDef.position.Set(col * W_SPRITESIZE, row * W_SPRITESIZE);
      b2Body *blockBody = world->CreateBody(&blockBodyDef);
      b2PolygonShape blockShape;
      blockShape.SetAsBox(W_SPRITESIZE / 2, W_SPRITESIZE / 2);
      b2FixtureDef blockFixtureDef;
      blockFixtureDef.shape = &blockShape;
      blockFixtureDef.density = 1.0f;
      blockFixtureDef.friction = 0.3f;
      blockBody->CreateFixture(&blockFixtureDef);
      break;
    }
    case 'P': {
      // player sprite
      player =
          new Player(renderer, world, col * W_SPRITESIZE, row * W_SPRITESIZE);
      break;
    }
    case 'E': {
      // enemy sprite
      enemy = new Enemy(renderer);
      enemy->setPosition(col * W_SPRITESIZE, row * W_SPRITESIZE);
      enemy->setSize(W_SPRITESIZE, W_SPRITESIZE);
      break;
    }
    default:
      col++;
      continue;
    }

    if (block) {
      block->type = blockType;
      block->setPosition(col * W_SPRITESIZE, row * W_SPRITESIZE);
      blocks.push_back(block);
    }
    col++;
  }

  fclose(file);
  SDL_Log("Loaded %zu blocks", blocks.size());
}
void Level::update() {
  // First update player physics (apply forces)
  if (player) {
    player->updatePhysics();
  }

  // Then step the physics world
  const float timeStep = 1.0f / 60.0f;
  const int velocityIterations = 8;
  const int positionIterations = 3;
  world->Step(timeStep, velocityIterations, positionIterations);

  // Finally update player position based on physics
  if (player) {
    player->update();
  }
}

void Level::render(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
  SDL_RenderClear(renderer);
  // Render the background
  if (background != nullptr) {
    SDL_RenderCopy(renderer, background, nullptr, nullptr);
  }
  // Render the blocks
  for (Block *block : blocks) {
    block->render(renderer, block->getX(), block->getY());
  }
  // Render the player
  if (player) {
    player->render(renderer);
  }
}
void Level::handleEvents(SDL_Event *event, SDL_Renderer *renderer) {
  if (player) {
    player->handleEvents(event, renderer);
  }
}
// Code created by Mouttaki Omar(王明清)