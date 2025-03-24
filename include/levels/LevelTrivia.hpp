#pragma once
#include "Level.hpp"
#include "GameState.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include "music.hpp"

class LevelTrivia : public Level {
public:
  LevelTrivia(SDL_Renderer *renderer);
  ~LevelTrivia();

  void render(SDL_Renderer *renderer) override;
  void handleEvents(SDL_Event *event, SDL_Renderer *renderer) override;
  void update() override;

private:
  // Game state
  enum GamePhase {
    SHOWING_QUESTION,
    GAME_OVER,
    GAME_WIN
  };
  
  GamePhase currentPhase = SHOWING_QUESTION;
  int currentQuestion = 0;
  const int totalQuestions = 4;
  std::string playerInput = "";
  bool inputActive = true;
  
  // Correct answers for each question
  const std::string answers[4] = {"K", "7", "36", "85"};
  
  // Question order tracking
  std::vector<int> questionOrder;
  
  // Textures
  std::vector<SDL_Texture*> questionBackgrounds;
  SDL_Texture* inputBoxTexture = nullptr;
  SDL_Rect inputBoxRect;
  
  // Font and timer
  TTF_Font* gameFont = nullptr;
  uint32_t questionStartTime = 0;
  uint32_t timeLimit = 15000; // 15 seconds per question
  
  // Helper methods
  void renderQuestionInfo(SDL_Renderer *renderer);
  void renderTimer(SDL_Renderer *renderer);
  void renderInputBox(SDL_Renderer *renderer);
  void nextQuestion();
  void checkAnswer();
  void shuffleQuestions();
};

LevelTrivia::LevelTrivia(SDL_Renderer *renderer) : Level(renderer) {
  // Load question backgrounds
  for (int i = 1; i <= totalQuestions; i++) {
    std::string path = "assets/trivia/" + std::to_string(i) + ".png";
    SDL_Texture* texture = nullptr;
    SDL_Surface* surface = Texture::loadFromFile(path.c_str(), renderer, texture);
    
    if (!texture) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading question background %d", i);
    } else {
      questionBackgrounds.push_back(texture);
    }
    
    if (surface) SDL_FreeSurface(surface);
  }
  
  // Initialize question order
  questionOrder.resize(totalQuestions);
  for (int i = 0; i < totalQuestions; i++) {
    questionOrder[i] = i;
  }
  
  // Shuffle questions initially
  shuffleQuestions();
  
  // Load input box texture
  SDL_Surface* inputBoxSurface = Texture::loadFromFile("assets/input_box/box.png", renderer, inputBoxTexture);
  if (!inputBoxTexture) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading input box texture");
  } else {
    // Set input box position (centered horizontally, near bottom of screen)
    inputBoxRect.w = 500;
    inputBoxRect.h = 500;
    inputBoxRect.x = (W_WIDTH - inputBoxRect.w) / 2;
    inputBoxRect.y = W_HEIGHT - 500;
  }
  if (inputBoxSurface) SDL_FreeSurface(inputBoxSurface);
  
  // Load font
  gameFont = TTF_OpenFont("assets/fonts/ARCADECLASSIC.ttf", 24);
  if (!gameFont) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font: %s", TTF_GetError());
  }
  
  // Initialize timer
  questionStartTime = SDL_GetTicks();
  
  SDL_Log("Trivia level loaded successfully");
  MUSIC.playMusic("amicitia");
}

LevelTrivia::~LevelTrivia() {
  // Clean up textures
  for (auto texture : questionBackgrounds) {
    if (texture) SDL_DestroyTexture(texture);
  }
  
  if (inputBoxTexture) SDL_DestroyTexture(inputBoxTexture);
  
  // Clean up font
  if (gameFont) TTF_CloseFont(gameFont);
}

void LevelTrivia::render(SDL_Renderer *renderer) {
  // Render current question background instead of calling Level::render()
  if (currentQuestion < totalQuestions && questionOrder[currentQuestion] < questionBackgrounds.size()) {
    SDL_RenderCopy(renderer, questionBackgrounds[questionOrder[currentQuestion]], NULL, NULL);
  } else {
    // Fallback to black background if texture not available
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
  }
  
  // Render question info (number and timer)
  renderQuestionInfo(renderer);
  
  // Render timer
  renderTimer(renderer);
  
  // Render input box and current input
  renderInputBox(renderer);
}

void LevelTrivia::update() {
  uint32_t currentTime = SDL_GetTicks();

  // Check if time limit is reached
  if (currentPhase == SHOWING_QUESTION && 
      currentTime - questionStartTime > timeLimit) {
    // Time's up for this question - move to game over
    currentPhase = GAME_OVER;
    questionStartTime = currentTime;
  }
  
  // Handle phase transitions
  if (currentPhase == GAME_OVER && currentTime - questionStartTime > 2000) {
    // Reset to first question after showing game over for 2 seconds
    currentQuestion = 0;
    playerInput = "";
    currentPhase = SHOWING_QUESTION;
    
    // Shuffle questions for a different order
    shuffleQuestions();
    
    questionStartTime = currentTime;
  } else if (currentPhase == GAME_WIN && currentTime - questionStartTime > 2000) {
    // Advance to next level after showing win for 2 seconds
    GameState::setCurrentLevel(GameState::current_level + 1);
    GameState::isLoading = true;
  }
}

