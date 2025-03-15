#pragma once
#include "bullet.hpp"
#include "player.hpp"
#include "sprite.hpp"
#include <SDL2/SDL.h>
#include <list>
#include <random>

class Enemy : public Sprite {
protected:
  Player *targetPlayer;
  std::list<std::unique_ptr<Bullet>> bullets;
  int health = 20;   // Increased health to 400
  int fireRate = 120; // Frames between shots
  int fireTimer = 0;
  int bulletSpeed = 2;
  std::random_device rd;
  std::mt19937 gen;
  std::uniform_real_distribution<> dodgeChance;
  std::uniform_real_distribution<> hitChance; // Added for bullet hit chance
  float minY = 400;       // Minimum Y position to prevent touching ground (from top)
  float maxY = 800;       // Maximum Y position (near bottom of screen)
  int maxHealth = 400;    // Added to track max health
  int baseFireRate = 400; // Slower initial fire rate
  
  // Flight pattern variables
  float flightTimer = 0;
  float flightSpeed = 0.01f;  // Reduced from 0.1f to make movement slower
  int flightPattern = 0;
  float flightAngle = 0;
  float targetX = 0, targetY = 0;
  bool patternInitialized = false;  // Track if pattern has been initialized

public:
  Enemy(SDL_Renderer *renderer);
  void linkToPlayer(Player *player) { targetPlayer = player; }
  ~Enemy();

  void update();
  void render(SDL_Renderer *renderer);
  void fireBullet(SDL_Renderer *renderer);
  void updateBullets();
  void takeDamage(int damage);
  bool checkBulletCollision(float bulletX, float bulletY);
  bool shouldDodge() { return dodgeChance(gen) < 0.75; } // 75% chance to dodge
  int getHealth() const { return health; }
  void updateFireRate();
  void dodgeBullets();
  // Add method to get max health
  int getMaxHealth() const { return maxHealth; }

private:
  void updateFlightPattern(); // New method to handle flight patterns
};

Enemy::Enemy(SDL_Renderer *renderer)
    : gen(rd()), dodgeChance(0.0, 1.0), hitChance(0.0, 1.0) {
  loadFromFile("assets/enemy/enemy.png", renderer);
  setPosition(0, 0);
}
void Enemy::update() {
  if (targetPlayer) {
    updateBullets();
    dodgeBullets();
    updateFireRate(); // Update fire rate based on current health
    
    // Only use flight patterns when health is below 50%
    float healthPercent = static_cast<float>(health) / maxHealth;
    if (healthPercent < 0.5f) {
      updateFlightPattern(); // Update flight pattern only when health is low
    }

    // Shoot at player periodically
    if (fireTimer <= 0) {
      fireBullet(SDL_GetRenderer(SDL_GetWindowFromID(1)));
      fireTimer = fireRate;
    } else {
      fireTimer--;
    }
  }
}
void Enemy::render(SDL_Renderer *renderer) {
  Sprite::render(renderer, getX(),
                 getY()); // Changed to pass x and y coordinates
  // Render bullets
  for (auto &bullet : bullets) {
    bullet->render(renderer);
  }
}
void Enemy::fireBullet(SDL_Renderer *renderer) {
  if (!targetPlayer)
    return;

  float dx = targetPlayer->getX() - getX();
  float dy = targetPlayer->getY() - getY();
  float angle = atan2(dy, dx) * 180 / M_PI;

  bullets.push_back(
      std::make_unique<Bullet>(renderer, getX(), getY(), angle, bulletSpeed));
}

