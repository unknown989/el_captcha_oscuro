#pragma once
#include "SDL2/SDL_mixer.h"
#include "enemy.hpp"
#include <CONSTANTS.hpp>
#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <cstdio>
#include <player.hpp>
#include <sprite.hpp>
#include <vector>
#include <map>
#include <string>
#include <random>
#include <ctime>

// Structure to represent a snowflake
struct Snowflake {
  float x, y;        // Position
  float speed;       // Falling speed
  float angle;       // Rotation angle
  float size;        // Size of snowflake
  float wobble;      // Horizontal wobble speed
  float wobblePos;   // Current wobble position
};

// A block is a simple sprite with a type
class Block : public Sprite {
public:
  Block(SDL_Renderer* renderer, const char* path) 
    : isCrumbling(false), crumbleTimer(-1.0f), timeToCrumble(2.0f), body(nullptr), isVisible(true) // Initialize new members
  {
    loadFromFile(path, renderer);
    setSize(W_SPRITESIZE, W_SPRITESIZE);
  }
  
  Block(SDL_Texture* texture)
    : isCrumbling(false), crumbleTimer(-1.0f), timeToCrumble(2.0f), body(nullptr), isVisible(true) // Initialize new members
  {
    this->texture = texture;
    setSize(W_SPRITESIZE, W_SPRITESIZE);
    // Set source rectangle for rendering
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = W_SPRITESIZE;
    srcRect.h = W_SPRITESIZE;
  }
  
  char type;
  bool isCrumbling;      // Is the timer active?
  float crumbleTimer;    // Time left before disappearing
  float timeToCrumble;   // How long the block lasts after touch
  b2Body* body;          // Pointer to its physics body
  bool isVisible;        // Control rendering
  
  // Custom render method
  void render(SDL_Renderer *renderer, int x, int y) {
      if (!isVisible) return; // Don't render if crumbled
      
      // Optional: Add visual effect for crumbling (e.g., change color/alpha)
      if (isCrumbling) {
          // Example: Fade out effect
          float alpha = (crumbleTimer / timeToCrumble) * 255.0f;
          if (alpha < 0) alpha = 0;
          if (alpha > 255) alpha = 255;
          SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(alpha));
          
          // Debug message for crumbling
          SDL_Log("Rendering crumbling block: timer=%.2f, alpha=%.0f", crumbleTimer, alpha);
      } else {
          SDL_SetTextureAlphaMod(texture, 255); // Reset alpha if not crumbling
      }
      
      Sprite::render(renderer, x, y); // Call base class render
      
      // Reset alpha mod after rendering this block
      SDL_SetTextureAlphaMod(texture, 255);
  }
};

// The base class for all levels
// It contains the Box2D world, the player, the blocks, and the background
class Level {
public:
  Level(SDL_Renderer *renderer);
  virtual ~Level();
  virtual void render(SDL_Renderer *renderer);
  virtual void handleEvents(SDL_Event *event, SDL_Renderer *renderer);
  virtual void update();
  void readLevel(const char *path, SDL_Renderer *renderer);
  void loadLevelBackground(const char *path, SDL_Renderer *renderer);
  bool isLevelLoaded() { return isLoaded; }
  void loadLevel() { isLoaded = true; }
  void unloadLevel() { isLoaded = false; }
  bool isLevelOver() { return this->over; };
  
  // Debug rendering toggle
  void toggleDebugDraw() { debugDraw = !debugDraw; }
  
  // Snow effect methods
  void initSnowEffect(SDL_Renderer *renderer);
  void updateSnowEffect();
  void renderSnowEffect(SDL_Renderer *renderer);

protected:
  std::vector<Block *> blocks;
  std::vector<Block *> exitBlocks;
  SDL_Texture *background = nullptr;
  Player *player;
  b2Vec2 gravity;
  SDL_Renderer *renderer;
  b2World *world;
  bool isLoaded = false;
  Enemy *enemy;
  bool over = false;
  bool debugDraw = false;  // Flag to toggle debug drawing
  
  // Queue for physics bodies to be destroyed safely after world step
  std::vector<b2Body*> bodiesToDestroy;
  
  // Texture cache to avoid reloading the same textures
  std::map<std::string, SDL_Texture*> textureCache;
  
