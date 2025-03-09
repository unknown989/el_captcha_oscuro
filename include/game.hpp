#include "CONSTANTS.hpp"
#include "mainmenu.hpp"
#include "sprite.hpp"
#include <GameState.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <levels/Level.hpp>
#include <levels/LevelOne.hpp>
#include <vector>
class Game {

public:
  Game();
  void run();
  void update();
  void render();
  void handleEvents();
  void clean();

private:
  mainmenu *menu;
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;
  bool isPlayingMenuMusic = true;
  Mix_Music *menumusic;
  std::vector<Level *> levels;
  int current_level = 0;
};

Game::Game() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                 SDL_GetError());
    exit(1);
  }
  initializeDisplayMode();

  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "SDL_image could not initialize! SDL_image Error: %s\n",
                 IMG_GetError());
    exit(1);
  }

  if (TTF_Init() == -1) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "SDL_ttf could not initialize! SDL_ttf Error: %s\n",
                 TTF_GetError());
    exit(1);
  }
  this->window = SDL_CreateWindow(W_NAME, 300, 100, W_WIDTH, W_HEIGHT, W_TYPE);
  if (this->window == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s",
                 SDL_GetError());
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "SDL_mixer could not initialize! SDL_mixer Error: %s\n",
                 Mix_GetError());
    exit(1);
  }
  this->renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (this->renderer == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s",
                 SDL_GetError());
  }
  menumusic = Mix_LoadMUS("assets/Sadness to happiness.wav");
  if (menumusic == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Couldn't load music: %s, music wont be played",
                 Mix_GetError());
  }
  if (isPlayingMenuMusic && menumusic != NULL) {
    Mix_VolumeMusic(10);
    Mix_PlayMusic(menumusic, -1);
  }
  menu = new mainmenu(
      renderer,
      [] {
        GameState::isMenu = false;
        GameState::setCurrentLevel(0);
      },
      NULL, [] { GameState::running = false; });
  levels.push_back(new LevelOne(renderer));
}
void Game::update() {
  if (GameState::isMenu) {
    isPlayingMenuMusic = true;
  } else {
    isPlayingMenuMusic = false;
  }

  // Make sure to call level update
  if (!GameState::isMenu && GameState::current_level >= 0 &&
      GameState::current_level < levels.size()) {
    if (!levels[GameState::current_level]->isLevelLoaded()) {
      levels[Game::current_level]->loadLevel();
    }
    levels[GameState::current_level]->update();
  }
}
void Game::run() {
  while (GameState::running) {
    handleEvents();
    update();
    render();
  }
}
void Game::render() {
  if (renderer == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer is not initialized!");
    return;
  }
  SDL_RenderClear(renderer);

  if (GameState::isMenu || GameState::current_level < 0) {
    if (menu == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Menu is not initialized!");
      return;
    }
    menu->render(renderer);
  } else {
    if (GameState::current_level < 0 ||
        GameState::current_level >= levels.size()) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Invalid current level: %d, levels size: %zu",
                   GameState::current_level, levels.size());
      return;
    }
    // render a gray screen
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    levels[GameState::current_level]->render(renderer);
  }

  SDL_RenderPresent(renderer);
}

void Game::handleEvents() {
  SDL_PollEvent(&event);
  if (event.type == SDL_QUIT) {
    GameState::running = false;
  }

  if (GameState::isMenu) {
    menu->handleEvents(event);
  }
  if (GameState::current_level >= 0) {
    levels[GameState::current_level]->handleEvents(&event, renderer);
  }

  if (event.type == SDL_KEYDOWN) {
    // make sdl break the game if Q was pressed
    if (event.key.keysym.sym == SDLK_q) {
      GameState::running = false;
    }
  }
}
void Game::clean() {
  Mix_FreeMusic(menumusic);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  Mix_Quit();
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}