#pragma once
#include "Level.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <soundmanager.hpp>
#include <string>

class LevelOne : public Level {
public:
  LevelOne(SDL_Renderer *renderer);
  ~LevelOne();
  void render(SDL_Renderer *renderer);
  void update();
  void handleEvents(SDL_Event *event, SDL_Renderer *renderer);

private:
  TTF_Font *statsFont = nullptr;
  TTF_Font *tutorialFont = nullptr;
  void renderPlayerStats(SDL_Renderer *renderer);
  void renderTutorial(SDL_Renderer *renderer);

  // Tutorial state tracking
  enum TutorialState {
    MOVE_LEFT_RIGHT,
    WALK_SLOW,
    DASH,
    AIM,
    JUMP,
    SHOOT,
    COMPLETE
  };

  TutorialState tutorialState = MOVE_LEFT_RIGHT;
  bool movedLeft = false;
  bool movedRight = false;
  bool walkedSlow = false;
  bool dashed = false;
  bool aimed = false;
  bool jumped = false;
  bool shot = false;
  bool readyToAdvance = false;

  // Mouse position tracking for aim detection
  int lastMouseX = 0;
  int lastMouseY = 0;
  int mouseMovementThreshold = 50; // Minimum mouse movement to count as aiming

  Uint32 messageTimer = 0;
  bool showAdvanceMessage = false;
};

LevelOne::LevelOne(SDL_Renderer *renderer) : Level(renderer) {
  SDL_Log("Loading level one...");
  readLevel("levels/lvl1.txt", renderer);
  loadLevelBackground("assets/backgrounds/level1.png", renderer);
  SOUND_MANAGER.playMusic("enigma");

  // Load fonts
  statsFont = TTF_OpenFont("assets/fonts/ARCADECLASSIC.ttf", 24);
  tutorialFont = TTF_OpenFont("assets/fonts/ARCADECLASSIC.ttf", 28);
  if (!statsFont || !tutorialFont) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font: %s",
                 TTF_GetError());
  }

  SDL_Log("Level one loaded. Number of blocks: %zu", blocks.size());
  player->shouldShot(true);

  // Initialize timer for message flashing
  messageTimer = SDL_GetTicks();

  // Get initial mouse position
  int x, y;
  SDL_GetMouseState(&x, &y);
  lastMouseX = x;
  lastMouseY = y;
}

void LevelOne::update() {
  Level::update();

  // Update bullet logic if player exists
  if (player) {
    player->updateBullets();
  }

  // Flash the advance message every second
  Uint32 currentTime = SDL_GetTicks();
  if (currentTime - messageTimer > 1000) {
    showAdvanceMessage = !showAdvanceMessage;
    messageTimer = currentTime;
  }
}

void LevelOne::render(SDL_Renderer *renderer) {
  Level::render(renderer);

  // Render player stats
  renderPlayerStats(renderer);

  // Render tutorial instructions
  renderTutorial(renderer);
}

void LevelOne::renderPlayerStats(SDL_Renderer *renderer) {
  if (!player || !statsFont)
    return;

  // Create health text in format "currentHealth/maxHealth"
  std::string healthText = "HEALTH: " + std::to_string(player->getHealth()) +
                           " / " + std::to_string(player->getMaxHealth());

  // Create bullets text
  std::string bulletsText = "BULLETS: " + std::to_string(player->getBullets()) + " / 10";;

  // Set text color
  SDL_Color textColor = {255, 255, 255, 255}; // White

  // Render health text
  SDL_Surface *healthSurface =
      TTF_RenderText_Solid(statsFont, healthText.c_str(), textColor);
  if (healthSurface) {
    SDL_Texture *healthTexture =
        SDL_CreateTextureFromSurface(renderer, healthSurface);
    if (healthTexture) {
      SDL_Rect healthRect = {20, 20, healthSurface->w, healthSurface->h};
      SDL_RenderCopy(renderer, healthTexture, NULL, &healthRect);
      SDL_DestroyTexture(healthTexture);
    }
    SDL_FreeSurface(healthSurface);
  }

  // Render bullets text
  SDL_Surface *bulletsSurface =
      TTF_RenderText_Solid(statsFont, bulletsText.c_str(), textColor);
  if (bulletsSurface) {
    SDL_Texture *bulletsTexture =
        SDL_CreateTextureFromSurface(renderer, bulletsSurface);
    if (bulletsTexture) {
      SDL_Rect bulletsRect = {20, 50, bulletsSurface->w, bulletsSurface->h};
      SDL_RenderCopy(renderer, bulletsTexture, NULL, &bulletsRect);
      SDL_DestroyTexture(bulletsTexture);
    }
    SDL_FreeSurface(bulletsSurface);
  }
}

