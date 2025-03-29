#pragma once
#include "GameState.hpp"
#include "Level.hpp"
#include "SDL2/SDL.h"
#include <cstdlib>
#include <ctime>
#include <string>
#include <SDL2/SDL_ttf.h>

enum LampState { ON_GREEN, ON_RED, OFF };

struct Lamp {
  int x;
  int y;
  LampState state;
};

class LevelLamp : public Level {
public:
  LevelLamp(SDL_Renderer *renderer);
  ~LevelLamp();

  void render(SDL_Renderer *renderer) override;
  void update() override;
  void handleEvents(SDL_Event *event, SDL_Renderer *renderer) override;
  bool isLevelComplete();

private:
  // Game phases
  enum GamePhase {
    SHOWING_PATTERN,
    PLAYER_INPUT,
    PHASE_COMPLETE,
    GAME_OVER,
    GAME_WIN
  };
  
  // Lamp grid
  Lamp lamps[8][6];
  Lamp input_lamps[8][6];
  Lamp (*current_lamps)[6];
  
  // Textures
  SDL_Texture *green = nullptr;
  SDL_Texture *red = nullptr;
  SDL_Texture *off = nullptr;
  SDL_Rect rect;
  
  // Game state
  GamePhase currentPhase = SHOWING_PATTERN;
  int currentLevel = 0;
  bool isNewPhaseTimer = false;
  uint32_t patternStartTime = 0;
  uint32_t inputStartTime = 0;
  
  // Level configuration
  uint32_t getPatternShowTime() const { return 2000 + (currentLevel * 1000); }
  uint32_t getInputTimeLimit() const { return 10000 + (currentLevel * 5000); }
  const int patternsPerLevel[3] = {4, 8, 12};
  
  // Helper methods
  void generatePattern();
  void renderPhaseInfo(SDL_Renderer *renderer);
  void renderTimer(SDL_Renderer *renderer);
  bool checkPatternMatch();
  void resetLevel();
  void startNextLevel();
  TTF_Font* gameFont = nullptr;
};

LevelLamp::LevelLamp(SDL_Renderer *renderer) : Level(renderer) {
  // Initialize random seed
  srand(static_cast<unsigned int>(time(nullptr)));
  
  // Load background
  loadLevelBackground("assets/backgrounds/lamplevel.png", renderer);
  
  // Load lamp textures
  SDL_Surface *surf = nullptr;
  
  surf = Texture::loadFromFile("assets/lamps/green.png", renderer, green);
  if(!green){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading green lamp");
    exit(1);
  }
  if(surf) SDL_FreeSurface(surf);
  
  surf = Texture::loadFromFile("assets/lamps/red.png", renderer, red);
  if(!red){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading red lamp");
    exit(1);
  }
  if(surf) SDL_FreeSurface(surf);
  
  surf = Texture::loadFromFile("assets/lamps/off.png", renderer, off);
  if(!off){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading off lamp");
    exit(1);
  }
  rect.w = surf->w;
  rect.h = surf->h;
  SDL_FreeSurface(surf);
  
  // Load font for game info
  gameFont = TTF_OpenFont("assets/fonts/ARCADECLASSIC.ttf", 24);
  if (!gameFont) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font: %s", TTF_GetError());
  }
  
  // Initialize game
  generatePattern();
  current_lamps = lamps;
  patternStartTime = SDL_GetTicks();
  
  SDL_Log("Lamp level loaded successfully");
}

LevelLamp::~LevelLamp() {
  // Clean up textures
  if (green) SDL_DestroyTexture(green);
  if (red) SDL_DestroyTexture(red);
  if (off) SDL_DestroyTexture(off);
  
  // Clean up font
  if (gameFont) TTF_CloseFont(gameFont);
}

void LevelLamp::render(SDL_Renderer *renderer) {
  // Call base class render first
  Level::render(renderer);
  
  // Calculate grid positioning to center on screen
  int lampSize = rect.w;
  int spacing = 10;
  int gridWidth = 8 * (lampSize + spacing);
  int gridHeight = 6 * (lampSize + spacing);
  int offsetX = (W_WIDTH - gridWidth) / 2;
  int offsetY = (W_HEIGHT - gridHeight) / 2;
  
  // Render lamps
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 6; j++) {
      rect.x = offsetX + i * (lampSize + spacing);
      rect.y = offsetY + j * (lampSize + spacing);
      
      switch (current_lamps[i][j].state) {
      case ON_GREEN:
        SDL_RenderCopy(renderer, green, NULL, &rect);
        break;
      case ON_RED:
        SDL_RenderCopy(renderer, red, NULL, &rect);
        break;
      case OFF:
        SDL_RenderCopy(renderer, off, NULL, &rect);
        break;
      }
    }
  }
  
  // Render phase info
  renderPhaseInfo(renderer);
  
  // Render timer
  renderTimer(renderer);
}

