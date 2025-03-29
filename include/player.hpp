#pragma once
#include "CONSTANTS.hpp"
#include "bullet.hpp"
#include "sprite.hpp"
#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <soundmanager.hpp>
#include <GameState.hpp>
#include <list>
#include <map>
#include <vector>


enum PlayerState
{
  IDLE,
  WALKING,
  JUMPING,
  FALLING,
  SPRINT,
  DASHING
};

class Player : public Sprite
{
public:
  Player(SDL_Renderer *renderer, b2World *world, int x, int y);
  ~Player();
  void render(SDL_Renderer *renderer);
  void update();
  void handleEvents(SDL_Event *event, SDL_Renderer *renderer);
  void handleMouseMotion(int x, int y);
  void fireBullet(SDL_Renderer *renderer);
  void updateBullets();
  void renderBullets(SDL_Renderer *renderer);
  void updatePhysics();
  bool isOnGround() const;

  // Debug rendering method
  void renderDebugSpriteBounds(SDL_Renderer *renderer);

  int getWidth() const { return 50; }
  int getHeight() const { return 50; }
  int getHealth() const { return health; }
  int getBullets() const { return bulletsCount; }
  int getMaxHealth() const { return maxHealth; }
  int getMaxBullets() const { return maxBullets; }
  float getVelocityX() const { return body->GetLinearVelocity().x; }
  float getVelocityY() const { return body->GetLinearVelocity().y; }
  b2Body *getBody() const { return body; } // Getter for the Box2D body
  void shouldShot(bool should) { canShot = should; }
  void takeDamage(int damage)
  {
    health -= damage;
    if (health < 0)
      health = 0;
  }
  const std::list<std::unique_ptr<Bullet>> &getBulletsObj() const
  {
    return bullets;
  }
  void removeBullet(Bullet *bullet)
  {
    bullets.remove_if([bullet](const std::unique_ptr<Bullet> &b)
                      { return b.get() == bullet; });
  }


private:
  PlayerState state;
  PlayerState previousState;
  b2Body *body;
  int runSpeed = 30;
  int walkSpeed = 15;
  int jumpForce = 60;          // Increased from 40 to 60 for higher jumps
  int dashForce = 150;         // Force applied during dash
  float dashVelocity = 250.0f; // Direct velocity for dash instead of force
  int dashDuration = 100;      // Increased duration for longer dash
  int currentSpeed = runSpeed;
  bool isJumping = false;
  bool isWalking = false;
  bool isRunning = true;
  bool isDashing = false;
  bool shouldJump = false;
  bool spacePressed = false; // Track if space key is currently pressed
  int walkingDirection = 0;
  float groundCheckDistance = 1.5f; // Increased from 1.1f for more reliable ground detection

  // Ground forgiveness timer allows jumping shortly after leaving the ground
  int groundForgivenessTime = 15; // Increased from 8 to 15 frames (about 250ms at 60fps)
  int groundForgivenessTimer = 0;

  // Dash properties
  int dashCooldown = 60;        // Frames to wait between dashes (60 frames = 1 second at 60 FPS)
  int dashTimer = 0;            // Current dash time
  int dashCooldownTimer = 1000; // Current cooldown time
  bool isFirstDash = true;

  // Animation properties
  struct Animation
  {
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

  // Reload properties
  bool isReloading = false;
  int reloadRate = 60; // Frames between each bullet reload (30 frames = 0.5
                       // seconds at 60 FPS)
  int reloadTimer = 0;
  int maxBullets = 10;

