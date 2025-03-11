#pragma once
#include "CONSTANTS.hpp"
#include "SDL2/SDL_render.h"
#include "bullet.hpp"
#include "sprite.hpp"
#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <list>
#include <map>
#include <vector>

enum PlayerState { IDLE, WALKING, JUMPING, FALLING, SPRINT, DASHING };

class Player : public Sprite {
public:
  Player(SDL_Renderer *renderer, b2World *world, int x, int y);
  ~Player();
  void handleEvents(SDL_Event *event, SDL_Renderer *renderer);
  void update();
  void updatePhysics();
  void render(SDL_Renderer *renderer);
  bool isOnGround() const;
  int getWidth() const { return W_SPRITESIZE * 1.4; }
  void handleMouseMotion(int mouseX, int mouseY);
  int getHeight() const { return W_SPRITESIZE * 1.4; }
  int getHealth() const { return health; }
  int getBullets() const { return bulletsCount; }
  int getMaxHealth() const { return maxHealth; }
  void shouldShot(bool should) { canShot = should; }
  void fireBullet(SDL_Renderer *renderer);
  void updateBullets();
  void renderBullets(SDL_Renderer *renderer);
  void takeDamage(int damage) {
    health -= damage;
    if (health < 0)
      health = 0;
  }
  const std::list<std::unique_ptr<Bullet>> &getBulletsObj() const {
    return bullets;
  }
  void removeBullet(Bullet *bullet) {
    bullets.remove_if([bullet](const std::unique_ptr<Bullet> &b) {
      return b.get() == bullet;
    });
  }

private:
  PlayerState state;
  PlayerState previousState;
  b2Body *body;
  int runSpeed = 30;
  int walkSpeed = 15;
  int jumpForce = 40;
  int dashForce = 150;         // Force applied during dash
  float dashVelocity = 500.0f; // Direct velocity for dash instead of force
  int dashDuration = 100;      // Increased duration for longer dash
  int currentSpeed = runSpeed;
  bool isJumping = false;
  bool isWalking = false;
  bool isRunning = true;
  bool isDashing = false;
  bool shouldJump = false;
  int walkingDirection = 0;
  float groundCheckDistance = 1.1f; // Distance to check below player for ground

  // Dash properties
  int dashCooldown =
      60; // Frames to wait between dashes (60 frames = 1 second at 60 FPS)
  int dashTimer = 0;            // Current dash time
  int dashCooldownTimer = 1000; // Current cooldown time

  // Animation properties
  struct Animation {
    std::vector<SDL_Texture *> frames;
    int frameCount;
    int frameDelay; // Delay between frames in milliseconds
  };

  std::map<PlayerState, Animation> animations;
  int currentFrame;
  int frameTimer;
  bool facingRight;

  int health = 100;
  int maxHealth = 100;
  int bulletsCount = 10;

  int bulletSpeed = 1;
  int fireRate = 10; // Frames between shots
  int fireTimer = 0; //
  std::list<std::unique_ptr<Bullet>> bullets;

  bool canShot = false;

  void loadAnimations(SDL_Renderer *renderer);
  void updateAnimation();
  // Gun properties
  SDL_Texture *gunTexture = nullptr;
  float gunRotation = 0.0f;
  int gunWidth = 32;
  int gunHeight = 16;
  int gunGap = 20; // Gap between player and gun in pixels
  int mouseX = 0, mouseY = 0;
  // Reload properties
  bool isReloading = false;
  int reloadRate = 60; // Frames between each bullet reload (30 frames = 0.5
                       // seconds at 60 FPS)
  int reloadTimer = 0;
  int maxBullets = 10;
};

void Player::fireBullet(SDL_Renderer *renderer) {
  if (fireTimer <= 0 && bulletsCount > 0) {
    // Calculate bullet spawn position (at gun tip)
    int playerCenterX = getX() + getWidth() / 2;
    int playerCenterY = getY() + getHeight() / 2;

    // Calculate angle between player and mouse
    float dx = mouseX - playerCenterX;
    float dy = mouseY - playerCenterY;
    float angle = atan2(dy, dx) * 180 / M_PI;

    // Calculate gun position with gap
    float gunDistance = gunGap;
    float gunPosX = playerCenterX + cos(angle * M_PI / 180) * gunDistance;
    float gunPosY = playerCenterY + sin(angle * M_PI / 180) * gunDistance;

    // Create bullet at gun position
    bullets.push_back(std::make_unique<Bullet>(renderer, gunPosX, gunPosY,
                                               angle, bulletSpeed));

    // Reset fire timer
    fireTimer = fireRate;

    // Decrease bullet count
    this->bulletsCount--;
  }
}

