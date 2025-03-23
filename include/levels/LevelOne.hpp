#pragma once
#include "Level.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

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
  level_music = Mix_LoadMUS("assets/music/La Fiola 2.wav");
  if (level_music == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Couldn't load music: %s, music wont be played",
                 Mix_GetError());
  }

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

  // Update tutorial state based on player actions
  if (player) {
    // Check player movement for tutorial progression
    if (tutorialState == MOVE_LEFT_RIGHT) {
      if (player->getVelocityX() < -0.1f)
        movedLeft = true;
      if (player->getVelocityX() > 0.1f)
        movedRight = true;

      if (movedLeft && movedRight) {
        tutorialState = WALK_SLOW;
      }
    } else if (tutorialState == WALK_SLOW) {
      // Continuous CTRL key check
      const Uint8 *keyState = SDL_GetKeyboardState(NULL);
      bool ctrlPressed = keyState[SDL_SCANCODE_LCTRL] || keyState[SDL_SCANCODE_RCTRL];
      
      // Check for sustained slow movement while CTRL is held
      if (ctrlPressed && abs(player->getVelocityX()) > 1.0f && abs(player->getVelocityX()) < 3.0f) {
        walkedSlow = true;
      }
    }
    // Check for dash (high velocity spike)
    if (abs(player->getVelocityX()) > 20.0f) {
      dashed = true;
    }
    
    if (dashed) {
      tutorialState = AIM;
    }
  }

  // Flash the advance message every second when ready
  if (readyToAdvance) {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - messageTimer > 1000) {
      showAdvanceMessage = !showAdvanceMessage;
      messageTimer = currentTime;
    }
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
  std::string healthText = std::to_string(player->getHealth()) + " I " +
                           std::to_string(player->getMaxHealth());

  // Create bullets text
  std::string bulletsText = std::to_string(player->getBullets());

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

  SDL_Color textColor = {255, 255, 0, 255};       // Yellow for instructions
  SDL_Color completedColor = {0, 255, 0, 255};    // Green for completed tasks
  SDL_Color advanceColor = {255, 0, 0, 255};      // Red for advance message
  SDL_Color disabledColor = {128, 128, 128, 255}; // Gray for disabled tasks

  std::string instructionText;
  int yPos = 100;
  int yStep = 40; // Reduced vertical spacing between instructions

  // Movement instructions
  if (tutorialState == MOVE_LEFT_RIGHT) {
    instructionText = "Use A and D keys to move left and right";
    textColor = {255, 255, 0, 255}; // Yellow
  } else {
    instructionText = "Move left and right  COMPLETED";
    textColor = completedColor;
  }

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
  if (tutorialState < WALK_SLOW) {
    textColor = disabledColor;
    instructionText = "Hold CTRL to walk slowly";
  } else if (tutorialState == WALK_SLOW) {
    textColor = {255, 255, 0, 255}; // Yellow
    instructionText = "Hold CTRL to walk slowly";
  } else {
    textColor = completedColor;
    instructionText = "Walk slowly  COMPLETED";
  }

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
  if (tutorialState < DASH) {
    textColor = disabledColor;
    instructionText = "Press SHIFT to dash";
  } else if (tutorialState == DASH) {
    textColor = {255, 255, 0, 255}; // Yellow
    instructionText = "Press SHIFT to dash";
  } else {
    textColor = completedColor;
    instructionText = "Dash  COMPLETED";
  }

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
  if (tutorialState < AIM) {
    textColor = disabledColor;
    instructionText = "Move the mouse to aim";
  } else if (tutorialState == AIM) {
    textColor = {255, 255, 0, 255}; // Yellow
    instructionText = "Move the mouse to aim";
  } else {
    textColor = completedColor;
    instructionText = "Aim  COMPLETED";
  }

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
  if (tutorialState < JUMP) {
    textColor = disabledColor;
    instructionText = "Press SPACE to jump";
  } else if (tutorialState == JUMP) {
    textColor = {255, 255, 0, 255}; // Yellow
    instructionText = "Press SPACE to jump";
  } else {
    textColor = completedColor;
    instructionText = "Jump  COMPLETED";
  }

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
  if (tutorialState < SHOOT) {
    textColor = disabledColor;
    instructionText = "Press LEFT MOUSE BUTTON to shoot";
  } else if (tutorialState == SHOOT) {
    textColor = {255, 255, 0, 255}; // Yellow
    instructionText = "Press LEFT MOUSE BUTTON to shoot";
  } else {
    textColor = completedColor;
    instructionText = "Shoot  COMPLETED";
  }

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
  if (readyToAdvance && showAdvanceMessage) {
    yPos += yStep + 20; // Extra space before the final instruction
    instructionText = "PRESS G TO ADVANCE TO NEXT LEVEL";

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
  if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_g &&
      readyToAdvance) {
    GameState::setCurrentLevel(2);
  }

  // Check for CTRL key for slow walking
  if (tutorialState == WALK_SLOW) {
    const Uint8 *keyState = SDL_GetKeyboardState(NULL);
    if (keyState[SDL_SCANCODE_LCTRL] || keyState[SDL_SCANCODE_RCTRL]) {
      // If CTRL is pressed and player is moving
      if (abs(player->getVelocityX()) > 0.1f && abs(player->getVelocityX()) < 5.0f) {
        walkedSlow = true;
      }
    }
  }

  // For mouse movement detection in AIM state
  if (tutorialState == AIM && event->type == SDL_MOUSEMOTION) {
    int x = event->motion.x;
    int y = event->motion.y;
    
    // Check if mouse has moved significantly from last position
    if (abs(x - lastMouseX) > mouseMovementThreshold || 
        abs(y - lastMouseY) > mouseMovementThreshold) {
      aimed = true;
    }
    
    // Update last mouse position
    lastMouseX = x;
    lastMouseY = y;
  }

  // Start music when level is loaded
  if (isLoaded && !isPlayingMusic) {
    Mix_VolumeMusic(100);
    Mix_PlayMusic(level_music, -1);
    isPlayingMusic = true;
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