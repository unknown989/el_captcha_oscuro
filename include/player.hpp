#pragma once
#include "sprite.hpp"
#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <map>
#include <vector>

enum PlayerState {
    IDLE,
    WALKING,
    JUMPING,
    FALLING,
    SPRINT,
    DASHING
};

class Player : public Sprite {
  public:
    Player(SDL_Renderer* renderer, b2World* world, int x, int y);
    ~Player();
    void handleEvents(SDL_Event* event);
    void update();
    void updatePhysics();
    void render(SDL_Renderer* renderer);
    bool isOnGround() const;
    int getWidth() const { return W_SPRITESIZE*1.4; }
    int getHeight() const { return W_SPRITESIZE*1.4; }

  private:
    PlayerState state;
    PlayerState previousState;
    b2Body* body;
    int runSpeed = 30;
    int walkSpeed = 15;
    int jumpForce = 40;
    int dashForce = 150;  // Force applied during dash
    float dashVelocity = 500.0f; // Direct velocity for dash instead of force
    int dashDuration = 100;     // Increased duration for longer dash
    int currentSpeed = runSpeed;
    bool isJumping = false;
    bool isWalking = false;
    bool isRunning = true;
    bool isDashing = false;
    bool shouldJump = false;
    int walkingDirection = 0;
    float groundCheckDistance = 1.1f; // Distance to check below player for ground
    
    // Dash properties
    int dashCooldown = 60;     // Frames to wait between dashes (60 frames = 1 second at 60 FPS)
    int dashTimer = 0;         // Current dash time
    int dashCooldownTimer = 1000; // Current cooldown time
    
    // Animation properties
    struct Animation {
        std::vector<SDL_Texture*> frames;
        int frameCount;
        int frameDelay;  // Delay between frames in milliseconds
    };
    
    std::map<PlayerState, Animation> animations;
    int currentFrame;
    int frameTimer;
    bool facingRight;
    
    void loadAnimations(SDL_Renderer* renderer);
    void updateAnimation();
};

Player::Player(SDL_Renderer* renderer, b2World* world, int x, int y) {
    loadFromFile("assets/blocks/player.png", renderer);
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
    
    // Load animations
    loadAnimations(renderer);
}

Player::~Player() {
    // Clean up animation textures
    for (auto& animPair : animations) {
        for (auto& texture : animPair.second.frames) {
            if (texture) {
                SDL_DestroyTexture(texture);
            }
        }
    }
}

