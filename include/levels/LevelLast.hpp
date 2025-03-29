#pragma once
#include "GameState.hpp"
#include "Level.hpp"
#include "enemy.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdint>
#include <string>
#include "music.hpp"


class LevelLast : public Level {
public:
  LevelLast(SDL_Renderer *renderer);
  ~LevelLast();
  void render(SDL_Renderer *renderer);
  void handleEvents(SDL_Event *event, SDL_Renderer *renderer);
  void update() override;

private:
  TTF_Font *statsFont = nullptr;
  void renderPlayerStats(SDL_Renderer *renderer);
  void renderHealthBars(SDL_Renderer *renderer);

  float enemyMovementTimer = 0;
  float enemyMovementInterval = 120; // Change direction every 2 seconds (60fps)
  int movementPattern = 0;           // Current movement pattern
  bool isEnemyAggressive = false;    // Becomes true when health is below 50%

  float timeScale = 1.0f;
  uint32_t lastFrameTime = 0;

  // Game state flags
  bool isGameOver = false;
  bool playerWon = false;

  // Method to render game over or win screen
  void renderGameEndScreen(SDL_Renderer *renderer);

  // Method to restart the level
  void restartLevel(SDL_Renderer *renderer);
};

LevelLast::LevelLast(SDL_Renderer *renderer) : Level(renderer) {
  SDL_Log("Loading level one...");
  readLevel("levels/lvl_last.txt", renderer);
  loadLevelBackground("assets/backgrounds/bosslevel.png", renderer);

  // Load font for player stats
  statsFont = TTF_OpenFont("assets/fonts/ARCADECLASSIC.ttf", 24);
  if (!statsFont) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font: %s",
                 TTF_GetError());
  }

  MUSIC.playMusic("boss");
  SDL_Log("Level one loaded. Number of blocks: %zu", blocks.size());
  player->shouldShot(true);
  enemy->linkToPlayer(player);
}

void LevelLast::update() {
  Level::update();
  if (player) {
    int playerHealth = player->getHealth();
    if (playerHealth <= 0) {
      SDL_Log("Player health is %d - triggering Game Over", playerHealth);
      isGameOver = true;
      playerWon = false;
    }
  }

  if (enemy) {
    int enemyHealth = enemy->getHealth();
    if (enemyHealth <= 0) {
      SDL_Log("Enemy health is %d - triggering Win condition", enemyHealth);
      isGameOver = true;
      playerWon = true;
    }
  }

  // If game is over, don't update anything else
  if (isGameOver) {
    return;
  }

  // Update time scale based on player health
  if (player) {
    float healthPercent =
        static_cast<float>(player->getHealth()) / player->getMaxHealth();
    if (healthPercent < 0.5f) {
      // Slow down the game when player health is low (60% normal speed)
      timeScale = 0.6f;
    } else {
      // Normal speed
      timeScale = 1.0f;
    }

    // Apply time scaling by delaying updates
    uint32_t currentTime = SDL_GetTicks();
    uint32_t elapsedTime = currentTime - lastFrameTime;

    // Only update if enough time has passed based on time scale
    // This time scaling system might prevent death detection
    if (elapsedTime < (16 / timeScale)) {
      return; // This return skips the rest of update()
    }

    lastFrameTime = currentTime;
  }
  Level::update();

  if (enemy) {
    // Update enemy movement pattern
    enemyMovementTimer++;

    // Check if enemy should change movement pattern
    if (enemyMovementTimer >= enemyMovementInterval) {
      enemyMovementTimer = 0;

      // Randomly select a new movement pattern
      movementPattern = rand() % 4; // 5 different patterns

      // Make intervals shorter as enemy health decreases
      float healthPercent = static_cast<float>(enemy->getHealth()) / 400.0f;
      enemyMovementInterval = 120 * (0.3f + (0.7f * healthPercent));

      // Enemy becomes more aggressive at low health
      isEnemyAggressive = (healthPercent < 0.5f);
    }

    // Apply the selected movement pattern
    int screenWidth, screenHeight;
    SDL_GetRendererOutputSize(SDL_GetRenderer(SDL_GetWindowFromID(1)),
                              &screenWidth, &screenHeight);

    float enemyX = enemy->getX();
    float enemyY = enemy->getY();
    float moveSpeed = isEnemyAggressive ? 4.0f : 2.0f;

    switch (movementPattern) {
    case 0: // Circle around player
      if (player) {
        float angle = enemyMovementTimer * 0.05f;
        float radius = isEnemyAggressive ? 150 : 250;
        float targetX = player->getX() + cos(angle) * radius;
        float targetY = player->getY() + sin(angle) * radius;

        // Move towards target position
        float dx = targetX - enemyX;
        float dy = targetY - enemyY;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist > 5) {
          enemy->setPosition(
              enemyX + (dx / dist) * moveSpeed,
              std::max(200.0f, enemyY + (dy / dist) * moveSpeed));
        }
      }
      break;
    case 1: // Zigzag horizontal movement
    {
      float zigzagSpeed = isEnemyAggressive ? 6.0f : 3.0f;
      float zigzagY = sin(enemyMovementTimer * 0.1f) * 50;
      enemy->setPosition(enemyX +
                             (isEnemyAggressive ? -zigzagSpeed : zigzagSpeed),
                         std::max(200.0f, enemyY + zigzagY * 0.5f));

      // Bounce off screen edges
      if (enemyX < 50 || enemyX > screenWidth - 50) {
        movementPattern = 3; // Switch to vertical movement
      }
    } break;

    case 2: // Vertical bouncing
    {
      float bounceSpeed = isEnemyAggressive ? 5.0f : 2.5f;
      enemy->setPosition(
          enemyX, std::max(200.0f, enemyY + (sin(enemyMovementTimer * 0.05f) *
                                             bounceSpeed)));
    } break;

    case 3: // Chase player (only when aggressive)
      if (player && isEnemyAggressive) {
        float dx = player->getX() - enemyX;
        float dy = player->getY() - enemyY;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist > 100) { // Don't get too close
          enemy->setPosition(
              enemyX + (dx / dist) * moveSpeed,
              std::max(200.0f, enemyY + (dy / dist) * moveSpeed));
        }
      }
      break;
    }
  }
}