  // Snow effect properties
  SDL_Texture* snowflakeTexture = nullptr;
  std::vector<Snowflake> snowflakes;
  bool snowEffectEnabled = true;
  int screenWidth = 0;
  int screenHeight = 0;
  std::mt19937 randomGenerator;
  
  // Helper method to get or load a texture
  SDL_Texture* getTexture(const char* path, SDL_Renderer* renderer);
  
  // Debug rendering method
  void renderDebugCollisions(SDL_Renderer* renderer);
};

// Helper method to get or load a texture
SDL_Texture* Level::getTexture(const char* path, SDL_Renderer* renderer) {
  std::string pathStr(path);
  
  // Check if texture is already in cache
  auto it = textureCache.find(pathStr);
  if (it != textureCache.end()) {
    return it->second;
  }
  
  // If not in cache, load it
  SDL_Texture* newTexture = nullptr;
  SDL_Surface* loadedSurface = Texture::loadFromFile(path, renderer, newTexture);
  
  if (loadedSurface == nullptr || newTexture == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "Unable to load image %s! SDL_image Error: %s\n", path,
                SDL_GetError());
    return nullptr;
  }
  
  SDL_FreeSurface(loadedSurface);
  
  // Add to cache
  textureCache[pathStr] = newTexture;
  return newTexture;
}

Level::Level(SDL_Renderer *renderer) : gravity(0.0f, 0.7f), renderer(renderer) {
  // box2d setup
  world = new b2World(gravity);
  world->SetAllowSleeping(false);

  // Improved physics parameters
  b2BodyDef bodyDef;
  bodyDef.linearDamping = 0.3f;  // Reduced for better movement
  bodyDef.angularDamping = 0.1f;
  bodyDef.fixedRotation = true; // Prevent rotation

  background = nullptr;
  player = nullptr;
  
  // Initialize snow effect
  // Get screen dimensions
  SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
  
  // Seed random generator
  randomGenerator.seed(static_cast<unsigned int>(time(nullptr)));
  
  // Initialize snow effect
  initSnowEffect(renderer);
}

Level::~Level() {
  for (Block *block : blocks) {
    delete block;
  }
  blocks.clear();

  // Clean up texture cache
  for (auto& pair : textureCache) {
    if (pair.second) {
      SDL_DestroyTexture(pair.second);
    }
  }
  textureCache.clear();

  // Clean up snowflake texture
  if (snowflakeTexture) {
    SDL_DestroyTexture(snowflakeTexture);
  }

  delete world; // Clean up Box2D world
  if (player) {
    delete player;
  }
  if (background) {
    SDL_DestroyTexture(background);
  }
}

void Level::loadLevelBackground(const char *path, SDL_Renderer *renderer) {
  // Load the background image
  SDL_Surface *loadedSurface =
      Texture::loadFromFile(path, renderer, background);
  if (loadedSurface == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Unable to load image %s! SDL_image Error: %s\n", path,
                 SDL_GetError());
    exit(1);
  }
  SDL_FreeSurface(loadedSurface);
}