void Player::updateBullets() {
  // Update fire timer
  if (fireTimer > 0) {
    fireTimer--;
  }

  // Get screen dimensions
  int screenWidth, screenHeight;
  SDL_GetRendererOutputSize(SDL_GetRenderer(SDL_GetWindowFromID(1)),
                            &screenWidth, &screenHeight);

  // Update bullets and remove those that are out of bounds
  auto it = bullets.begin();
  while (it != bullets.end()) {
    (*it)->update();

    if ((*it)->isOutOfBounds(screenWidth, screenHeight)) {
      it = bullets.erase(it);
    } else {
      ++it;
    }
  }
}

void Player::renderBullets(SDL_Renderer *renderer) {
  for (auto &bullet : bullets) {
    bullet->render(renderer);
  }
}

Player::Player(SDL_Renderer *renderer, b2World *world, int x, int y) {
  state = IDLE;
  previousState = IDLE;
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.position.Set(x, y);
  bodyDef.fixedRotation = true;
  body = world->CreateBody(&bodyDef);
  b2PolygonShape shape;
  shape.SetAsBox(W_SPRITESIZE / 2, W_SPRITESIZE / 2);
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &shape;
  fixtureDef.density = 1.0f;
  fixtureDef.friction = 0.3f;
  body->CreateFixture(&fixtureDef);

  // Initialize animation properties
  currentFrame = 0;
  frameTimer = 0;
  facingRight = true;

  // Load gun texture
  SDL_Surface *gunSurface =
      Texture::loadFromFile("assets/gun/player.png", renderer, gunTexture);
  if (gunSurface) {
    gunWidth = W_SPRITESIZE / 10;
    gunHeight = W_SPRITESIZE / 10;
    SDL_FreeSurface(gunSurface);
  }

  // Load animations
  loadAnimations(renderer);
}

Player::~Player() {
  // Clean up animation textures
  for (auto &animPair : animations) {
    for (auto &texture : animPair.second.frames) {
      if (texture) {
        SDL_DestroyTexture(texture);
      }
    }
  }

  // Clean up gun texture
  if (gunTexture) {
    SDL_DestroyTexture(gunTexture);
  }
}