void Player::loadAnimations(SDL_Renderer* renderer) {
    // Load idle animation
    Animation idleAnim;
    idleAnim.frameCount = 4;  // Assuming 4 frames for idle
    idleAnim.frameDelay = 150;  // 150ms between frames
    for (int i = 0; i < idleAnim.frameCount; i++) {
        char path[100];
        sprintf(path, "assets/player/idle/idle_%d.png", i);
        SDL_Texture* texture = nullptr;
        SDL_Surface* surface = Texture::loadFromFile(path, renderer, texture);
        if (surface) {
            idleAnim.frames.push_back(texture);
            SDL_FreeSurface(surface);
        }
    }
    animations[IDLE] = idleAnim;
    
    // Load walking animation
    Animation walkAnim;
    walkAnim.frameCount = 6;  // Assuming 6 frames for walking
    walkAnim.frameDelay = 100;  // 100ms between frames
    for (int i = 0; i < walkAnim.frameCount; i++) {
        char path[100];
        sprintf(path, "assets/player/walk/walk_%d.png", i);
        SDL_Texture* texture = nullptr;
        SDL_Surface* surface = Texture::loadFromFile(path, renderer, texture);
        if (surface) {
            walkAnim.frames.push_back(texture);
            SDL_FreeSurface(surface);
        }
    }
    animations[WALKING] = walkAnim;
    
    // Load sprint animation
    Animation sprintAnim;
    sprintAnim.frameCount = 6;  // Assuming 6 frames for sprint
    sprintAnim.frameDelay = 80;  // 80ms between frames (faster than walking)
    for (int i = 0; i < sprintAnim.frameCount; i++) {
        char path[100];
        sprintf(path, "assets/player/sprint/sprint_%d.png", i);
        SDL_Texture* texture = nullptr;
        SDL_Surface* surface = Texture::loadFromFile(path, renderer, texture);
        if (surface) {
            sprintAnim.frames.push_back(texture);
            SDL_FreeSurface(surface);
        }
    }
    animations[SPRINT] = sprintAnim;
    
    // Load jumping animation
    Animation jumpAnim;
    jumpAnim.frameCount = 2;  // Assuming 2 frames for jumping
    jumpAnim.frameDelay = 200;  // 200ms between frames
    for (int i = 0; i < jumpAnim.frameCount; i++) {
        char path[100];
        sprintf(path, "assets/player/jump/jump_%d.png", i);
        SDL_Texture* texture = nullptr;
        SDL_Surface* surface = Texture::loadFromFile(path, renderer, texture);
        if (surface) {
            jumpAnim.frames.push_back(texture);
            SDL_FreeSurface(surface);
        }
    }
    animations[JUMPING] = jumpAnim;
    
    // Load falling animation
    Animation fallAnim;
    fallAnim.frameCount = 2;  // Assuming 2 frames for falling
    fallAnim.frameDelay = 200;  // 200ms between frames
    for (int i = 0; i < fallAnim.frameCount; i++) {
        char path[100];
        sprintf(path, "assets/player/land/land_%d.png", i);
        SDL_Texture* texture = nullptr;
        SDL_Surface* surface = Texture::loadFromFile(path, renderer, texture);
        if (surface) {
            fallAnim.frames.push_back(texture);
            SDL_FreeSurface(surface);
        }
    }
    animations[FALLING] = fallAnim;
    
    // Load dashing animation
    Animation dashAnim;
    dashAnim.frameCount = 2;  // Assuming 2 frames for dashing
    dashAnim.frameDelay = 100;  // 100ms between frames
    for (int i = 0; i < dashAnim.frameCount; i++) {
        char path[100];
        sprintf(path, "assets/player/dash/dash_%d.png", i);
        SDL_Texture* texture = nullptr;
        SDL_Surface* surface = Texture::loadFromFile(path, renderer, texture);
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
            Animation& currentAnim = animations[state];
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
        b2Body* playerBody;
        
        float ReportFixture(b2Fixture* fixture, const b2Vec2& point, 
                           const b2Vec2& normal, float fraction) override {
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
            body->ApplyLinearImpulse(b2Vec2(0, -jumpImpulse), body->GetWorldCenter(), true);
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
            float targetVelocityX = walkingDirection * (isRunning ? runSpeed : walkSpeed);
            body->SetLinearVelocity(b2Vec2(targetVelocityX, body->GetLinearVelocity().y));
        }
    }
    
    // Screen wrapping - teleport player to opposite side when leaving screen boundaries
    b2Vec2 position = body->GetPosition();
    bool teleported = false;
    
    // Get screen dimensions
    int screenWidth, screenHeight;
    SDL_GetRendererOutputSize(SDL_GetRenderer(SDL_GetWindowFromID(1)), &screenWidth, &screenHeight);
    
    // Check horizontal boundaries
    if (position.x < -W_SPRITESIZE) {
        position.x = screenWidth + W_SPRITESIZE/2;
        teleported = true;
    } else if (position.x > screenWidth + W_SPRITESIZE) {
        position.x = -W_SPRITESIZE/2;
        teleported = true;
    }
    
    // Check vertical boundaries
    if (position.y < -W_SPRITESIZE) {
        position.y = screenHeight + W_SPRITESIZE/2;
        teleported = true;
    } else if (position.y > screenHeight + W_SPRITESIZE) {
        position.y = -W_SPRITESIZE/2;
        teleported = true;
    }
    
    // Apply teleportation if needed
    if (teleported) {
        body->SetTransform(position, body->GetAngle());
    }
    
    setPosition((int)body->GetPosition().x, (int)body->GetPosition().y);
    
    // Update animation
    updateAnimation();
}

void Player::render(SDL_Renderer* renderer) {
    // Get current animation frame
    SDL_Texture* currentTexture = nullptr;
    
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
        SDL_RenderCopyEx(renderer, currentTexture, nullptr, &destRect, 0, nullptr, flip);
    } else {
        // Fallback to original rendering if animation frame is not available
        Sprite::render(renderer, getX(), getY());
    }
}

void Player::handleEvents(SDL_Event* event) {
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
    }
}