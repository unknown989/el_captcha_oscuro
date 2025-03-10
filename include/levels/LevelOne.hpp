#pragma once
#include "Level.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

class LevelOne : public Level {
  public:
    LevelOne(SDL_Renderer* renderer);
    ~LevelOne();
    void render(SDL_Renderer* renderer);
    void handleEvents(SDL_Event* event, SDL_Renderer* renderer);
  
  private:
    TTF_Font* statsFont = nullptr;
    void renderPlayerStats(SDL_Renderer* renderer);
};

LevelOne::LevelOne(SDL_Renderer* renderer) : Level(renderer) {
    SDL_Log("Loading level one...");
    readLevel("levels/lvl1.txt", renderer);
    loadLevelBackground("assets/backgrounds/level1.png", renderer);
    level_music = Mix_LoadMUS("assets/music/La Fiola 2.wav");
    if (level_music == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load music: %s, music wont be played", Mix_GetError());
    }
    
    // Load font for player stats
    statsFont = TTF_OpenFont("assets/fonts/ARCADECLASSIC.ttf", 24);
    if (!statsFont) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font: %s", TTF_GetError());
    }
    
    SDL_Log("Level one loaded. Number of blocks: %zu", blocks.size());
    player->shouldShot(true);
}

void LevelOne::render(SDL_Renderer* renderer) {
    Level::render(renderer);
    
    // Make sure player bullets are updated in the level's update cycle
    if (player) {
        player->updateBullets();
    }
    
    // Render player stats
    renderPlayerStats(renderer);
}

void LevelOne::renderPlayerStats(SDL_Renderer* renderer) {
    if (!player || !statsFont) return;
    
    // Create health text in format "currentHealth/maxHealth"
    std::string healthText = std::to_string(player->getHealth()) + " I " + 
                             std::to_string(player->getMaxHealth());
    
    // Create bullets text
    std::string bulletsText = std::to_string(player->getBullets());
    
    // Set text color
    SDL_Color textColor = {255, 255, 255, 255}; // White
    
    // Render health text
    SDL_Surface* healthSurface = TTF_RenderText_Solid(statsFont, healthText.c_str(), textColor);
    if (healthSurface) {
        SDL_Texture* healthTexture = SDL_CreateTextureFromSurface(renderer, healthSurface);
        if (healthTexture) {
            SDL_Rect healthRect = {20, 20, healthSurface->w, healthSurface->h};
            SDL_RenderCopy(renderer, healthTexture, NULL, &healthRect);
            SDL_DestroyTexture(healthTexture);
        }
        SDL_FreeSurface(healthSurface);
    }
    
    // Render bullets text
    SDL_Surface* bulletsSurface = TTF_RenderText_Solid(statsFont, bulletsText.c_str(), textColor);
    if (bulletsSurface) {
        SDL_Texture* bulletsTexture = SDL_CreateTextureFromSurface(renderer, bulletsSurface);
        if (bulletsTexture) {
            SDL_Rect bulletsRect = {20, 50, bulletsSurface->w, bulletsSurface->h};
            SDL_RenderCopy(renderer, bulletsTexture, NULL, &bulletsRect);
            SDL_DestroyTexture(bulletsTexture);
        }
        SDL_FreeSurface(bulletsSurface);
    }
}

void LevelOne::handleEvents(SDL_Event* event, SDL_Renderer* renderer) {
    Level::handleEvents(event, renderer);
    if (isLoaded && !isPlayingMusic) {
        Mix_VolumeMusic(100);
        Mix_PlayMusic(level_music, -1);
        isPlayingMusic = true;
    }
}

LevelOne::~LevelOne() {
    // Clean up font
    if (statsFont) {
        TTF_CloseFont(statsFont);
        statsFont = nullptr;
    }
}