void Enemy::updateBullets() {
  int screenWidth, screenHeight;
  SDL_GetRendererOutputSize(SDL_GetRenderer(SDL_GetWindowFromID(1)),
                            &screenWidth, &screenHeight);

  auto it = bullets.begin();
  while (it != bullets.end()) {
    (*it)->update();

    // Check collision with player
    if (targetPlayer &&
        (*it)->isColliding(targetPlayer->getX(), targetPlayer->getY(),
                           targetPlayer->getWidth(),
                           targetPlayer->getHeight())) {
      // Only 25% chance to actually hit the player
      if (hitChance(gen) < 0.25) {
        targetPlayer->takeDamage(10);
      }
      // Always remove the bullet on collision, regardless of damage
      it = bullets.erase(it);
    } else if ((*it)->isOutOfBounds(screenWidth, screenHeight)) {
      it = bullets.erase(it);
    } else {
      ++it;
    }
  }
}

void Enemy::dodgeBullets() {
  if (!targetPlayer)
    return;

  // Get screen dimensions for boundary checking
  int screenWidth, screenHeight;
  SDL_GetRendererOutputSize(SDL_GetRenderer(SDL_GetWindowFromID(1)),
                          &screenWidth, &screenHeight);
  float minXFromEdge = 100; // Stay 100 pixels from left/right edges

  // Check player bullets and dodge if necessary
  for (const auto &bullet : targetPlayer->getBulletsObj()) {
    if (shouldDodge()) {
      float dx = bullet->getX() - getX();
      float dy = bullet->getY() - getY();
      float distance = sqrt(dx * dx + dy * dy);

      if (distance < minY) { // Only dodge nearby bullets
        // Move perpendicular to bullet direction
        float moveX = -dy / distance * 5;
        float moveY = dx / distance * 5;

        // Calculate new position
        float newX = getX() + moveX;
        float newY = getY() + moveY;

        // Ensure enemy stays within screen bounds
        newX = std::max(minXFromEdge, std::min(screenWidth - minXFromEdge, newX));
        newY = std::max(minY, std::min(maxY, newY));

        // Set the new position
        setPosition(newX, newY);
      }
    }
  }
}

void Enemy::takeDamage(int damage) {
  health -= damage;
  if (health < 0)
    health = 0;

  // Update fire rate after taking damage
  updateFireRate();
}

bool Enemy::checkBulletCollision(float bulletX, float bulletY) {
  return (bulletX >= getX() && bulletX <= getX() + getRect().w &&
          bulletY >= getY() && bulletY <= getY() + getRect().h);
}