  void loadAnimations(SDL_Renderer *renderer);
  void updateAnimation();
  // Gun properties
  SDL_Texture *gunTexture = nullptr;
  float gunRotation = 0.0f;
  int gunWidth = 32;
  int gunHeight = 16;
  int gunGap = 20; // Gap between player and gun in pixels
  int mouseX = 0, mouseY = 0;
};

const float PPM = 32.0f; // Match the PPM value used elsewhere
void Player::fireBullet(SDL_Renderer *renderer)
{
  if (fireTimer <= 0 && bulletsCount > 0)
  {
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

void Player::updateBullets()
{
  // Update fire timer
  if (fireTimer > 0)
  {
    fireTimer--;
  }

  // Get screen dimensions
  int screenWidth, screenHeight;
  SDL_GetRendererOutputSize(SDL_GetRenderer(SDL_GetWindowFromID(1)),
                            &screenWidth, &screenHeight);

  // Update bullets and remove those that are out of bounds
  auto it = bullets.begin();
  while (it != bullets.end())
  {
    (*it)->update();

    if ((*it)->isOutOfBounds(screenWidth, screenHeight))
    {
      it = bullets.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

void Player::renderBullets(SDL_Renderer *renderer)
{
  for (auto &bullet : bullets)
  {
    bullet->render(renderer);
  }
}

// In the Player constructor, update the collision shape setup

Player::Player(SDL_Renderer *renderer, b2World *world, int x, int y)
{
  const float PPM = 32.0f; // Match Level.hpp's pixels per meter

  state = IDLE;
  previousState = IDLE;
  spacePressed = false;
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;

  // Convert pixel coordinates to Box2D meters
  bodyDef.position.Set((x + W_SPRITESIZE / 2) / PPM,
                       (y + W_SPRITESIZE / 2) / PPM);

  bodyDef.fixedRotation = true;
  body = world->CreateBody(&bodyDef);

  // Match hitbox size exactly to the player's visual size
  float playerWidth = getWidth();   // Use the actual width from getWidth()
  float playerHeight = getHeight(); // Use the actual height from getHeight()

  // Create a box shape for the player's body - make collision box smaller
  float hitboxScale = 0.7f; // Reduce collision box to 70% of visual size
  b2PolygonShape shape;
  shape.SetAsBox((playerWidth / 2 * hitboxScale) / PPM, (playerHeight / 2 * hitboxScale) / PPM);

  // Create a fixture for the player's body
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &shape;
  fixtureDef.density = 1.0f;
  fixtureDef.friction = 0.001f; // Reduced from 0.01f to match icy block friction (0.001f)

  // Add higher wall friction to prevent climbing
  fixtureDef.restitution = 0.05f; // Small bounce to match blocks

  // Add user data to identify player fixture
  fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(this);

  body->CreateFixture(&fixtureDef);

  // Initialize animation properties
  currentFrame = 0;
  frameTimer = 0;
  facingRight = true;

  // Load gun texture
  SDL_Surface *gunSurface =
      Texture::loadFromFile("assets/gun/player.png", renderer, gunTexture);
  if (gunSurface)
  {
    gunWidth = W_SPRITESIZE / 10;
    gunHeight = W_SPRITESIZE / 10;
    SDL_FreeSurface(gunSurface);
  }

  // Load animations
  loadAnimations(renderer);
}

Player::~Player()
{
  // Clean up animation textures
  for (auto &animPair : animations)
  {
    for (auto &texture : animPair.second.frames)
    {
      if (texture)
      {
        SDL_DestroyTexture(texture);
      }
    }
  }

  // Clean up gun texture
  if (gunTexture)
  {
    SDL_DestroyTexture(gunTexture);
  }
}

void Player::loadAnimations(SDL_Renderer *renderer)
{
  // Load idle animation
  Animation idleAnim;
  idleAnim.frameCount = 4;   // Assuming 4 frames for idle
  idleAnim.frameDelay = 150; // 150ms between frames
  for (int i = 0; i < idleAnim.frameCount; i++)
  {
    char path[100];
    sprintf(path, "assets/player/idle/idle_%d.png", i);
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = Texture::loadFromFile(path, renderer, texture);
    if (surface)
    {
      idleAnim.frames.push_back(texture);
      SDL_FreeSurface(surface);
    }
  }
  animations[IDLE] = idleAnim;

  // Load walking animation
  Animation walkAnim;
  walkAnim.frameCount = 6;   // Assuming 6 frames for walking
  walkAnim.frameDelay = 100; // 100ms between frames
  for (int i = 0; i < walkAnim.frameCount; i++)
  {
    char path[100];
    sprintf(path, "assets/player/walk/walk_%d.png", i);
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = Texture::loadFromFile(path, renderer, texture);
    if (surface)
    {
      walkAnim.frames.push_back(texture);
      SDL_FreeSurface(surface);
    }
  }
  animations[WALKING] = walkAnim;

  // Load sprint animation
  Animation sprintAnim;
  sprintAnim.frameCount = 6;  // Assuming 6 frames for sprint
  sprintAnim.frameDelay = 80; // 80ms between frames (faster than walking)
  for (int i = 0; i < sprintAnim.frameCount; i++)
  {
    char path[100];
    sprintf(path, "assets/player/sprint/sprint_%d.png", i);
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = Texture::loadFromFile(path, renderer, texture);
    if (surface)
    {
      sprintAnim.frames.push_back(texture);
      SDL_FreeSurface(surface);
    }
  }
  animations[SPRINT] = sprintAnim;

  // Load jumping animation
  Animation jumpAnim;
  jumpAnim.frameCount = 2;   // Assuming 2 frames for jumping
  jumpAnim.frameDelay = 200; // 200ms between frames
  for (int i = 0; i < jumpAnim.frameCount; i++)
  {
    char path[100];
    sprintf(path, "assets/player/jump/jump_%d.png", i);
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = Texture::loadFromFile(path, renderer, texture);
    if (surface)
    {
      jumpAnim.frames.push_back(texture);
      SDL_FreeSurface(surface);
    }
  }
  animations[JUMPING] = jumpAnim;

  // Load falling animation
  Animation fallAnim;
  fallAnim.frameCount = 2;   // Assuming 2 frames for falling
  fallAnim.frameDelay = 200; // 200ms between frames
  for (int i = 0; i < fallAnim.frameCount; i++)
  {
    char path[100];
    sprintf(path, "assets/player/land/land_%d.png", i);
    SDL_Texture *texture = nullptr;
    SDL_Surface *surface = Texture::loadFromFile(path, renderer, texture);
    if (surface)
    {
      fallAnim.frames.push_back(texture);
      SDL_FreeSurface(surface);
    }
  }
  animations[FALLING] = fallAnim;
}

void Player::updateAnimation()
{
  // Only update animation if not dashing
  if (state != DASHING)
  {
    // Reset animation if state changed
    if (state != previousState)
    {
      currentFrame = 0;
      frameTimer = 0;
      previousState = state;
    }

    // Update animation frame
    frameTimer++;
    if (animations.find(state) != animations.end())
    {
      Animation &currentAnim = animations[state];
      if (frameTimer >= currentAnim.frameDelay)
      {
        frameTimer = 0;
        currentFrame = (currentFrame + 1) % currentAnim.frameCount;
      }
    }
  }

  // Update facing direction based on movement
  if (walkingDirection < 0)
  {
    facingRight = false;
  }
  else if (walkingDirection > 0)
  {
    facingRight = true;
  }
}

bool Player::isOnGround() const
{
  // If we're within the forgiveness time, consider the player on ground
  if (groundForgivenessTimer > 0)
  {
    return true;
  }

  const float PPM = 32.0f;        // Match the PPM value used elsewhere
  const float hitboxScale = 0.7f; // Match the scale used in constructor

  // Create a small ray cast downward from player's position
  b2Vec2 start = body->GetPosition();

  // Calculate the bottom of the player's collision box
  float playerHeight = getHeight() * hitboxScale;
  float playerWidth = getWidth() * hitboxScale;

  // Cast from the bottom of the player's collision box
  start.y += (playerHeight / 2) / PPM;

  // Increase ray length for more reliable ground detection
  float rayLength = 0.5f; // Increased from 0.25f to 0.5f for better detection
  b2Vec2 end = start + b2Vec2(0, rayLength);

  // Calculate width for side checks - use wider spacing for gaps
  float sideOffset1 = (playerWidth / 2) / PPM * 0.7f; // Reduce side check width to prevent wall climbing
  float sideOffset2 = (playerWidth / 4) / PPM * 0.7f; // Reduce side check width

  // Add more ray casts to cover more ground points
  b2Vec2 startLeft1 = start - b2Vec2(sideOffset1, 0);
  b2Vec2 endLeft1 = startLeft1 + b2Vec2(0, rayLength);

  b2Vec2 startLeft2 = start - b2Vec2(sideOffset2, 0);
  b2Vec2 endLeft2 = startLeft2 + b2Vec2(0, rayLength);

  b2Vec2 startRight1 = start + b2Vec2(sideOffset1, 0);
  b2Vec2 endRight1 = startRight1 + b2Vec2(0, rayLength);

  b2Vec2 startRight2 = start + b2Vec2(sideOffset2, 0);
  b2Vec2 endRight2 = startRight2 + b2Vec2(0, rayLength);

  // Use Box2D's ray cast to check for ground
  class GroundRayCastCallback : public b2RayCastCallback
  {
  public:
    bool hit = false;
    b2Body *playerBody;

    float ReportFixture(b2Fixture *fixture, const b2Vec2 &point,
                        const b2Vec2 &normal, float fraction) override
    {
      // Skip player's own fixture
      if (fixture->GetBody() == playerBody)
      {
        return -1; // Continue checking
      }

      // Strict check: Only count hits from directly below (normal pointing up)
      // This prevents detecting walls as ground
      if (normal.y < -0.85f)
      { // Almost vertical normal (was -0.5f)
        hit = true;
        return 0; // Stop checking
      }
      return -1; // Continue checking
    }
  };

  // Check center ray first
  GroundRayCastCallback callback;
  callback.playerBody = body;
  body->GetWorld()->RayCast(&callback, start, end);

  if (callback.hit)
  {
    return true;
  }

  // If center ray didn't hit, try all side rays
  GroundRayCastCallback callbackLeft1;
  callbackLeft1.playerBody = body;
  body->GetWorld()->RayCast(&callbackLeft1, startLeft1, endLeft1);

  if (callbackLeft1.hit)
  {
    return true;
  }

  GroundRayCastCallback callbackLeft2;
  callbackLeft2.playerBody = body;
  body->GetWorld()->RayCast(&callbackLeft2, startLeft2, endLeft2);

  if (callbackLeft2.hit)
  {
    return true;
  }

  GroundRayCastCallback callbackRight1;
  callbackRight1.playerBody = body;
  body->GetWorld()->RayCast(&callbackRight1, startRight1, endRight1);

  if (callbackRight1.hit)
  {
    return true;
  }

  GroundRayCastCallback callbackRight2;
  callbackRight2.playerBody = body;
  body->GetWorld()->RayCast(&callbackRight2, startRight2, endRight2);

  return callbackRight2.hit;
}

void Player::updatePhysics()
{
  b2Vec2 vel = body->GetLinearVelocity();

  // Handle dashing
  if (isDashing)
  {
    if (dashTimer > 0)
    {
      // Apply dash velocity directly instead of force
      int dashDirection = walkingDirection != 0 ? walkingDirection : (facingRight ? 1 : -1);

      // Set a fixed high velocity instead of applying force - convert to m/s
      body->SetLinearVelocity(b2Vec2(dashDirection * dashVelocity / PPM, 0));

      // Disable gravity during dash for more consistent movement
      body->SetGravityScale(0.0f);

      // Store previous state if this is the first frame of dashing
      if (state != DASHING)
      {
        previousState = state;
      }

      state = DASHING;
      dashTimer--;
    }
    else
    {
      // End dash
      isDashing = false;
      dashCooldownTimer = dashCooldown;

      // Re-enable gravity
      body->SetGravityScale(1.0f);

      // Preserve some momentum after dash ends
      b2Vec2 currentVel = body->GetLinearVelocity();
      body->SetLinearVelocity(b2Vec2(currentVel.x * 0.5f, currentVel.y));
    }
  }
  else
  {
    // Normal movement when not dashing

    // Detect wall climbing attempt - when moving horizontally against a wall but not on ground
    if (!isOnGround() && walkingDirection != 0)
    {
      // Check if there's a wall in the direction of movement
      b2Vec2 start = body->GetPosition();
      float playerWidth = getWidth() * 0.7f; // Same hitbox scale as elsewhere
      float horizontalCheckDistance = 0.2f;  // Short distance to check for walls

      // Position ray at center of player, in direction of movement
      b2Vec2 end = start + b2Vec2(walkingDirection * horizontalCheckDistance, 0);

      // Check for collision with wall
      class WallRayCastCallback : public b2RayCastCallback
      {
      public:
        bool hit = false;
        b2Body *playerBody = nullptr;

        float ReportFixture(b2Fixture *fixture, const b2Vec2 &point,
                            const b2Vec2 &normal, float fraction) override
        {
          // Skip player's own fixture
          if (fixture->GetBody() == playerBody)
          {
            return -1;
          }
          hit = true;
          return 0;
        }
      };

      WallRayCastCallback wallCallback;
      wallCallback.playerBody = body;
      body->GetWorld()->RayCast(&wallCallback, start, end);

      // If we're trying to move into a wall while in the air
      if (wallCallback.hit)
      {
        // Apply a slight push away from wall to prevent climbing
        body->ApplyLinearImpulse(
            b2Vec2(-walkingDirection * 0.05f, 0.05f), // Push away from wall with slight downward force
            body->GetWorldCenter(),
            true);
      }
    }

    // Check for ceiling collisions - if the player is moving upward
    if (vel.y < 0)
    {
      // Cast a ray upward to check for ceiling
      b2Vec2 start = body->GetPosition();
      float playerHeight = getHeight() * 0.7f; // Same hitbox scale as elsewhere

      // Cast from top of player's head
      start.y -= (playerHeight / 2) / PPM;

      float ceilingCheckDistance = 0.2f;
      b2Vec2 end = start - b2Vec2(0, ceilingCheckDistance);

      class CeilingRayCastCallback : public b2RayCastCallback
      {
      public:
        bool hit = false;
        b2Body *playerBody = nullptr;

        float ReportFixture(b2Fixture *fixture, const b2Vec2 &point,
                            const b2Vec2 &normal, float fraction) override
        {
          // Skip player's own fixture
          if (fixture->GetBody() == playerBody)
          {
            return -1;
          }
          hit = true;
          return 0;
        }
      };

      CeilingRayCastCallback ceilingCallback;
      ceilingCallback.playerBody = body;
      body->GetWorld()->RayCast(&ceilingCallback, start, end);

      // If we hit a ceiling while moving upward
      if (ceilingCallback.hit)
      {
        // Stop upward velocity and apply a small downward velocity
        if (vel.y < 0)
        {
          // Calculate a bounce-back velocity based on current velocity
          float bounceVelocity = vel.y * -0.2f; // 20% bounce in the opposite direction
          // Ensure minimum bounce
          if (bounceVelocity < 0.5f)
          {
            bounceVelocity = 0.5f;
          }

          body->SetLinearVelocity(b2Vec2(vel.x, bounceVelocity)); // Apply downward velocity with bounce
          isJumping = false;                                      // Stop the jump
          shouldJump = false;                                     // Prevent immediate re-jump
        }
      }
    }

    // Set current speed based on walk/run state, but only when on ground
    if (isOnGround())
    {
      currentSpeed = isRunning ? runSpeed : walkSpeed;
    }
    else
    {
      // Use a slightly higher speed in air for better control
      currentSpeed = isRunning ? (runSpeed * 0.9f) : (walkSpeed * 0.9f);
    }

    // Horizontal movement with smoother acceleration - convert to m/s
    float targetVelocityX = walkingDirection * currentSpeed / PPM;

    // When we want to stop (walkingDirection == 0), adjust deceleration to simulate icy surfaces
    if (walkingDirection == 0 && isOnGround())
    {
      // Apply a very small deceleration on icy surfaces to simulate sliding
      // Just dampen current velocity slightly instead of zeroing it out
      float iceDeceleration = 0.02f; // Very small deceleration for ice
      float newVelX = vel.x * (1.0f - iceDeceleration);

      // Only update if speed is changing significantly
      if (abs(newVelX - vel.x) > 0.001f)
      {
        body->SetLinearVelocity(b2Vec2(newVelX, vel.y));
      }
    }
    // For active movement, apply normal impulse but with reduced deceleration
    else
    {
      // Add a velocity boost to prevent getting stuck at very low speeds
      if (abs(vel.x) < 0.15f && walkingDirection != 0 && isOnGround())
      {
        // Give a stronger boost to overcome edge-catching
        float boostFactor = 0.5f;
        // Only modify the x component, preserving y velocity
        body->SetLinearVelocity(b2Vec2(boostFactor * walkingDirection, vel.y));
      }

      float velocityChange = targetVelocityX - vel.x;
      float impulseX = body->GetMass() * velocityChange;
      body->ApplyLinearImpulse(b2Vec2(impulseX, 0), body->GetWorldCenter(), true);
    }

    // Jumping - only if on ground
    if (isJumping && isOnGround())
    {
      // Apply a strong upward impulse instead of setting velocity directly -
      // convert to m/s
      float jumpImpulse = body->GetMass() * jumpForce / PPM;

      // Cancel any downward velocity first for more consistent jumps
      if (vel.y > 0)
      {
        body->SetLinearVelocity(b2Vec2(vel.x, 0));
      }

      body->ApplyLinearImpulse(b2Vec2(0, -jumpImpulse), body->GetWorldCenter(),
                               true);
      state = JUMPING;
      isJumping = false;
    }

    // Update player state based on physics
    if (!isOnGround())
    {
      if (vel.y < 0)
      {
        state = JUMPING;
      }
      else
      {
        state = FALLING;
      }
    }
    else if (abs(vel.x) > 0.5f)
    {
      // Determine if player is walking intentionally or sliding on ice
      if (abs(walkingDirection) > 0)
      {
        // Normal walking/running when actively moving
        state = (isRunning && isWalking) ? SPRINT : WALKING;
      }
      else if (abs(vel.x) > 0.8f)
      {
        // Fast sliding shows sprint animation
        state = SPRINT;
      }
      else
      {
        // Slow sliding shows walking animation
        state = WALKING;
      }
    }
    else
    {
      state = IDLE;
    }
  }

  // Update dash cooldown timer
  if (dashCooldownTimer > 0)
  {
    dashCooldownTimer--;
  }
}

void Player::update()
{

  const float PPM = 32.0f; // Match the PPM value used elsewhere

  // Store previous velocity for speed bug detection
  b2Vec2 prevVel = body->GetLinearVelocity();

  // Anti-sticking code: detect when player is stuck
  static int stuckCounter = 0;
  static b2Vec2 lastPosition = body->GetPosition();
  b2Vec2 position = body->GetPosition();

  // If player has movement input but position hasn't changed much AND is on ground
  // For icy surfaces, make the check a bit more lenient to avoid false positives
  if (walkingDirection != 0 &&
      abs(position.x - lastPosition.x) < 0.008f && // More lenient for icy surfaces
      abs(prevVel.x) < 0.25f &&
      isOnGround())
  {
    stuckCounter++;

    // If stuck for multiple frames, apply unsticking force
    if (stuckCounter > 3)
    { // Slightly more frames to confirm actual sticking vs. ice physics
      // Apply stronger horizontal impulse in the direction of input
      float unstickImpulse = body->GetMass() * 1.4f; // Slightly reduced for icy surfaces

      // Apply a pure horizontal force with no vertical component
      body->ApplyLinearImpulse(
          b2Vec2(walkingDirection * unstickImpulse, 0.0f),
          body->GetWorldCenter(),
          true);

      // Get current velocity to preserve vertical component
      b2Vec2 currentVel = body->GetLinearVelocity();

      // Reset velocity to ensure we don't have any opposing forces
      // Strictly maintain the current Y velocity to avoid any Y-axis changes
      body->SetLinearVelocity(b2Vec2(walkingDirection * 0.6f, currentVel.y));

      stuckCounter = 0;

      // Log detection of sticking for debugging
      SDL_Log("Unsticking player on ice at position (%f, %f) with velocity (%f, %f)",
              position.x, position.y, currentVel.x, currentVel.y);
    }
  }
  else
  {
    stuckCounter = 0;
  }

  // Save current position for next frame
  lastPosition = position;

  // Check if we're actually on the ground (without using the forgiveness timer)
  bool physicallyOnGround = false;

  {
    // Simple raycast to check ground - using existing implementation
    const float hitboxScale = 0.7f;
    b2Vec2 start = body->GetPosition();
    float playerHeight = getHeight() * hitboxScale;
    start.y += (playerHeight / 2) / PPM;
    float rayLength = 0.5f; // Increased ray length for better detection
    b2Vec2 end = start + b2Vec2(0, rayLength);

    // Use a local ray cast callback for ground detection
    class GroundCallback : public b2RayCastCallback
    {
    public:
      bool hit = false;
      b2Body *playerBody = nullptr;

      float ReportFixture(b2Fixture *fixture, const b2Vec2 &point,
                          const b2Vec2 &normal, float fraction) override
      {
        if (fixture->GetBody() == playerBody)
        {
          return -1; // Skip player's own fixture
        }
        if (normal.y < -0.85f)
        { // Strict ground check to prevent wall climbing
          hit = true;
          return 0;
        }
        return -1;
      }
    };

    GroundCallback callback;
    callback.playerBody = body;
    body->GetWorld()->RayCast(&callback, start, end);

    physicallyOnGround = callback.hit;
  }

  // Handle ground forgiveness timer
  if (physicallyOnGround)
  {
    // Reset forgiveness timer when on ground
    groundForgivenessTimer = groundForgivenessTime;
  }
  else if (groundForgivenessTimer > 0)
  {
    // Decrement timer when not on ground
    groundForgivenessTimer--;
  }

  // Reset jump flag if on ground (which can include forgiveness time)
  if (isOnGround())
  {
    shouldJump = true;

    // Fix speed accumulation bug by directly setting velocity when landing
    if (previousState == FALLING || previousState == JUMPING)
    {
      // Preserve horizontal velocity but reset any accumulated values
      float targetVelocityX =
          walkingDirection * (isRunning ? runSpeed : walkSpeed);
      body->SetLinearVelocity(
          b2Vec2(targetVelocityX / PPM, body->GetLinearVelocity().y));
    }
  }

  // Screen wrapping - teleport player to opposite side when leaving screen
  // boundaries
  position = body->GetPosition();
  bool teleported = false;

  // Get screen dimensions
  int screenWidth, screenHeight;
  SDL_GetRendererOutputSize(SDL_GetRenderer(SDL_GetWindowFromID(1)),
                            &screenWidth, &screenHeight);

  // Check horizontal boundaries - convert Box2D meters to pixels for comparison
  float playerX = position.x * PPM;
  float playerY = position.y * PPM;

  if (playerX < -W_SPRITESIZE)
  {
    position.x = (screenWidth + W_SPRITESIZE / 2) / PPM;
    teleported = true;
  }
  else if (playerX > screenWidth + W_SPRITESIZE)
  {
    position.x = (-W_SPRITESIZE / 2) / PPM;
    teleported = true;
  }

  // Check vertical boundaries
  if (playerY < -W_SPRITESIZE)
  {
    position.y = (screenHeight + W_SPRITESIZE / 2) / PPM;
    teleported = true;
  }
  else if (playerY > screenHeight + W_SPRITESIZE)
  {
    position.y = (-W_SPRITESIZE / 2) / PPM;
    teleported = true;
  }

  // Apply teleportation if needed
  if (teleported)
  {
    body->SetTransform(position, body->GetAngle());
  }

  // Calculate the offset to center the sprite on the physics body
  float playerWidth = W_SPRITESIZE * 0.8f;  // Same as in constructor
  float playerHeight = W_SPRITESIZE * 0.9f; // Same as in constructor

  // Convert Box2D position (meters) to pixel position for rendering
  // Adjust position to center the sprite on the physics body
  int renderX = (int)(position.x * PPM - getWidth() / 2);
  int renderY = (int)(position.y * PPM - getHeight() / 2);

  setPosition(renderX, renderY);

  // Update animation
  updateAnimation();
  PlayerState currentState = state; // Get the state determined by physics

  // --- Walking/Sprinting Sound ---
  bool shouldBeWalking = (currentState == WALKING || currentState == SPRINT);

  if (shouldBeWalking && !GameState::isPlayingWalkingSound)
  {
    // Player STARTING to walk/sprint
    SOUND_MANAGER.setSoundEffectVolume(10);    // Set volume once when starting
    SOUND_MANAGER.playSoundEffect("walk", -1); // Play looped
    GameState::isPlayingWalkingSound = true;

    // Stop dash sound if it was playing (can't walk and dash simultaneously)
    if (GameState::isPlayingDashingSound)
    {
      SOUND_MANAGER.stopSoundEffect("dash");
      GameState::isPlayingDashingSound = false;
    }
  }
  else if (!shouldBeWalking && GameState::isPlayingWalkingSound)
  {
    // Player STOPPING walking/sprinting
    SOUND_MANAGER.stopSoundEffect("walk");
    GameState::isPlayingWalkingSound = false;
  }

  // --- Dashing Sound ---
  bool shouldBeDashing = (currentState == DASHING);

  if (shouldBeDashing && !GameState::isPlayingDashingSound)
  {
    // Player STARTING to dash
    SOUND_MANAGER.setSoundEffectVolume(10);    // Set volume once when starting
    SOUND_MANAGER.playSoundEffect("dash", -1); // Play looped (use -1 for looping)
    GameState::isPlayingDashingSound = true;

    // Stop walking sound if it was playing (can't walk and dash simultaneously)
    if (GameState::isPlayingWalkingSound)
    {
      SOUND_MANAGER.stopSoundEffect("walk");
      GameState::isPlayingWalkingSound = false;
    }
  }
  else if (!shouldBeDashing && GameState::isPlayingDashingSound)
  {
    // Player STOPPING dashing
    SOUND_MANAGER.stopSoundEffect("dash");
    GameState::isPlayingDashingSound = false;
  }

  // --- Reload Sound ---
  // This logic seems intended to play the sound periodically during reload.
  // Let's refine it slightly to play when a bullet is actually added.
  if (isReloading && bulletsCount < maxBullets)
  {
    if (reloadTimer <= 0)
    {
      // Play sound just before the bullet count increments in the next part of this block
      SOUND_MANAGER.setSoundEffectVolume(50);
      SOUND_MANAGER.playSoundEffect("reload"); // Play once

      bulletsCount++;
      reloadTimer = reloadRate; // Reset timer *after* playing sound and adding bullet

      if (bulletsCount >= maxBullets)
      {
        isReloading = false; // Stop reloading process
      }
    }
    else
    {
      reloadTimer--;
    }
  }
  else if (!isReloading && reloadTimer != reloadRate)
  {
    // Ensure timer is reset if reloading stops prematurely (e.g., player action interrupts)
    reloadTimer = 0; // Or keep its value if you want to resume reload timer later? Resetting is safer.
  }
  // Update bullets
  updateBullets();

  // Handle reloading
  if (isReloading && bulletsCount < maxBullets)
  {
    if (reloadTimer <= 0)
    {
      bulletsCount++;
      reloadTimer = reloadRate;

      // Stop reloading if we've reached max bullets
      if (bulletsCount >= maxBullets)
      {
        isReloading = false;
      }
    }
    else
    {
      reloadTimer--;
    }
  }
}

void Player::handleMouseMotion(int x, int y)
{
  mouseX = x;
  mouseY = y;
}

void Player::render(SDL_Renderer *renderer)
{
  // Get current animation frame
  SDL_Texture *currentTexture = nullptr;

  // If dashing, use the current frame from the previous state
  if (state == DASHING)
  {
    // Use the animation from the state before dashing
    if (animations.find(previousState) != animations.end() &&
        !animations[previousState].frames.empty() &&
        currentFrame < animations[previousState].frames.size())
    {
      currentTexture = animations[previousState].frames[currentFrame];
    }
  }
  else
  {
    // Normal animation handling
    if (animations.find(state) != animations.end() &&
        !animations[state].frames.empty() &&
        currentFrame < animations[state].frames.size())
    {
      currentTexture = animations[state].frames[currentFrame];
    }
  }

  if (currentTexture)
  {
    SDL_Rect destRect = {getX(), getY(), getWidth(), getHeight()};

    // Flip texture based on facing direction
    SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    // Normal rendering without brightness effect
    SDL_SetTextureColorMod(currentTexture, 255, 255, 255); // Normal color
    SDL_RenderCopyEx(renderer, currentTexture, nullptr, &destRect, 0, nullptr,
                     flip);
  }
  else
  {
    // Fallback to original rendering if animation frame is not available
    Sprite::render(renderer, getX(), getY());
  }

  renderBullets(renderer);
  // Render gun
  if (gunTexture)
  {
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
    if (angle > 90 || angle < -90)
    {
      gunFlip = SDL_FLIP_VERTICAL;
    }

    // Render gun with rotation
    SDL_RenderCopyEx(renderer, gunTexture, nullptr, &gunRect, angle, nullptr,
                     gunFlip);
  }
}

void Player::handleEvents(SDL_Event *event, SDL_Renderer *renderer)
{
  if (event->type == SDL_KEYDOWN)
  {
    switch (event->key.keysym.sym)
    {
    case SDLK_SPACE:
      // Only trigger jump if space wasn't already pressed
      if (!spacePressed && shouldJump && (isOnGround() || groundForgivenessTimer > 0))
      {
        isJumping = true;
        shouldJump = false;
        // SOUND_MANAGER.setMusicVolume(10); // Likely meant setSoundEffectVolume
        SOUND_MANAGER.setSoundEffectVolume(30); // Example volume for jump
        SOUND_MANAGER.playSoundEffect("jump");  // Play jump sound once
        groundForgivenessTimer = 0;
      }
      spacePressed = true;
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
      if (!isReloading && bulletsCount < maxBullets)
      {
        isReloading = true;
        reloadTimer = reloadRate;
      }
      break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
      // Only change running state if on ground
      if (isOnGround())
      {
        isRunning = false;
      }
      break;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
      if ((dashCooldownTimer == 0 && !isDashing) || isFirstDash)
      {
        // Sound is handled in update() based on state change, no need to play here directly
        isDashing = true;
        dashTimer = dashDuration;
        // state = DASHING; // State is set in updatePhysics based on isDashing flag
        isFirstDash = false;
        b2Vec2 currentVel = body->GetLinearVelocity();
        int dashDirection = walkingDirection != 0 ? walkingDirection : (facingRight ? 1 : -1);
        body->SetLinearVelocity(b2Vec2(dashDirection * dashVelocity / PPM, currentVel.y));
        body->SetGravityScale(0.0f);
      }
      break;
    default:
      break;
    }
  }
  if (event->type == SDL_KEYUP)
  {
    switch (event->key.keysym.sym)
    {
    case SDLK_SPACE:
      spacePressed = false;
      break;
    case SDLK_a:
      if (walkingDirection == -1)
      {
        walkingDirection = 0;
        isWalking = false;
      }
      break;
    case SDLK_d:
      if (walkingDirection == 1)
      {
        walkingDirection = 0;
        isWalking = false;
      }
      break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
      // Only change running state if on ground
      if (isOnGround())
      {
        isRunning = true;
      }
      break;
    default:
      break;
    }
  }
  else if (event->type == SDL_MOUSEMOTION)
  {
    // Update mouse position for gun rotation
    handleMouseMotion(event->motion.x, event->motion.y);
  }
  else if (event->type == SDL_MOUSEBUTTONDOWN)
  {
    if (event->button.button == SDL_BUTTON_LEFT && canShot)
    {
      SOUND_MANAGER.setMusicVolume(10);
      if (bulletsCount > 0)
      {
        SOUND_MANAGER.playSoundEffect("shoot");
      }

      fireBullet(renderer);
    }
  }
}

// Add this to the Player class declaration (public section)
void renderDebugSpriteBounds(SDL_Renderer *renderer);

// Add this implementation after the render method

void Player::renderDebugSpriteBounds(SDL_Renderer *renderer)
{
  const float PPM = 32.0f; // Match the PPM value used elsewhere

  // Set debug draw color (semi-transparent green for sprite bounds)
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 128);

  // Draw rectangle around the sprite's visual dimensions
  SDL_Rect spriteRect = {getX(), getY(), getWidth(), getHeight()};

  // Draw outline
  SDL_RenderDrawRect(renderer, &spriteRect);

  // Draw diagonal lines to make the box more visible
  SDL_RenderDrawLine(renderer, spriteRect.x, spriteRect.y,
                     spriteRect.x + spriteRect.w, spriteRect.y + spriteRect.h);
  SDL_RenderDrawLine(renderer, spriteRect.x + spriteRect.w, spriteRect.y,
                     spriteRect.x, spriteRect.y + spriteRect.h);

  // Also draw the collision hitbox (blue)
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128);
  float hitboxScale = 0.7f; // Same scale as in constructor

  // Calculate collision box dimensions
  int collisionWidth = getWidth() * hitboxScale;
  int collisionHeight = getHeight() * hitboxScale;
  int collisionX = getX() + (getWidth() - collisionWidth) / 2;
  int collisionY = getY() + (getHeight() - collisionHeight) / 2;

  SDL_Rect collisionRect = {collisionX, collisionY, collisionWidth, collisionHeight};
  SDL_RenderDrawRect(renderer, &collisionRect);

  // Draw ground check rays (yellow)
  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 200);

  // Calculate ray positions based on player's position and hitbox
  b2Vec2 position = body->GetPosition();
  float playerHeight = getHeight() * hitboxScale;
  float playerWidth = getWidth() * hitboxScale;

  // Center ray - Start at bottom center of collision box
  int rayCenterStartX = (int)(position.x * PPM);
  int rayCenterStartY = (int)((position.y + playerHeight / 2 / PPM) * PPM);
  int rayEndY = rayCenterStartY + (int)(0.25f * PPM); // 25cm ray (match updated length)

  // Side rays at full and quarter width
  float sideOffset1 = playerWidth / 2;
  float sideOffset2 = playerWidth / 4;

  // Calculate ray start positions
  int rayLeft1StartX = rayCenterStartX - (int)sideOffset1;
  int rayLeft2StartX = rayCenterStartX - (int)sideOffset2;
  int rayRight1StartX = rayCenterStartX + (int)sideOffset1;
  int rayRight2StartX = rayCenterStartX + (int)sideOffset2;

  // Draw the rays
  SDL_RenderDrawLine(renderer, rayCenterStartX, rayCenterStartY, rayCenterStartX, rayEndY);
  SDL_RenderDrawLine(renderer, rayLeft1StartX, rayCenterStartY, rayLeft1StartX, rayEndY);
  SDL_RenderDrawLine(renderer, rayLeft2StartX, rayCenterStartY, rayLeft2StartX, rayEndY);
  SDL_RenderDrawLine(renderer, rayRight1StartX, rayCenterStartY, rayRight1StartX, rayEndY);
  SDL_RenderDrawLine(renderer, rayRight2StartX, rayCenterStartY, rayRight2StartX, rayEndY);

  // Reset blend mode
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}