void LevelTrivia::handleEvents(SDL_Event *event, SDL_Renderer *renderer) {
  Level::handleEvents(event, renderer);
  
  if (currentPhase != SHOWING_QUESTION) return;
  
  // Handle text input
  if (event->type == SDL_KEYDOWN) {
    if (event->key.keysym.sym == SDLK_BACKSPACE && playerInput.length() > 0) {
      // Handle backspace
      playerInput.pop_back();
    } else if (event->key.keysym.sym == SDLK_RETURN) {
      // Handle enter key - check answer
      checkAnswer();
    }
  } else if (event->type == SDL_TEXTINPUT) {
    // Add text input to player's answer
    playerInput += event->text.text;
  }
}

void LevelTrivia::renderQuestionInfo(SDL_Renderer *renderer) {
  if (!gameFont) return;
  
  SDL_Color textColor = {255, 255, 255, 255};
  
  // Render question number
  std::string questionText = "Question: " + std::to_string(currentQuestion + 1) + 
                             " / " + std::to_string(totalQuestions);
  
  SDL_Surface* textSurface = TTF_RenderText_Solid(gameFont, questionText.c_str(), textColor);
  if (textSurface) {
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture) {
      SDL_Rect textRect = {(W_WIDTH - textSurface->w) / 2, 30, textSurface->w, textSurface->h};
      SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
      SDL_DestroyTexture(textTexture);
    }
    SDL_FreeSurface(textSurface);
  }
  
  // Render phase text
  std::string phaseText;
  switch (currentPhase) {
    case SHOWING_QUESTION:
      phaseText = "Enter your answer";
      break;
    case GAME_OVER:
      phaseText = "Game Over";
      break;
    case GAME_WIN:
      phaseText = "You Win!";
      break;
  }
  
  textSurface = TTF_RenderText_Solid(gameFont, phaseText.c_str(), textColor);
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

void LevelTrivia::renderTimer(SDL_Renderer *renderer) {
  if (!gameFont) return;
  
  uint32_t currentTime = SDL_GetTicks();
  int timeRemaining = 0;
  
  // Calculate time remaining
  if (currentPhase == SHOWING_QUESTION) {
    timeRemaining = (timeLimit - (currentTime - questionStartTime)) / 1000 + 1;
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
      SDL_Rect textRect = {(W_WIDTH - textSurface->w) / 2, 110, textSurface->w, textSurface->h};
      SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
      SDL_DestroyTexture(textTexture);
    }
    SDL_FreeSurface(textSurface);
  }
}

void LevelTrivia::renderInputBox(SDL_Renderer *renderer) {
  // Render input box
  if (inputBoxTexture) {
    SDL_RenderCopy(renderer, inputBoxTexture, NULL, &inputBoxRect);
  }
  
  // Render player input text
  if (!gameFont) return;
  
  SDL_Color textColor = {255, 255, 255, 255}; // White text for input
  
  SDL_Surface* textSurface = TTF_RenderText_Solid(gameFont, playerInput.c_str(), textColor);
  if (textSurface) {
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture) {
      SDL_Rect textRect = {
        inputBoxRect.x + (inputBoxRect.w - textSurface->w) / 2,
        inputBoxRect.y + (inputBoxRect.h - textSurface->h) / 2,
        textSurface->w,
        textSurface->h
      };
      SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
      SDL_DestroyTexture(textTexture);
    }
    SDL_FreeSurface(textSurface);
  }
}

void LevelTrivia::nextQuestion() {
  currentQuestion++;
  playerInput = "";
  
  if (currentQuestion >= totalQuestions) {
    // All questions answered correctly
    currentPhase = GAME_WIN;
  } else {
    // Reset timer for next question
    questionStartTime = SDL_GetTicks();
  }
}

void LevelTrivia::checkAnswer() {
  if (currentQuestion < totalQuestions) {
    // Check if answer is correct
    if (playerInput == answers[questionOrder[currentQuestion]]) {
      nextQuestion();
    } else {
      // Wrong answer
      currentPhase = GAME_OVER;
      questionStartTime = SDL_GetTicks();
    }
  }
}

void LevelTrivia::shuffleQuestions() {
  // Seed the random number generator
  srand(static_cast<unsigned int>(time(nullptr)));
  
  // Fisher-Yates shuffle algorithm
  for (int i = totalQuestions - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    std::swap(questionOrder[i], questionOrder[j]);
  }
}