void LevelLamp::update() {
  uint32_t currentTime = SDL_GetTicks();

  switch (currentPhase) {
    case SHOWING_PATTERN:
      if (currentTime - patternStartTime > getPatternShowTime()) {
        // Clear all lamps in input grid
        for (int i = 0; i < 8; i++) {
          for (int j = 0; j < 6; j++) {
            input_lamps[i][j].x = lamps[i][j].x;
            input_lamps[i][j].y = lamps[i][j].y;
            input_lamps[i][j].state = OFF;
          }
        }
        current_lamps = input_lamps;
        currentPhase = PLAYER_INPUT;
        inputStartTime = currentTime;
        SDL_Log("Switching to PLAYER_INPUT phase at time: %u", currentTime);
      }
      break;
      
    case PLAYER_INPUT:
      if (currentTime - inputStartTime > getInputTimeLimit()) {
        SDL_Log("Input time limit reached, switching to GAME_OVER");
        currentPhase = GAME_OVER;
        patternStartTime = currentTime;
      }
      break;
      
    case PHASE_COMPLETE:
      if (currentTime - patternStartTime > 1000) {
        startNextLevel();
      }
      break;
      
    case GAME_OVER:
      if (currentTime - patternStartTime > 2000) {
        resetLevel();
      }
      break;
      
    case GAME_WIN:
      if (currentTime - patternStartTime > 2000) {
        GameState::setCurrentLevel(GameState::current_level + 1);
        GameState::isLoading = true;
      }
      break;
  }
  
  // Call base class update
  Level::update();
}

void LevelLamp::handleEvents(SDL_Event *event, SDL_Renderer *renderer) {
  Level::handleEvents(event, renderer);
  
  // Handle mouse clicks to toggle lamps during player input phase
  if (currentPhase == PLAYER_INPUT && event->type == SDL_MOUSEBUTTONDOWN && 
      event->button.button == SDL_BUTTON_LEFT) {
    int mouseX = event->button.x;
    int mouseY = event->button.y;
    
    // Calculate grid positioning
    int lampSize = rect.w;
    int spacing = 10;
    int gridWidth = 8 * (lampSize + spacing);
    int gridHeight = 6 * (lampSize + spacing);
    int offsetX = (W_WIDTH - gridWidth) / 2;
    int offsetY = (W_HEIGHT - gridHeight) / 2;
    
    // Check if click is within grid
    if (mouseX >= offsetX && mouseX < offsetX + gridWidth &&
        mouseY >= offsetY && mouseY < offsetY + gridHeight) {
        
      // Calculate which lamp was clicked
      int i = (mouseX - offsetX) / (lampSize + spacing);
      int j = (mouseY - offsetY) / (lampSize + spacing);
      
      // Ensure within bounds
      if (i >= 0 && i < 8 && j >= 0 && j < 6) {
        SOUND_MANAGER.playSoundEffect("click");
        // Toggle lamp state
        if (input_lamps[i][j].state == OFF) {
          input_lamps[i][j].state = ON_GREEN;
        } else if (input_lamps[i][j].state == ON_GREEN) {
          input_lamps[i][j].state = ON_RED;
        } else {
          input_lamps[i][j].state = OFF;
        }
        
        // Check if pattern is complete
        if (checkPatternMatch()) {
          currentPhase = PHASE_COMPLETE;
          patternStartTime = SDL_GetTicks();
        }
      }

    }
  }
}