void Level::readLevel(const char *path, SDL_Renderer *renderer) {
  // Add conversion factor (pixels per meter)
  const float PPM = 32.0f;  // 32 pixels = 1 Box2D meter

  // Open the file
  FILE *file = fopen(path, "r");
  if (file == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not open file %s", path);
    return;
  }
  int row = 0;
  int col = 0;
  char blockType;
  // the level file format is 30 blocks by 17 blocks
  while ((blockType = fgetc(file)) != EOF && row < 30) {
    if (blockType == '\n') {
      row++;
      col = 0;
      continue;
    }
    Block *block = nullptr;
    // Loading blocks according to the type
    switch (blockType) {
      // Dirt block + physics
    case 'D': {
      SDL_Texture* texture = getTexture("assets/blocks/dirt.png", renderer);
      if (texture) {
        block = new Block(texture);
        block->type = blockType;
        
        // Set block position in pixels
        block->setPosition(col * W_SPRITESIZE, row * W_SPRITESIZE);
        
        // Convert pixel coordinates to Box2D meters - center of the block
        float xPos = (col * W_SPRITESIZE) + (W_SPRITESIZE/2);
        float yPos = (row * W_SPRITESIZE) + (W_SPRITESIZE/2);
        
        // Set collision dimensions to match sprite size exactly
        float blockWidth = W_SPRITESIZE;
        float blockHeight = W_SPRITESIZE;

        b2BodyDef blockBodyDef;
        blockBodyDef.type = b2_staticBody;
        blockBodyDef.position.Set(xPos / PPM, yPos / PPM);
        b2Body *blockBody = world->CreateBody(&blockBodyDef);
        
        // Make hitbox match sprite size closely to provide solid surfaces
        float hitboxScale = 0.99f;  // Reverted from 0.7f back to 0.99f as requested
        b2PolygonShape blockShape;
        blockShape.SetAsBox(
            (blockWidth/2 * hitboxScale) / PPM, 
            (blockHeight/2 * hitboxScale) / PPM
        );
        
        b2FixtureDef blockFixtureDef;
        blockFixtureDef.shape = &blockShape;
        blockFixtureDef.density = 1.0f;
        blockFixtureDef.friction = 0.01f;  // Keep lower friction to prevent sticking
        blockFixtureDef.restitution = 0.0f;  // No bounce
        
        // Add collision filtering
        blockFixtureDef.filter.categoryBits = 0x0001;  // Block category
        blockFixtureDef.filter.maskBits = 0xFFFF;      // Collide with everything
        
        blockBody->CreateFixture(&blockFixtureDef);
      }
      break;
    }
    
    // Update maze block with the same improvements
    case 'm': {
      SDL_Texture* texture = getTexture("assets/blocks/maze.png", renderer);
      if (texture) {
        block = new Block(texture);
        block->type = blockType;
        
        // Set block position in pixels
        block->setPosition(col * W_SPRITESIZE, row * W_SPRITESIZE);
        
        // Same coordinate conversion as dirt block
        float xPos = (col * W_SPRITESIZE) + (W_SPRITESIZE/2);
        float yPos = (row * W_SPRITESIZE) + (W_SPRITESIZE/2);
        
        b2BodyDef blockBodyDef;
        blockBodyDef.type = b2_staticBody;
        blockBodyDef.position.Set(xPos / PPM, yPos / PPM);
        b2Body *blockBody = world->CreateBody(&blockBodyDef);
        
        // Make hitbox match sprite size closely to provide solid surfaces
        float hitboxScale = 0.99f;  // Reverted from 0.7f back to 0.99f as requested
        b2PolygonShape blockShape;
        blockShape.SetAsBox(
            (W_SPRITESIZE/2 * hitboxScale) / PPM, 
            (W_SPRITESIZE/2 * hitboxScale) / PPM
        );
        
        b2FixtureDef blockFixtureDef;
        blockFixtureDef.shape = &blockShape;
        blockFixtureDef.density = 1.0f;
        blockFixtureDef.friction = 0.001f;  // Nearly zero friction for icy sliding effect
        blockFixtureDef.restitution = 0.05f;  // Slight bounce for smoother movement
        
        // Add collision filtering
        blockFixtureDef.filter.categoryBits = 0x0001;  // Block category
        blockFixtureDef.filter.maskBits = 0xFFFF;      // Collide with everything
        
        blockBody->CreateFixture(&blockFixtureDef);
      }
      break;
    }
    
    // Parkour block - smaller and bouncier for platforming
    case 'p': {
      SDL_Texture* texture = getTexture("assets/blocks/parkour.png", renderer);
      if (!texture) {
        texture = getTexture("assets/blocks/maze.png", renderer);
      }
      
      if (texture) {
        block = new Block(texture);
        block->type = blockType;
        block->setPosition(col * W_SPRITESIZE, row * W_SPRITESIZE);
        
        float xPos = (col * W_SPRITESIZE) + (W_SPRITESIZE/2);
        float yPos = (row * W_SPRITESIZE) + (W_SPRITESIZE/2);
        
        b2BodyDef blockBodyDef;
        blockBodyDef.type = b2_staticBody; // Keep static for now
        blockBodyDef.position.Set(xPos / PPM, yPos / PPM);
        b2Body *blockBody = world->CreateBody(&blockBodyDef);
        
        // Store the body pointer in the Block object
        block->body = blockBody; 
        
        float hitboxScale = 0.5f;
        b2PolygonShape blockShape;
        blockShape.SetAsBox((W_SPRITESIZE * hitboxScale) / PPM, (W_SPRITESIZE * hitboxScale) / PPM);
        
        b2FixtureDef blockFixtureDef;
        blockFixtureDef.shape = &blockShape;
        blockFixtureDef.density = 1.0f;
        blockFixtureDef.friction = 0.001f; 
        blockFixtureDef.restitution = 0.05f;
        
        // Store the Block pointer in the fixture's user data for the listener
        blockFixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(block);
        
        blockBody->CreateFixture(&blockFixtureDef);
      }
      break;
    }
    
    case 'P': {
      // player sprite
      player =
          new Player(renderer, world, col * W_SPRITESIZE, row * W_SPRITESIZE);
      break;
    }
    case 'E': {
      // enemy sprite
      enemy = new Enemy(renderer);
      enemy->setPosition(col * W_SPRITESIZE, row * W_SPRITESIZE);
      enemy->setSize(W_SPRITESIZE, W_SPRITESIZE);
      break;
    }
    
    // Add exit block type
    case 'e': {
      SDL_Texture* texture = getTexture("assets/blocks/exit.png", renderer);
      if (!texture) {
        // Fallback to dirt texture if exit texture doesn't exist
        texture = getTexture("assets/blocks/dirt.png", renderer);
      }
      
      if (texture) {
        block = new Block(texture);
        block->type = blockType;
        
        // Set block position in pixels
        block->setPosition(col * W_SPRITESIZE, row * W_SPRITESIZE);
        
        // Create a static body for collision detection, but make it a sensor
        float xPos = (col * W_SPRITESIZE) + (W_SPRITESIZE/2);
        float yPos = (row * W_SPRITESIZE) + (W_SPRITESIZE/2);
        
        b2BodyDef exitBodyDef;
        exitBodyDef.type = b2_staticBody;
        exitBodyDef.position.Set(xPos / PPM, yPos / PPM);
        b2Body *exitBody = world->CreateBody(&exitBodyDef);
        
        b2PolygonShape exitShape;
        // Use a slightly smaller hitbox for the sensor if needed, or full size
        exitShape.SetAsBox((W_SPRITESIZE/2) / PPM, (W_SPRITESIZE/2) / PPM); 
        
        b2FixtureDef exitFixtureDef;
        exitFixtureDef.shape = &exitShape;
        exitFixtureDef.isSensor = true; // Make it a sensor so player passes through
        
        // Store the Block pointer in userData to identify it during collision
        // Use a struct or pair if you need to store more data later
        exitFixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(block); 
        
        exitBody->CreateFixture(&exitFixtureDef);
        
        // Add to a separate list if needed, or just the main blocks list
        exitBlocks.push_back(block); // Keep track for rendering if needed
      }
      break;
    }
    default:
      col++;
      continue;
    }

    if (block) {
      block->type = blockType;
      // We already set the position above, so no need to do it again here
      blocks.push_back(block);
    }
    col++;
  }

  fclose(file);
  SDL_Log("Loaded %zu blocks", blocks.size());
}

