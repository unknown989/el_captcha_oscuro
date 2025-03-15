#pragma once
#include "Level.hpp"
#include "GameState.hpp"

class Credits : public Level {
public:
    Credits(SDL_Renderer* renderer);
    void render(SDL_Renderer* renderer) override;
    void update() ;
    void handleEvents(SDL_Event* event, SDL_Renderer* renderer) override;

private:
    uint32_t startTime;
    const uint32_t creditDuration = 10000; // 10 seconds in milliseconds
};

Credits::Credits(SDL_Renderer* renderer) : Level(renderer) {
    startTime = SDL_GetTicks();
    loadLevelBackground("assets/backgrounds/credits.png", renderer); // Create this image
}

void Credits::render(SDL_Renderer* renderer) {
    Level::render(renderer); // Render base level background
}

void Credits::update() {
    // Check if credits duration has passed
    if (SDL_GetTicks() - startTime > creditDuration) {
        GameState::setCurrentLevel(-1); // Return to main menu
    }
    Level::update();
}

void Credits::handleEvents(SDL_Event* event, SDL_Renderer* renderer) {
    // Allow skipping credits with ESC or Space
    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_ESCAPE || event->key.keysym.sym == SDLK_SPACE) {
            GameState::setCurrentLevel(-1);
        }
    }
    Level::handleEvents(event, renderer);
}
// Code created by Mouttaki Omar(王明清)