void LevelLamp::generatePattern() {
  // Reset all lamps
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 6; j++) {
      // Calculate grid positioning to center on screen
      int lampSize = rect.w;
      int spacing = 10;
      int gridWidth = 8 * (lampSize + spacing);
      int gridHeight = 6 * (lampSize + spacing);
      int offsetX = (W_WIDTH - gridWidth) / 2;
      int offsetY = (W_HEIGHT - gridHeight) / 2;
      
      // Set proper positions
      lamps[i][j].x = offsetX + i * (lampSize + spacing);
      lamps[i][j].y = offsetY + j * (lampSize + spacing);
      input_lamps[i][j].x = lamps[i][j].x;
      input_lamps[i][j].y = lamps[i][j].y;
      
      lamps[i][j].state = OFF;
      input_lamps[i][j].state = OFF;
    }
  }
  
  // Generate pattern based on current level
  int patternCount = patternsPerLevel[currentLevel < 3 ? currentLevel : 2];
  int lit = 0;
  
  while (lit < patternCount) {
    int i = rand() % 8;
    int j = rand() % 6;
    
    // Only set if currently OFF
    if (lamps[i][j].state == OFF) {
      lamps[i][j].state = (rand() % 2 == 0) ? ON_GREEN : ON_RED;
      lit++;
    }
  }
  
  SDL_Log("Generated pattern with %d lamps for level %d", patternCount, currentLevel + 1);
}

void LevelLamp::renderPhaseInfo(SDL_Renderer *renderer) {
  if (!gameFont) return;
  
  std::string phaseText;
  SDL_Color textColor = {255, 255, 255, 255};
  
  switch (currentPhase) {
    case SHOWING_PATTERN:
      phaseText = "Memorize Pattern";
      break;
    case PLAYER_INPUT:
      phaseText = "Reproduce Pattern";
      break;
    case PHASE_COMPLETE:
      phaseText = "Level Complete";
      break;
    case GAME_OVER:
      phaseText = "Game Over";
      break;
    case GAME_WIN:
      phaseText = "You Win";
      break;
  }
  
  // Render phase text
  SDL_Surface* textSurface = TTF_RenderText_Solid(gameFont, phaseText.c_str(), textColor);
  if (textSurface) {
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture) {
      SDL_Rect textRect = {(W_WIDTH - textSurface->w) / 2, 30, textSurface->w, textSurface->h};
      SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
      SDL_DestroyTexture(textTexture);
    }
    SDL_FreeSurface(textSurface);
  }
  
  // Render level info
  std::string levelText = "Level: " + std::to_string(currentLevel + 1);
  textSurface = TTF_RenderText_Solid(gameFont, levelText.c_str(), textColor);
  if (textSurface) {
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture) {
      SDL_Rect textRect = {20, 20, textSurface->w, textSurface->h};
      SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
      SDL_DestroyTexture(textTexture);
    }
    SDL_FreeSurface(textSurface);
  }
}

bool LevelLamp::checkPatternMatch() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 6; j++) {
      if (lamps[i][j].state != input_lamps[i][j].state) {
        return false;
      }
    }
  }
  return true;
}

void LevelLamp::resetLevel() {
  currentLevel = 0;
  generatePattern();
  current_lamps = lamps;
  currentPhase = SHOWING_PATTERN;
  patternStartTime = SDL_GetTicks();
}

void LevelLamp::startNextLevel() {
  currentLevel++;
  
  if (currentLevel >= 3) {
    currentPhase = GAME_WIN;
  } else {
    generatePattern();
    current_lamps = lamps;
    currentPhase = SHOWING_PATTERN;
  }
  
  patternStartTime = SDL_GetTicks();
}

bool LevelLamp::isLevelComplete() {
  return currentPhase == GAME_WIN;
}

// Add new method to render timer
void LevelLamp::renderTimer(SDL_Renderer *renderer) {
  if (!gameFont) return;
  
  uint32_t currentTime = SDL_GetTicks();
  int timeRemaining = 0;
  
  // Calculate time remaining based on current phase
  if (currentPhase == SHOWING_PATTERN) {
    timeRemaining = (getPatternShowTime() - (currentTime - patternStartTime)) / 1000 + 1;
  } else if (currentPhase == PLAYER_INPUT) {
    timeRemaining = (getInputTimeLimit() - (currentTime - inputStartTime)) / 1000 + 1;
  } else {
    return; // Don't show timer for other phases
  }
  
  // Ensure time doesn't go negative
  if (timeRemaining < 0) timeRemaining = 0;
  
  // Create timer text
  std::string timerText = "Time: " + std::to_string(timeRemaining);
  SDL_Color textColor = {255, 255, 0, 255}; // Yellow for timer
  
  // Render timer text
  SDL_Surface* textSurface = TTF_RenderText_Solid(gameFont, timerText.c_str(), textColor);
  if (textSurface) {
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture) {
      SDL_Rect textRect = {(W_WIDTH - textSurface->w) / 2, 70, textSurface->w, textSurface->h};
      SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
      SDL_DestroyTexture(textTexture);
    }
    SDL_FreeSurface(textSurface);
  }
}