void Level::update() {
  // --- Update Crumbling Blocks --- 
  float timeStep = 1.0f / 60.0f; // Assuming 60 FPS, get this properly if possible
  
  // Count crumbling blocks for debug
  int crumblingBlockCount = 0;
  
  for (Block* block : blocks) {
      // Ensure block pointer is valid before accessing members
      if (!block) continue; 
      
      // Debug: Log parkour block state periodically 
      if (block->type == 'p') {
          static int frameCounter = 0;
          if (frameCounter++ % 60 == 0) { // Log every 60 frames
              SDL_Log("Parkour block status: isCrumbling=%d, timer=%.2f, isVisible=%d", 
                     block->isCrumbling, block->crumbleTimer, block->isVisible);
          }
      }
      
      if (block->type == 'p' && block->isCrumbling) {
          crumblingBlockCount++;
          
          block->crumbleTimer -= timeStep;
          SDL_Log("Updating crumbling block timer: %.2f", block->crumbleTimer);
          
          if (block->crumbleTimer <= 0) {
              // Timer finished, mark for destruction and hide
              if (block->body && block->isVisible) { // Check if body exists and not already marked
                  SDL_Log("Parkour block timer finished. Queuing body %p for destruction.", block->body);
                  bodiesToDestroy.push_back(block->body);
                  block->isVisible = false; // Stop rendering
                  block->isCrumbling = false; // Stop timer updates
                  block->body = nullptr; // Prevent adding again
              }
          }
      }
  }
  
  // Debug: log the count of crumbling blocks
  if (crumblingBlockCount > 0) {
      SDL_Log("Total crumbling blocks: %d", crumblingBlockCount);
  }

  // --- Update Player Physics --- 
  if (player) {
    player->updatePhysics();
  }

  // --- Step Physics World --- 
  const int velocityIterations = 8;
  const int positionIterations = 3;
  if (world) { // Ensure world exists before stepping
      world->Step(timeStep, velocityIterations, positionIterations);
  }

  // --- Destroy Queued Bodies --- 
  // Safely destroy bodies AFTER the world step
  if (world && !bodiesToDestroy.empty()) {
       SDL_Log("Processing destruction queue: %zu bodies.", bodiesToDestroy.size());
       for (b2Body* body : bodiesToDestroy) {
           if (body) { // Double check pointer is valid
               SDL_Log("Destroying body %p", body);
               world->DestroyBody(body);
           } else {
               SDL_Log("Attempted to destroy a null body pointer in queue.");
           }
       }
       bodiesToDestroy.clear(); // Clear the queue
       SDL_Log("Destruction queue processed.");
  }

  // --- Update Player Position/State (Based on new physics state) --- 
  if (player) {
    player->update();
  }
  
  // --- Update Snow --- 
  // Update snow physics - keep animation smooth
  updateSnowEffect(); 
}

