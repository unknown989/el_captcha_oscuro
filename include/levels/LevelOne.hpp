#pragma once
#include "Level.hpp"
#include <SDL2/SDL.h>

class LevelOne : public Level {
  public:
    LevelOne(SDL_Renderer* renderer);
    ~LevelOne();
    void render(SDL_Renderer* renderer);
    void handleEvents(SDL_Event* event);
};

LevelOne::LevelOne(SDL_Renderer* renderer) : Level(renderer) {
    SDL_Log("Loading level one...");
    readLevel("levels/lvl1.txt", renderer);
    loadLevelBackground("assets/backgrounds/level1.png", renderer);
    level_music = Mix_LoadMUS("assets/La Fiola 2.wav");
    if (level_music == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load music: %s, music wont be played", Mix_GetError());
    }
    SDL_Log("Level one loaded. Number of blocks: %zu", blocks.size());
}

void LevelOne::render(SDL_Renderer* renderer) {
    Level::render(renderer);
}

void LevelOne::handleEvents(SDL_Event* event) {
    Level::handleEvents(event);
    if (isLoaded && !isPlayingMusic) {
        Mix_VolumeMusic(100);
        Mix_PlayMusic(level_music, -1);
        isPlayingMusic = true;
    }

}

LevelOne::~LevelOne() {
    // Clean up physics space if needed
}