void Player::loadAnimations(SDL_Renderer *renderer) {
  // Load idle animation
  Animation idleAnim;
  idleAnim.frameCount = 4;   // Assuming 4 frames for idle
  idleAnim.frameDelay = 150; // 150ms between frames
  for (int i = 0; i < idleAnim.frameCount; i++) {
    char path[100];
    sprintf(path, "assets/player/idle/idle_%d.png", i);
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = Texture::loadFromFile(path, renderer, texture);
    if (surface) {
      idleAnim.frames.push_back(texture);
      SDL_FreeSurface(surface);
    }
  }
  animations[IDLE] = idleAnim;

  // Load walking animation
  Animation walkAnim;
  walkAnim.frameCount = 6;   // Assuming 6 frames for walking
  walkAnim.frameDelay = 100; // 100ms between frames
  for (int i = 0; i < walkAnim.frameCount; i++) {
    char path[100];
    sprintf(path, "assets/player/walk/walk_%d.png", i);
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = Texture::loadFromFile(path, renderer, texture);
    if (surface) {
      walkAnim.frames.push_back(texture);
      SDL_FreeSurface(surface);
    }
  }
  animations[WALKING] = walkAnim;

  // Load sprint animation
  Animation sprintAnim;
  sprintAnim.frameCount = 6;  // Assuming 6 frames for sprint
  sprintAnim.frameDelay = 80; // 80ms between frames (faster than walking)
  for (int i = 0; i < sprintAnim.frameCount; i++) {
    char path[100];
    sprintf(path, "assets/player/sprint/sprint_%d.png", i);
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = Texture::loadFromFile(path, renderer, texture);
    if (surface) {
      sprintAnim.frames.push_back(texture);
      SDL_FreeSurface(surface);
    }
  }
  animations[SPRINT] = sprintAnim;

  // Load jumping animation
  Animation jumpAnim;
  jumpAnim.frameCount = 2;   // Assuming 2 frames for jumping
  jumpAnim.frameDelay = 200; // 200ms between frames
  for (int i = 0; i < jumpAnim.frameCount; i++) {
    char path[100];
    sprintf(path, "assets/player/jump/jump_%d.png", i);
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = Texture::loadFromFile(path, renderer, texture);
    if (surface) {
      jumpAnim.frames.push_back(texture);
      SDL_FreeSurface(surface);
    }
  }
  animations[JUMPING] = jumpAnim;

  // Load falling animation
  Animation fallAnim;
  fallAnim.frameCount = 2;   // Assuming 2 frames for falling
  fallAnim.frameDelay = 200; // 200ms between frames
  for (int i = 0; i < fallAnim.frameCount; i++) {
    char path[100];
    sprintf(path, "assets/player/land/land_%d.png", i);
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = Texture::loadFromFile(path, renderer, texture);
    if (surface) {
      fallAnim.frames.push_back(texture);
      SDL_FreeSurface(surface);
    }
  }
  animations[FALLING] = fallAnim;

  // Load dashing animation
  Animation dashAnim;
  dashAnim.frameCount = 2;   // Assuming 2 frames for dashing
  dashAnim.frameDelay = 100; // 100ms between frames
  for (int i = 0; i < dashAnim.frameCount; i++) {
    char path[100];
    sprintf(path, "assets/player/dash/dash_%d.png", i);
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = Texture::loadFromFile(path, renderer, texture);
    if (surface) {
      dashAnim.frames.push_back(texture);
      SDL_FreeSurface(surface);
    }
  }
  animations[DASHING] = dashAnim;
}

void Player::updateAnimation() {
  // Only update animation if not dashing
  if (state != DASHING) {
    // Reset animation if state changed
    if (state != previousState) {
      currentFrame = 0;
      frameTimer = 0;
      previousState = state;
    }

    // Update animation frame
    frameTimer++;
    if (animations.find(state) != animations.end()) {
      Animation &currentAnim = animations[state];
      if (frameTimer >= currentAnim.frameDelay) {
        frameTimer = 0;
        currentFrame = (currentFrame + 1) % currentAnim.frameCount;
      }
    }
  }

  // Update facing direction based on movement
  if (walkingDirection < 0) {
    facingRight = false;
  } else if (walkingDirection > 0) {
    facingRight = true;
  }
}

bool Player::isOnGround() const {
  // Create a small ray cast downward from player's position
  b2Vec2 start = body->GetPosition();
  // Cast from the bottom of the player's collision box
  start.y += (W_SPRITESIZE / 2) - 0.1f;
  b2Vec2 end = start + b2Vec2(0, groundCheckDistance);

  // Use Box2D's ray cast to check for ground
  class GroundRayCastCallback : public b2RayCastCallback {
  public:
    bool hit = false;
    b2Body *playerBody;

    float ReportFixture(b2Fixture *fixture, const b2Vec2 &point,
                        const b2Vec2 &normal, float fraction) override {
      // Skip player's own fixture
      if (fixture->GetBody() == playerBody) {
        return -1; // Continue checking
      }
      hit = true;
      return 0; // Stop checking
    }
  };

  GroundRayCastCallback callback;
  callback.playerBody = body; // Store player body to skip self-collision
  body->GetWorld()->RayCast(&callback, start, end);

  return callback.hit;
}