void LevelOne::renderTutorial(SDL_Renderer *renderer) {
  if (!tutorialFont)
    return;

  SDL_Color textColor = {255, 255, 0, 255};  // Yellow for instructions
  SDL_Color advanceColor = {255, 0, 0, 255}; // Red for advance message

  std::string instructionText;
  int yPos = 100;
  int yStep = 40; // Spacing between instructions

  // Movement instructions
  instructionText = "Use A and D keys to move left and right";
  SDL_Surface *surface =
      TTF_RenderText_Solid(tutorialFont, instructionText.c_str(), textColor);
  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
      SDL_Rect rect = {100, yPos, surface->w, surface->h};
      SDL_RenderCopy(renderer, texture, NULL, &rect);
      SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
  }

  // Walk slow instructions
  yPos += yStep;
  instructionText = "Hold CTRL to walk slowly";
  surface =
      TTF_RenderText_Solid(tutorialFont, instructionText.c_str(), textColor);
  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
      SDL_Rect rect = {100, yPos, surface->w, surface->h};
      SDL_RenderCopy(renderer, texture, NULL, &rect);
      SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
  }

  // Dash instructions
  yPos += yStep;
  instructionText = "Press SHIFT to dash";
  surface =
      TTF_RenderText_Solid(tutorialFont, instructionText.c_str(), textColor);
  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
      SDL_Rect rect = {100, yPos, surface->w, surface->h};
      SDL_RenderCopy(renderer, texture, NULL, &rect);
      SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
  }

  // Aim instructions
  yPos += yStep;
  instructionText = "Move the mouse to aim";
  surface =
      TTF_RenderText_Solid(tutorialFont, instructionText.c_str(), textColor);
  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
      SDL_Rect rect = {100, yPos, surface->w, surface->h};
      SDL_RenderCopy(renderer, texture, NULL, &rect);
      SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
  }

  // Jump instructions
  yPos += yStep;
  instructionText = "Press SPACE to jump";
  surface =
      TTF_RenderText_Solid(tutorialFont, instructionText.c_str(), textColor);
  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
      SDL_Rect rect = {100, yPos, surface->w, surface->h};
      SDL_RenderCopy(renderer, texture, NULL, &rect);
      SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
  }

  // Shoot instructions
  yPos += yStep;
  instructionText = "Press LEFT MOUSE BUTTON to shoot";
  surface =
      TTF_RenderText_Solid(tutorialFont, instructionText.c_str(), textColor);
  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
      SDL_Rect rect = {100, yPos, surface->w, surface->h};
      SDL_RenderCopy(renderer, texture, NULL, &rect);
      SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
  }
  yPos += yStep;
  instructionText = "Press R to reload";
  surface =
      TTF_RenderText_Solid(tutorialFont, instructionText.c_str(), textColor);
  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
      SDL_Rect rect = {100, yPos, surface->w, surface->h};
      SDL_RenderCopy(renderer, texture, NULL, &rect);
      SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
  }
  yPos += yStep;
  instructionText = "Press Ctrl+R to restart the parkour level";
  surface =
      TTF_RenderText_Solid(tutorialFont, instructionText.c_str(), textColor);
  if (surface) {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
      SDL_Rect rect = {100, yPos, surface->w, surface->h};
      SDL_RenderCopy(renderer, texture, NULL, &rect);
      SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
  }

  // Advance to next level message
  if (showAdvanceMessage) {
    yPos += yStep + 20; // Extra space before the final instruction
    instructionText = "PRESS G TO CONTINUE";

    surface = TTF_RenderText_Solid(tutorialFont, instructionText.c_str(),
                                   advanceColor);
    if (surface) {
      SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
      if (texture) {
        SDL_Rect rect = {100, yPos, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_DestroyTexture(texture);
      }
      SDL_FreeSurface(surface);
    }
  }
}

void LevelOne::handleEvents(SDL_Event *event, SDL_Renderer *renderer) {
  Level::handleEvents(event, renderer);

  // Check for G key press to advance level
  if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_g) {
    GameState::setCurrentLevel(2);
  }
}

LevelOne::~LevelOne() {
  // Clean up fonts
  if (statsFont) {
    TTF_CloseFont(statsFont);
    statsFont = nullptr;
  }

  if (tutorialFont) {
    TTF_CloseFont(tutorialFont);
    tutorialFont = nullptr;
  }
}