void Level::render(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
  SDL_RenderClear(renderer);
  // Render the background
  if (background != nullptr) {
    SDL_RenderCopy(renderer, background, nullptr, nullptr);
  }
  // Render the blocks
  for (Block *block : blocks) {
    block->render(renderer, block->getX(), block->getY());
  }
  // Render the player
  if (player) {
    player->render(renderer);
  }
  
  // Render debug collision boxes if enabled
  if (debugDraw) {
    renderDebugCollisions(renderer);
  }
  
  // Update snow physics - moved here to keep animation smooth
  updateSnowEffect();
}

void Level::renderDebugCollisions(SDL_Renderer* renderer) {
  const float PPM = 32.0f;  // 32 pixels = 1 Box2D meter
  
  // Set debug draw color (semi-transparent red)
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
  
  // Iterate through all bodies in the world
  for (b2Body* body = world->GetBodyList(); body; body = body->GetNext()) {
    // Iterate through all fixtures on this body
    for (b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
      // We only handle polygon shapes for now
      if (fixture->GetType() == b2Shape::e_polygon) {
        b2PolygonShape* poly = (b2PolygonShape*)fixture->GetShape();
        
        // Get vertex count
        int vertexCount = poly->m_count;
        
        // Convert vertices to screen coordinates
        SDL_Point points[b2_maxPolygonVertices];
        for (int i = 0; i < vertexCount; i++) {
          // Get vertex in world coordinates
          b2Vec2 worldPoint = body->GetWorldPoint(poly->m_vertices[i]);
          
          // Convert to screen coordinates
          points[i].x = (int)(worldPoint.x * PPM);
          points[i].y = (int)(worldPoint.y * PPM);
        }
        
        // Draw the polygon outline
        for (int i = 0; i < vertexCount; i++) {
          int j = (i + 1) % vertexCount;
          SDL_RenderDrawLine(renderer, points[i].x, points[i].y, points[j].x, points[j].y);
        }
        
        // Also draw a filled polygon with lower alpha
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 64);
        if (vertexCount >= 3) {
          // For simple boxes, we can use this approach
          SDL_Rect rect;
          rect.x = points[0].x;
          rect.y = points[0].y;
          rect.w = points[2].x - points[0].x;
          rect.h = points[2].y - points[0].y;
          SDL_RenderFillRect(renderer, &rect);
        }
        
        // Reset color for next shape
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
      }
    }
  }
  
  // Also render the player's sprite bounds if debug is enabled
  if (player) {
    player->renderDebugSpriteBounds(renderer);
  }
  
  // Reset blend mode
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void Level::handleEvents(SDL_Event *event, SDL_Renderer *renderer) {
  if (player) {
    player->handleEvents(event, renderer);
  }
  
  // Toggle debug drawing with F1 key
  if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_F1) {
    toggleDebugDraw();
    SDL_Log("Debug drawing %s", debugDraw ? "enabled" : "disabled");
  }
}