void Player::updatePhysics() {
  b2Vec2 vel = body->GetLinearVelocity();

  // Handle dashing
  if (isDashing) {
    if (dashTimer > 0) {
      // Apply dash velocity directly instead of force
      int dashDirection = facingRight ? 1 : -1;

      // Set a fixed high velocity instead of applying force
      body->SetLinearVelocity(b2Vec2(dashDirection * dashVelocity, 0));

      // Disable gravity during dash for more consistent movement
      body->SetGravityScale(0.0f);

      // Store previous state if this is the first frame of dashing
      if (state != DASHING) {
        previousState = state;
      }

      state = DASHING;
      dashTimer--;
    } else {
      // End dash
      isDashing = false;
      dashCooldownTimer = dashCooldown;

      // Re-enable gravity
      body->SetGravityScale(1.0f);

      // Preserve some momentum after dash ends
      b2Vec2 currentVel = body->GetLinearVelocity();
      body->SetLinearVelocity(b2Vec2(currentVel.x * 0.5f, currentVel.y));
    }
  } else {
    // Normal movement when not dashing

    // Set current speed based on walk/run state, but only when on ground
    if (isOnGround()) {
      currentSpeed = isRunning ? runSpeed : walkSpeed;
    }

    // Horizontal movement with smoother acceleration
    float targetVelocityX = walkingDirection * currentSpeed;
    float velocityChange = targetVelocityX - vel.x;
    float impulseX = body->GetMass() * velocityChange;
    body->ApplyLinearImpulse(b2Vec2(impulseX, 0), body->GetWorldCenter(), true);

    // Jumping - only if on ground
    if (isJumping && isOnGround()) {
      // Apply a strong upward impulse instead of setting velocity directly
      float jumpImpulse = body->GetMass() * jumpForce;
      body->ApplyLinearImpulse(b2Vec2(0, -jumpImpulse), body->GetWorldCenter(),
                               true);
      state = JUMPING;
      isJumping = false;
    }

    // Update player state based on physics
    if (!isOnGround()) {
      if (vel.y < 0) {
        state = JUMPING;
      } else {
        state = FALLING;
      }
    } else if (abs(vel.x) > 0.5f) {
      // Set state to SPRINT if running, WALKING if walking
      state = (isRunning && isWalking) ? SPRINT : WALKING;
    } else {
      state = IDLE;
    }
  }

  // Update dash cooldown timer
  if (dashCooldownTimer > 0) {
    dashCooldownTimer--;
  }
}

void Player::update() {
  // Store previous velocity for speed bug detection
  b2Vec2 prevVel = body->GetLinearVelocity();

  // Reset jump flag if on ground
  if (isOnGround()) {
    shouldJump = true;

    // Fix speed accumulation bug by directly setting velocity when landing
    if (previousState == FALLING || previousState == JUMPING) {
      // Preserve horizontal velocity but reset any accumulated values
      float targetVelocityX =
          walkingDirection * (isRunning ? runSpeed : walkSpeed);
      body->SetLinearVelocity(
          b2Vec2(targetVelocityX, body->GetLinearVelocity().y));
    }
  }

  // Screen wrapping - teleport player to opposite side when leaving screen
  // boundaries
  b2Vec2 position = body->GetPosition();
  bool teleported = false;

  // Get screen dimensions
  int screenWidth, screenHeight;
  SDL_GetRendererOutputSize(SDL_GetRenderer(SDL_GetWindowFromID(1)),
                            &screenWidth, &screenHeight);

  // Check horizontal boundaries
  if (position.x < -W_SPRITESIZE) {
    position.x = screenWidth + W_SPRITESIZE / 2;
    teleported = true;
  } else if (position.x > screenWidth + W_SPRITESIZE) {
    position.x = -W_SPRITESIZE / 2;
    teleported = true;
  }

  // Check vertical boundaries
  if (position.y < -W_SPRITESIZE) {
    position.y = screenHeight + W_SPRITESIZE / 2;
    teleported = true;
  } else if (position.y > screenHeight + W_SPRITESIZE) {
    position.y = -W_SPRITESIZE / 2;
    teleported = true;
  }

  // Apply teleportation if needed
  if (teleported) {
    body->SetTransform(position, body->GetAngle());
  }

  setPosition((int)body->GetPosition().x, (int)body->GetPosition().y);

  // Update animation
  updateAnimation();

  // Update bullets
  updateBullets();

  // Handle reloading
  if (isReloading && bulletsCount < maxBullets) {
    if (reloadTimer <= 0) {
      bulletsCount++;
      reloadTimer = reloadRate;

      // Stop reloading if we've reached max bullets
      if (bulletsCount >= maxBullets) {
        isReloading = false;
      }
    } else {
      reloadTimer--;
    }
  }
}

void Player::handleMouseMotion(int x, int y) {
  mouseX = x;
  mouseY = y;
}