void LevelLast::render(SDL_Renderer *renderer) {
  // Add sanity check at start of render
  if (!isGameOver) {
    if (player && player->getHealth() <= 0) {
      SDL_Log("Late detection - Player death in render");
      isGameOver = true;
      playerWon = false;
    }
    if (enemy && enemy->getHealth() <= 0) {
      SDL_Log("Late detection - Enemy death in render");
      isGameOver = true;
      playerWon = true;
    }
  }

  // Render the base level
  Level::render(renderer);

  // Apply screen shake and blur if player health is low
  bool applyScreenEffects = false;
  if (player && player->getHealth() <= player->getMaxHealth() / 2 &&
      !isGameOver) {
    applyScreenEffects = true;

    // Screen shake effect - make it more intense when game is slowed down
    if (rand() % 100 < 30) {            // 30% chance to shake
      int shakeAmount = 3 + rand() % 3; // Shake between 3-5 pixels
      if (timeScale < 1.0f) {
        shakeAmount += 2; // More intense shake during slow motion
      }
      SDL_Rect viewport = {shakeAmount - (rand() % (shakeAmount * 2)),
                           shakeAmount - (rand() % (shakeAmount * 2)), 0, 0};
      SDL_RenderSetViewport(renderer, &viewport);
    }
  }

  if (enemy && !isGameOver) {
    enemy->update();
    enemy->render(renderer);

    // Check player bullets hitting enemy
    if (player) {
      // Store bullets to remove
      std::vector<Bullet *> bulletsToRemove;

      // Check collisions
      for (const auto &bullet : player->getBulletsObj()) {
        if (enemy->checkBulletCollision(bullet->getX(), bullet->getY())) {
          enemy->takeDamage(15);
          bulletsToRemove.push_back(bullet.get());
        }
      }

      // Remove collided bullets through the player interface
      for (auto bullet : bulletsToRemove) {
        player->removeBullet(bullet);
      }
    }
  }

  // Make sure player bullets are updated in the level's update cycle
  if (player && !isGameOver) {
    player->updateBullets();
  }

  // Reset viewport if we applied screen shake
  if (applyScreenEffects) {
    SDL_RenderSetViewport(renderer, NULL);

    // Apply blur effect by rendering a semi-transparent overlay
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 50); // Semi-transparent black
    SDL_Rect fullScreen;
    SDL_GetRendererOutputSize(renderer, &fullScreen.w, &fullScreen.h);
    fullScreen.x = 0;
    fullScreen.y = 0;
    SDL_RenderFillRect(renderer, &fullScreen);
  }

  // Render player stats
  if (!isGameOver) {
    renderPlayerStats(renderer);
    renderHealthBars(renderer);
  }

  // Always check if game is over and render the appropriate screen
  if (isGameOver) {
    SDL_Log("Rendering game end screen: %s",
            playerWon ? "Win" : "Game Over"); // Add debug logging
    renderGameEndScreen(renderer);
  }
}
void LevelLast::renderHealthBars(SDL_Renderer *renderer) {
  // Render enemy health bar
  if (enemy) {
    int maxHealth = 400; // Enemy's max health
    int currentHealth = enemy->getHealth();
    float healthPercent = static_cast<float>(currentHealth) / maxHealth;

    // Position the health bar above the enemy
    int barWidth = 100;
    int barHeight = 10;
    int barX = enemy->getX() - barWidth / 2;
    int barY = enemy->getY() - enemy->getRect().h / 2 - 20;

    // Draw background (black)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect bgRect = {barX, barY, barWidth, barHeight};
    SDL_RenderFillRect(renderer, &bgRect);

    // Draw health (red to green based on health percentage)
    int healthWidth = static_cast<int>(barWidth * healthPercent);
    int r = static_cast<int>(255 * (1 - healthPercent));
    int g = static_cast<int>(255 * healthPercent);
    SDL_SetRenderDrawColor(renderer, r, g, 0, 255);
    SDL_Rect healthRect = {barX, barY, healthWidth, barHeight};
    SDL_RenderFillRect(renderer, &healthRect);

    // Draw border (white)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bgRect);

    // Display enemy health number above health bar
    if (statsFont) {
      // Create health text
      std::string enemyHealthText = std::to_string(currentHealth);

      // Set text color (white)
      SDL_Color textColor = {255, 255, 255, 255};

      // Render health text
      SDL_Surface *enemyHealthSurface =
          TTF_RenderText_Solid(statsFont, enemyHealthText.c_str(), textColor);
      if (enemyHealthSurface) {
        SDL_Texture *enemyHealthTexture =
            SDL_CreateTextureFromSurface(renderer, enemyHealthSurface);
        if (enemyHealthTexture) {
          // Position text above the health bar
          SDL_Rect enemyHealthRect = {
              barX + (barWidth - enemyHealthSurface->w) /
                         2,                     // Center text above bar
              barY - enemyHealthSurface->h - 5, // 5 pixels above the health bar
              enemyHealthSurface->w, enemyHealthSurface->h};
          SDL_RenderCopy(renderer, enemyHealthTexture, NULL, &enemyHealthRect);
          SDL_DestroyTexture(enemyHealthTexture);
        }
        SDL_FreeSurface(enemyHealthSurface);
      }
    }
  }

  // Render player health bar
  if (player) {
    int maxHealth = player->getMaxHealth();
    int currentHealth = player->getHealth();
    float healthPercent = static_cast<float>(currentHealth) / maxHealth;

    // Position the health bar at the top of the screen
    int barWidth = 200;
    int barHeight = 15;
    int barX = 20;
    int barY = 80;

    // Draw background (black)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect bgRect = {barX, barY, barWidth, barHeight};
    SDL_RenderFillRect(renderer, &bgRect);

    // Draw health (red to green based on health percentage)
    int healthWidth = static_cast<int>(barWidth * healthPercent);
    int r = static_cast<int>(255 * (1 - healthPercent));
    int g = static_cast<int>(255 * healthPercent);
    SDL_SetRenderDrawColor(renderer, r, g, 0, 255);
    SDL_Rect healthRect = {barX, barY, healthWidth, barHeight};
    SDL_RenderFillRect(renderer, &healthRect);

    // Draw border (white)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
  }
}