void Level::initSnowEffect(SDL_Renderer *renderer) {
  // Load snowflake texture
  snowflakeTexture = getTexture("assets/snow/flake.png", renderer);
  
  // If texture couldn't be loaded, create a simple white dot texture
  if (!snowflakeTexture) {
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 16, 16, 32, 0, 0, 0, 0);
    if (surface) {
      SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 255, 255, 255, 200));
      snowflakeTexture = SDL_CreateTextureFromSurface(renderer, surface);
      SDL_FreeSurface(surface);
    }
  }
  
  // Create distribution for random values
  std::uniform_real_distribution<float> xDist(0.0f, static_cast<float>(screenWidth));
  std::uniform_real_distribution<float> speedDist(1.0f, 3.0f);
  std::uniform_real_distribution<float> sizeDist(0.2f, 1.0f);
  std::uniform_real_distribution<float> wobbleDist(0.1f, 0.5f);
  std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
  
  // Create 200 snowflakes with random properties
  snowflakes.clear();
  for (int i = 0; i < 200; i++) {
    Snowflake flake;
    flake.x = xDist(randomGenerator);
    flake.y = std::uniform_real_distribution<float>(-50.0f, static_cast<float>(screenHeight))(randomGenerator);
    flake.speed = speedDist(randomGenerator);
    flake.size = sizeDist(randomGenerator);
    flake.wobble = wobbleDist(randomGenerator);
    flake.wobblePos = 0.0f;
    flake.angle = angleDist(randomGenerator);
    snowflakes.push_back(flake);
  }
}

void Level::updateSnowEffect() {
  if (!snowEffectEnabled) return;
  
  // Create distribution for random values
  std::uniform_real_distribution<float> xDist(0.0f, static_cast<float>(screenWidth));
  std::uniform_real_distribution<float> speedDist(1.0f, 3.0f);
  std::uniform_real_distribution<float> sizeDist(0.2f, 1.0f);
  std::uniform_real_distribution<float> wobbleDist(0.1f, 0.5f);
  std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
  
  // Update each snowflake
  for (auto& flake : snowflakes) {
    // Update position
    flake.y += flake.speed;
    
    // Update wobble
    flake.wobblePos += flake.wobble;
    flake.x += sinf(flake.wobblePos) * 0.5f;
    
    // Rotate snowflake
    flake.angle += 0.1f * flake.speed;
    if (flake.angle > 360.0f) flake.angle -= 360.0f;
    
    // Reset snowflake if it goes off screen
    if (flake.y > screenHeight + 20) {
      flake.x = xDist(randomGenerator);
      flake.y = -10.0f;
      flake.speed = speedDist(randomGenerator);
      flake.size = sizeDist(randomGenerator);
      flake.wobble = wobbleDist(randomGenerator);
    }
  }
}

void Level::renderSnowEffect(SDL_Renderer *renderer) {
  if (!snowEffectEnabled || !snowflakeTexture) return;
  
  // Get texture dimensions
  int texWidth, texHeight;
  SDL_QueryTexture(snowflakeTexture, NULL, NULL, &texWidth, &texHeight);
  
  for (const auto& flake : snowflakes) {
    // Set up destination rectangle
    SDL_Rect destRect = {
      static_cast<int>(flake.x - (texWidth * flake.size) / 2),
      static_cast<int>(flake.y - (texHeight * flake.size) / 2),
      static_cast<int>(texWidth * flake.size),
      static_cast<int>(texHeight * flake.size)
    };
    
    // Set up rotation center
    SDL_Point center = {
      static_cast<int>(texWidth * flake.size) / 2,
      static_cast<int>(texHeight * flake.size) / 2
    };
    
    // Render snowflake with rotation
    SDL_RenderCopyEx(
      renderer,
      snowflakeTexture,
      NULL,           // Use the entire texture
      &destRect,      // Destination rectangle
      flake.angle,    // Rotation angle
      &center,        // Rotation center
      SDL_FLIP_NONE   // No flipping
    );
  }
}

// Code created by Mouttaki Omar(王明清)