void Player::render(SDL_Renderer *renderer) {
  // Get current animation frame
  SDL_Texture *currentTexture = nullptr;

  // If dashing, use the current frame from the previous state
  if (state == DASHING) {
    // Use the animation from the state before dashing
    if (animations.find(previousState) != animations.end() &&
        !animations[previousState].frames.empty() &&
        currentFrame < animations[previousState].frames.size()) {
      currentTexture = animations[previousState].frames[currentFrame];
    }
  } else {
    // Normal animation handling
    if (animations.find(state) != animations.end() &&
        !animations[state].frames.empty() &&
        currentFrame < animations[state].frames.size()) {
      currentTexture = animations[state].frames[currentFrame];
    }
  }

  if (currentTexture) {
    SDL_Rect destRect = {getX(), getY(), getWidth(), getHeight()};

    // Flip texture based on facing direction
    SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    // Normal rendering without brightness effect
    SDL_SetTextureColorMod(currentTexture, 255, 255, 255); // Normal color
    SDL_RenderCopyEx(renderer, currentTexture, nullptr, &destRect, 0, nullptr,
                     flip);
  } else {
    // Fallback to original rendering if animation frame is not available
    Sprite::render(renderer, getX(), getY());
  }

  renderBullets(renderer);
  // Render gun
  if (gunTexture) {
    // Calculate center of player
    int playerCenterX = getX() + getWidth() / 2;
    int playerCenterY = getY() + getHeight() / 2;

    // Calculate angle between player and mouse
    float dx = mouseX - playerCenterX;
    float dy = mouseY - playerCenterY;
    float angle = atan2(dy, dx) * 180 / M_PI; // Convert to degrees

    // Calculate gun position with gap
    float gunDistance = gunGap;
    float gunPosX = playerCenterX + cos(angle * M_PI / 180) * gunDistance;
    float gunPosY = playerCenterY + sin(angle * M_PI / 180) * gunDistance;

    // Create destination rectangle for gun
    SDL_Rect gunRect = {static_cast<int>(gunPosX - gunWidth / 2),
                        static_cast<int>(gunPosY - gunHeight / 2), gunWidth,
                        gunHeight};

    // Determine if gun should be flipped
    SDL_RendererFlip gunFlip = SDL_FLIP_NONE;
    if (angle > 90 || angle < -90) {
      gunFlip = SDL_FLIP_VERTICAL;
    }

    // Render gun with rotation
    SDL_RenderCopyEx(renderer, gunTexture, nullptr, &gunRect, angle, nullptr,
                     gunFlip);
  }
}

void Player::handleEvents(SDL_Event *event, SDL_Renderer *renderer) {
  if (event->type == SDL_KEYDOWN) {
    switch (event->key.keysym.sym) {
    case SDLK_SPACE:
      if (shouldJump && isOnGround()) {
        isJumping = true;
        shouldJump = false;
      }
      break;
    case SDLK_a:
      walkingDirection = -1;
      isWalking = true;
      break;
    case SDLK_d:
      walkingDirection = 1;
      isWalking = true;
      break;
    case SDLK_r:
      // Start reloading if not already reloading and not at max bullets
      if (!isReloading && bulletsCount < maxBullets) {
        isReloading = true;
        reloadTimer = reloadRate;
      }
      break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
      // Only change running state if on ground
      if (isOnGround()) {
        isRunning = false;
      }
      break;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
      // Initiate dash if not on cooldown and not already dashing
      if (dashCooldownTimer == 0 && !isDashing) {
        isDashing = true;
        dashTimer = dashDuration;
      }
      break;
    default:
      break;
    }
  }
  if (event->type == SDL_KEYUP) {
    switch (event->key.keysym.sym) {
    case SDLK_a:
      if (walkingDirection == -1) {
        walkingDirection = 0;
        isWalking = false;
      }
      break;
    case SDLK_d:
      if (walkingDirection == 1) {
        walkingDirection = 0;
        isWalking = false;
      }
      break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
      // Only change running state if on ground
      if (isOnGround()) {
        isRunning = true;
      }
      break;
    default:
      break;
    }
  } else if (event->type == SDL_MOUSEMOTION) {
    // Update mouse position for gun rotation
    handleMouseMotion(event->motion.x, event->motion.y);
  } else if (event->type == SDL_MOUSEBUTTONDOWN) {
    if (event->button.button == SDL_BUTTON_LEFT && canShot) {
      fireBullet(renderer);
    }
  }
}