void LevelLast::renderPlayerStats(SDL_Renderer *renderer) {
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

void LevelLast::handleEvents(SDL_Event *event, SDL_Renderer *renderer) {
  // Debug check - print health values when P is pressed
  if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_p) {
    if (player) {
      SDL_Log("Player health: %d/%d", player->getHealth(), player->getMaxHealth());
    }
    if (enemy) {
      SDL_Log("Enemy health: %d/400", enemy->getHealth());
    }
    SDL_Log("Game over state: %s", isGameOver ? "true" : "false");
    SDL_Log("Current level: %d", GameState::current_level);
  }

  // Handle game over state inputs
  if (isGameOver) {
    if (event->type == SDL_KEYDOWN) {
      if (playerWon && event->key.keysym.sym == SDLK_c) {
        SDL_Log("Transitioning to credits screen");
        GameState::setCurrentLevel(99);
        GameState::isLoading = true;  // This is the key fix - we need to set isLoading to true
        return;
      }
      else if (event->key.keysym.sym == SDLK_g) {
        SDL_Log("Restarting level after game over");
        restartLevel(renderer);
        return;
      }
    }
    return; // Skip other input processing when game is over
  }

  // Only handle other events if game is not over
  Level::handleEvents(event, renderer);

}
void LevelLast::renderGameEndScreen(SDL_Renderer *renderer) {
  // Get screen dimensions
  int screenWidth, screenHeight;
  SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

  // Create semi-transparent overlay
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
  SDL_Rect overlay = {0, 0, screenWidth, screenHeight};
  SDL_RenderFillRect(renderer, &overlay);

  // Prepare text to display
  std::string mainMessage = playerWon ? "YOU WIN!" : "GAME OVER";
  std::string subMessage = playerWon ? "Press C for Credits"
                                     : // Changed win message
                               "Press G to restart";

  SDL_Log("Rendering end screen with message: %s",
          mainMessage.c_str()); // Add debug logging

  // Set text color
  SDL_Color textColor =
      playerWon ? SDL_Color{255, 215, 0, 255}
                : SDL_Color{255, 0, 0, 255}; // Gold for win, red for game over
  SDL_Color subTextColor = {255, 255, 255, 255}; // White for sub-message

  // Render main message (larger font)
  TTF_Font *largeFont = TTF_OpenFont("assets/fonts/ARCADECLASSIC.ttf", 72);
  if (largeFont) {
    SDL_Surface *messageSurface =
        TTF_RenderText_Solid(largeFont, mainMessage.c_str(), textColor);
    if (messageSurface) {
      SDL_Texture *messageTexture =
          SDL_CreateTextureFromSurface(renderer, messageSurface);
      if (messageTexture) {
        SDL_Rect messageRect = {(screenWidth - messageSurface->w) / 2,
                                (screenHeight - messageSurface->h) / 2 - 50,
                                messageSurface->w, messageSurface->h};
        SDL_RenderCopy(renderer, messageTexture, NULL, &messageRect);
        SDL_DestroyTexture(messageTexture);
      }
      SDL_FreeSurface(messageSurface);
    }
    TTF_CloseFont(largeFont);
  }

  // Render sub-message
  if (statsFont) {
    SDL_Surface *subMessageSurface =
        TTF_RenderText_Solid(statsFont, subMessage.c_str(), subTextColor);
    if (subMessageSurface) {
      SDL_Texture *subMessageTexture =
          SDL_CreateTextureFromSurface(renderer, subMessageSurface);
      if (subMessageTexture) {
        SDL_Rect subMessageRect = {(screenWidth - subMessageSurface->w) / 2,
                                   (screenHeight - subMessageSurface->h) / 2 +
                                       50,
                                   subMessageSurface->w, subMessageSurface->h};
        SDL_RenderCopy(renderer, subMessageTexture, NULL, &subMessageRect);
        SDL_DestroyTexture(subMessageTexture);
      }
      SDL_FreeSurface(subMessageSurface);
    }
  }
}

void LevelLast::restartLevel(SDL_Renderer *renderer) {
  // Reset game state flags
  isGameOver = false;
  playerWon = false;

  // Reload the level
  readLevel("levels/lvl_last.txt", renderer);

  // Reset player and enemy
  if (player) {
    player->takeDamage(-player->getMaxHealth()); // Heal to full health
    SDL_Log("Reset player health to %d", player->getHealth());
    player->shouldShot(true); // Make sure player can shoot
  }

  if (enemy) {
    enemy->takeDamage(-enemy->getMaxHealth()); // Heal to full health
    SDL_Log("Reset enemy health to %d", enemy->getHealth());
    enemy->linkToPlayer(player);
  }

  // Reset other state variables
  timeScale = 1.0f;
  lastFrameTime = SDL_GetTicks();
  enemyMovementTimer = 0;
}
LevelLast::~LevelLast() {
  // Clean up font
  if (statsFont) {
    TTF_CloseFont(statsFont);
    statsFont = nullptr;
  }
}
// Code created by Mouttaki Omar(王明清)