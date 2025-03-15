#pragma once
#include <SDL2/SDL.h>
constexpr const bool IS_DEBUG = false; // Set to true to run in windowed mode
int W_WIDTH = 800;
int W_HEIGHT = 450;
constexpr const char* W_NAME = "El Captcha Oscuro";
constexpr const SDL_WindowFlags W_TYPE = IS_DEBUG ? SDL_WINDOW_OPENGL : SDL_WINDOW_FULLSCREEN;
constexpr const char* W_ASSETS = "./assets/";
constexpr const char* W_FONTS = "./fonts/";
constexpr const int W_SPRITESIZE = 64;

void initializeDisplayMode() {
    // Get the current display mode to know the full resolution of the screen
    SDL_DisplayMode displayMode;
    int a = SDL_GetCurrentDisplayMode(0, &displayMode);
    if (a != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not get display mode: %s", SDL_GetError());
        exit(1);
    }
    if (!IS_DEBUG) {
        W_WIDTH = displayMode.w;
        W_HEIGHT = displayMode.h;
    }
}

// Code created by Mouttaki Omar(王明清)