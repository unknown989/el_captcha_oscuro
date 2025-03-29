#pragma once
#include "CONSTANTS.hpp"
#include "buttons.hpp"
#include "textures.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <functional>
#include "soundmanager.hpp"
class mainmenu {
private:
  Button buttons[3];
  SDL_Rect background_rect;
  TTF_Font *font;
  SDL_Rect text_rect;
  SDL_Texture *text_texture;
  SDL_Texture *background_tex = NULL;

public:
  mainmenu(SDL_Renderer *renderer, std::function<void()> start_callback,
           std::function<void()> settings_callback,
           std::function<void()> exit_callback);
  ~mainmenu();
  void render(SDL_Renderer *renderer);
  void handleEvents(SDL_Event event);
};
mainmenu::mainmenu(SDL_Renderer *renderer, std::function<void()> start_callback,
                   std::function<void()> settings_callback,
                   std::function<void()> exit_callback) {
  // menu's background
  SDL_Surface *a = Texture::loadFromFile("assets/backgrounds/menu.png",
                                         renderer, background_tex);
  if (a == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Unable to load image %s! SDL_image Error: %s\n",
                 "mainmenu.png", SDL_GetError());
  }
  SDL_FreeSurface(a);
  background_rect.x = 0;
  background_rect.y = 0;
  background_rect.w = W_WIDTH;
  background_rect.h = W_HEIGHT;
  // menu's font
  this->font = TTF_OpenFont("assets/fonts/ARCADECLASSIC.TTF", 60);
  SDL_Color color = {255, 255, 255};
  SDL_Surface *text_surface = TTF_RenderText_Solid(font, W_NAME, color);
  text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
  // centering the text
  text_rect.x = W_WIDTH / 2 - text_surface->w / 2;
  text_rect.y = W_HEIGHT / 4 - text_surface->h / 2;
  text_rect.w = text_surface->w;
  text_rect.h = text_surface->h;
  SDL_FreeSurface(text_surface);

  // loading the buttons texture
  buttons[0].loadFromFile("assets/buttons/play.png", renderer);
  buttons[0].setSize(200, 100);
  buttons[0].setPosition(W_WIDTH / 2 - buttons[0].getRect().w / 2,
                         W_HEIGHT / 2 - buttons[1].getRect().h / 2);
  // setting the buttons callback
  buttons[0].setCallback(start_callback);

  buttons[1].loadFromFile("assets/buttons/settings.png", renderer);
  buttons[1].setSize(200, 100);
  buttons[1].setPosition(W_WIDTH / 2 - buttons[1].getRect().w / 2,
                         W_HEIGHT / 2 - buttons[1].getRect().h / 2 + 200);
  buttons[1].setCallback(settings_callback);

  buttons[2].loadFromFile("assets/buttons/exit.png", renderer);
  buttons[2].setSize(200, 100);
  buttons[2].setPosition(W_WIDTH / 2 - buttons[2].getRect().w / 2,
                         W_HEIGHT / 2 - buttons[2].getRect().h / 2 + 350);
  buttons[2].setCallback(exit_callback);
  SOUND_MANAGER.playMusic("menu");
  SOUND_MANAGER.setMusicVolume(10);
}

mainmenu::~mainmenu() {
  if (font) {
    TTF_CloseFont(font);
    font = nullptr;
  }
  SDL_DestroyTexture(background_tex);
  SDL_DestroyTexture(text_texture);
}

void mainmenu::render(SDL_Renderer *renderer) {

  // setting up the alpha channel
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_RenderCopy(renderer, background_tex, NULL, &background_rect);
  // drawing a semi-transparent black rectangle
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
  SDL_RenderFillRect(renderer, &background_rect);
  buttons[0].render(renderer);
  buttons[1].render(renderer);
  buttons[2].render(renderer);
  SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
}

void mainmenu::handleEvents(SDL_Event event) {
  buttons[0].handleEvents(event);
  buttons[1].handleEvents(event);
  buttons[2].handleEvents(event);
}
// Code created by Mouttaki Omar(王明清)
