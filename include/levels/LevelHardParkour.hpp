#pragma once
#include "Level.hpp"
#include <box2d/box2d.h>

class LevelHardParkour : public Level
{
private:
    int currentDifficulty;
    bool difficultyChanged;

public:
    LevelHardParkour(SDL_Renderer *renderer, int difficulty = 1) : Level(renderer), currentDifficulty(difficulty), difficultyChanged(false)
    {
        // Set even lower gravity for better parkour jumping
        gravity = b2Vec2(0.0f, 0.5f);
        world->SetGravity(gravity);

        // Load level based on difficulty
        loadLevelWithDifficulty(renderer, currentDifficulty);

        // Load background
        loadLevelBackground("assets/backgrounds/level1.png", renderer);

        // Level is now loaded
        loadLevel();
    }

    ~LevelHardParkour() {}

    void loadLevelWithDifficulty(SDL_Renderer *renderer, int difficulty) 
    {
        char levelFilePath[100];
        sprintf(levelFilePath, "levels/hard_parkour_%d.txt", difficulty);
        
        // Read level from file
        readLevel(levelFilePath, renderer);
        
        // Log the difficulty level being loaded
        SDL_Log("Loading parkour level with difficulty %d from %s", difficulty, levelFilePath);
        
        difficultyChanged = false;
    }

    void changeDifficulty(SDL_Renderer *renderer, int newDifficulty) 
    {
        if (newDifficulty < 1) newDifficulty = 1;
        if (newDifficulty > 3) newDifficulty = 3;
        
        if (currentDifficulty != newDifficulty) {
            currentDifficulty = newDifficulty;
            difficultyChanged = true;
            restartLevel(renderer);
        }
    }

    void update() override
    {
        Level::update();

        // Check if player has reached the end (E block)
        if (player)
        {
            int playerX = player->getX() / W_SPRITESIZE;
            int playerY = player->getY() / W_SPRITESIZE;

            // Check if player is at the exit position (should match where 'E' is in the level file)
            // For level 1, exit is at column 29, row 2
            // For level 2, exit is at column 31, row 2
            // For level 3, exit is at column 31, row 3
            bool reachedExit = false;
            
            switch (currentDifficulty) {
                case 1:
                    reachedExit = (playerY >= 2 && playerX >= 28);
                    break;
                case 2:
                    reachedExit = (playerY >= 2 && playerX >= 30);
                    break;
                case 3:
                    reachedExit = (playerY >= 3 && playerX >= 30);
                    break;
                default:
                    reachedExit = (playerY >= 2 && playerX >= 28);
            }
            
            if (reachedExit) {
                this->over = true;
            }
        }
    }

    void render(SDL_Renderer *renderer) override
    {
        // First render all normal level elements
        Level::render(renderer);

        // Add special rendering for parkour level if needed
        if (debugDraw)
        {
            // Display dash hint
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

            // Create a text surface
            SDL_Rect dashHintRect = {20, 20, 250, 40};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_RenderFillRect(renderer, &dashHintRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &dashHintRect);
            
            // Display the current difficulty
            SDL_Rect difficultyRect = {20, 70, 250, 40};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_RenderFillRect(renderer, &difficultyRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &difficultyRect);
        }

        // Render snow effect on top of everything else
        renderSnowEffect(renderer);
    }

    // Toggle snow effect
    void toggleSnowEffect()
    {
        snowEffectEnabled = !snowEffectEnabled;
        SDL_Log("Snow effect %s", snowEffectEnabled ? "enabled" : "disabled");
    }

    void handleEvents(SDL_Event *event, SDL_Renderer *renderer) override
    {
        // First handle base level events
        Level::handleEvents(event, renderer);

        // Handle level-specific events
        if (event->type == SDL_KEYDOWN)
        {
            switch (event->key.keysym.sym)
            {
            case SDLK_F2:
                toggleSnowEffect();
                break;
            case SDLK_r:
                // Check if CTRL is being held (to differentiate from reload)
                if (event->key.keysym.mod & KMOD_CTRL) {
                    restartLevel(renderer);
                }
                break;
            case SDLK_1:
                // Change to difficulty 1 (easy)
                changeDifficulty(renderer, 1);
                break;
            case SDLK_2:
                // Change to difficulty 2 (medium)
                changeDifficulty(renderer, 2);
                break;
            case SDLK_3:
                // Change to difficulty 3 (hard)
                changeDifficulty(renderer, 3);
                break;
            }
        }
    }

    void restartLevel(SDL_Renderer *renderer)
    {
        // Clean up existing level components
        for (Block *block : blocks)
        {
            // Remove Box2D body if it exists
            if (block->type == 'D' || block->type == 'm' || block->type == 'p') {
                // Find and destroy the body at this position
                for (b2Body* body = world->GetBodyList(); body; body = body->GetNext()) {
                    // Calculate expected position for this block
                    float xPos = (block->getX() + W_SPRITESIZE/2) / 32.0f; // PPM = 32.0f
                    float yPos = (block->getY() + W_SPRITESIZE/2) / 32.0f;
                    
                    // Check if this body is at the expected position (with some tolerance)
                    b2Vec2 bodyPos = body->GetPosition();
                    if (fabs(bodyPos.x - xPos) < 0.1f && fabs(bodyPos.y - yPos) < 0.1f) {
                        world->DestroyBody(body);
                        break;
                    }
                }
            }
            delete block;
        }
        blocks.clear();

        // Clean up player physics body and object
        if (player)
        {
            // Get the player's body and properly remove it from the world
            b2Body* playerBody = player->getBody();
            if (playerBody) {
                world->DestroyBody(playerBody);
            }
            
            delete player;
            player = nullptr;
        }

        // Reset level state
        over = false;

        // Recreate the Box2D world to ensure a clean state
        delete world;
        world = new b2World(gravity);
        world->SetAllowSleeping(false);

        // Re-read the level file with current difficulty
        loadLevelWithDifficulty(renderer, currentDifficulty);

        // Log restart event
        SDL_Log("Level restarted with clean physics state, difficulty: %d", currentDifficulty);
    }
};