void Enemy::updateFireRate() {
  // Calculate health percentage
  float healthPercent = static_cast<float>(health) / maxHealth;

  // Adjust fire rate: faster as health decreases
  // At full health: baseFireRate (120)
  // At 0 health: minimum of 30 (4x faster)
  fireRate = baseFireRate * (0.25f + (0.75f * healthPercent));

  // Ensure fire rate doesn't go below minimum
  if (fireRate < 30)
    fireRate = 30;
}
void Enemy::updateFlightPattern() {
  // Get screen dimensions
  int screenWidth, screenHeight;
  SDL_GetRendererOutputSize(SDL_GetRenderer(SDL_GetWindowFromID(1)),
                            &screenWidth, &screenHeight);
                            
  // Update maxY based on screen height
  maxY = screenHeight - 200; // Stay 200 pixels from bottom
  
  // Define minimum X distance from screen edges
  float minXFromEdge = 100; // Stay 100 pixels from left/right edges

  // Increment timer
  flightTimer += 0.5f;  // Slower timer increment for more gradual movement

  // Change flight pattern periodically
  if (flightTimer > 300) { // Change pattern less frequently (5 seconds)
    flightTimer = 0;
    flightPattern = rand() % 3; // Reduced to 3 more predictable patterns
    patternInitialized = false; // Reset initialization flag

    // Adjust flight speed based on health, but keep it slower overall
    float healthPercent = static_cast<float>(health) / maxHealth;
    flightSpeed = 1.0f + (2.0f * (1.0f - healthPercent)); // Slower speeds

    // Set random target position for some patterns
    targetX = minXFromEdge + rand() % (screenWidth - 2 * static_cast<int>(minXFromEdge));
    // Keep Y within valid range (remember Y is inverted)
    targetY = minY + rand() % (static_cast<int>(maxY - minY));
  }

  float currentX = getX();
  float currentY = getY();

  // Apply the current flight pattern
  switch (flightPattern) {
  case 0: // Horizontal patrol
    {
      // Initialize pattern if needed
      if (!patternInitialized) {
        targetX = minXFromEdge;
        patternInitialized = true;
      }
      
      // Move horizontally back and forth
      if (currentX < minXFromEdge + 10) {
        targetX = screenWidth - minXFromEdge;
      } else if (currentX > screenWidth - minXFromEdge - 10) {
        targetX = minXFromEdge;
      }
      
      // Move towards target
      float dx = targetX - currentX;
      float dist = fabs(dx);
      
      if (dist > 5.0f) {
        setPosition(currentX + (dx > 0 ? 1 : -1) * flightSpeed * 2, currentY);
      }
    }
    break;

  case 1: // Simple circle pattern
    {
      float radius = 150.0f;
      flightAngle += 0.01f; // Slower rotation
      
      // Ensure circle stays within screen boundaries
      float centerX = screenWidth / 2;
      float centerY = screenHeight / 3;
      
      // Adjust radius if needed to stay within boundaries
      float maxRadius = std::min(centerX - minXFromEdge, centerY - minY);
      if (radius > maxRadius) radius = maxRadius;
      
      float newX = centerX + cos(flightAngle) * radius;
      float newY = centerY + sin(flightAngle) * radius;
      
      // Move gradually toward the circle position
      float dx = newX - currentX;
      float dy = newY - currentY;
      float dist = sqrt(dx * dx + dy * dy);
      
      if (dist > 5.0f) {
        // Ensure Y stays within bounds (remember Y is inverted)
        float nextY = currentY + (dy/dist) * flightSpeed;
        nextY = std::max(minY, std::min(maxY, nextY));
        
        // Ensure X stays within bounds
        float nextX = currentX + (dx/dist) * flightSpeed;
        nextX = std::max(minXFromEdge, std::min(screenWidth - minXFromEdge, nextX));
        
        setPosition(nextX, nextY);
      }
    }
    break;

  case 2: // Gentle approach and retreat
    if (targetPlayer) {
      // Calculate distance to player
      float dx = targetPlayer->getX() - currentX;
      float dy = targetPlayer->getY() - currentY;
      float dist = sqrt(dx * dx + dy * dy);
      
      // If too close, move away
      if (dist < 200) {
        float nextY = currentY - (dy/dist) * flightSpeed;
        nextY = std::max(minY, std::min(maxY, nextY));
        
        float nextX = currentX - (dx/dist) * flightSpeed;
        nextX = std::max(minXFromEdge, std::min(screenWidth - minXFromEdge, nextX));
        
        setPosition(nextX, nextY);
      } 
      // If too far, move closer
      else if (dist > 400) {
        float nextY = currentY + (dy/dist) * flightSpeed;
        nextY = std::max(minY, std::min(maxY, nextY));
        
        float nextX = currentX + (dx/dist) * flightSpeed;
        nextX = std::max(minXFromEdge, std::min(screenWidth - minXFromEdge, nextX));
        
        setPosition(nextX, nextY);
      }
    }
    break;
  }

  // Ensure enemy stays within screen bounds
  float newX = getX();
  float newY = getY();

  if (newX < minXFromEdge)
    newX = minXFromEdge;
  if (newX > screenWidth - minXFromEdge)
    newX = screenWidth - minXFromEdge;
  
  // Fix Y bounds (remember Y is inverted)
  if (newY < minY)
    newY = minY;
  if (newY > maxY)
    newY = maxY;

  setPosition(newX, newY);
}
Enemy::~Enemy() { bullets.clear(); }

// Code created by Mouttaki Omar(王明清)