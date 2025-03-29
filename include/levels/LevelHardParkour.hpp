#pragma once
#include "Level.hpp"
#include <box2d/box2d.h>

// Forward declare Player
class Player;

// Custom contact listener to detect player hitting the exit
class MyContactListener : public b2ContactListener
{
public:
    bool playerHitExit = false; // Flag to indicate player reached exit

    void BeginContact(b2Contact *contact) override
    {
        // Get the two fixtures involved in the contact
        b2Fixture *fixtureA = contact->GetFixtureA();
        b2Fixture *fixtureB = contact->GetFixtureB();

        // Debug: Log fixture contact
        SDL_Log("Contact detected between fixtures: %p and %p", fixtureA, fixtureB);

        // Get the user data from both fixtures
        uintptr_t userDataA = fixtureA->GetUserData().pointer;
        uintptr_t userDataB = fixtureB->GetUserData().pointer;

        // Debug: Log user data
        SDL_Log("User data pointers: %p and %p", (void *)userDataA, (void *)userDataB);

        // Simplified detection: Try to cast each userData to Block* and check
        Block *blockA = nullptr;
        Block *blockB = nullptr;

        if (userDataA)
        {
            blockA = reinterpret_cast<Block *>(userDataA);
            // Only consider it a valid Block if it has a known block type
            if (blockA && (blockA->type != 'p' && blockA->type != 'e' &&
                           blockA->type != 'm' && blockA->type != 'D'))
            {
                blockA = nullptr; // Reset if not a valid block type
            }
        }

        if (userDataB)
        {
            blockB = reinterpret_cast<Block *>(userDataB);
            // Only consider it a valid Block if it has a known block type
            if (blockB && (blockB->type != 'p' && blockB->type != 'e' &&
                           blockB->type != 'm' && blockB->type != 'D'))
            {
                blockB = nullptr; // Reset if not a valid block type
            }
        }

        // At this point, if one pointer is a block and one isn't,
        // assume the non-block pointer is the player (or something else we don't care about)

        // Check for exit block collision
        if ((blockA && blockA->type == 'e') || (blockB && blockB->type == 'e'))
        {
            SDL_Log("Exit block collision detected!");
            playerHitExit = true;
        }

        // Check for parkour block collision
        Block *parkourBlock = nullptr;
        if (blockA && blockA->type == 'p')
        {
            parkourBlock = blockA;
        }
        else if (blockB && blockB->type == 'p')
        {
            parkourBlock = blockB;
        }

        if (parkourBlock)
        {
            SDL_Log("Parkour block collision detected! Block type: %c", parkourBlock->type);

            // Start crumble timer if not already crumbling
            if (!parkourBlock->isCrumbling)
            {
                parkourBlock->isCrumbling = true;
                parkourBlock->crumbleTimer = parkourBlock->timeToCrumble;
                SDL_Log("Starting crumble timer for parkour block! Timer set to: %.2f",
                        parkourBlock->crumbleTimer);
            }
            else
            {
                SDL_Log("Parkour block is already crumbling. Current timer: %.2f",
                        parkourBlock->crumbleTimer);
            }
        }
    }
};

class LevelHardParkour : public Level
{
private:
    int currentDifficulty;
    bool difficultyChanged;
    MyContactListener contactListener; // Add contact listener instance
    bool playerReachedExit = false;    // Flag set by contact listener
    SDL_Renderer *m_renderer;          // Store the renderer pointer

public:
    LevelHardParkour(SDL_Renderer *renderer, int difficulty = 1)
        : Level(renderer), currentDifficulty(difficulty), difficultyChanged(false), m_renderer(renderer) // Store renderer
    {
        // Set even lower gravity for better parkour jumping
        gravity = b2Vec2(0.0f, 0.5f);
        world->SetGravity(gravity);

        // Set the custom contact listener for the world
        world->SetContactListener(&contactListener);

        // Load level based on difficulty
        loadLevelWithDifficulty(m_renderer, currentDifficulty); // Use stored renderer

        // Load background
        loadLevelBackground("assets/backgrounds/level1.png", m_renderer); // Use stored renderer

        // Level is now loaded
        loadLevel();
    }

    ~LevelHardParkour() {}

    void loadLevelWithDifficulty(SDL_Renderer *renderer, int difficulty)
    {
        char levelFilePath[100];
        sprintf(levelFilePath, "levels/hard_parkour_%d.txt", difficulty);

        // Reset the exit flag when loading a new level
        playerReachedExit = false;
        contactListener.playerHitExit = false;

        // Read level from file (this creates the player and blocks)
        readLevel(levelFilePath, renderer); // Use passed renderer

        // Log the difficulty level being loaded
        SDL_Log("Loading parkour level with difficulty %d from %s", difficulty, levelFilePath);

        difficultyChanged = false;
    }

    void changeDifficulty(SDL_Renderer *renderer, int newDifficulty)
    {
        if (newDifficulty < 1)
            newDifficulty = 1;
        if (newDifficulty > 3)
        {
            // If we exceed max difficulty, maybe loop back or end game?
            SDL_Log("Reached max difficulty!");
            GameState::setCurrentLevel(GameState::current_level + 1);
            return; // Don't restart
        }

        if (currentDifficulty != newDifficulty)
        {
            currentDifficulty = newDifficulty;
            difficultyChanged = true;
            restartLevel(renderer); // Use passed renderer for restart
        }
    }

    void update() override
    {
        Level::update(); // This steps the world and updates player

        // Check if the contact listener flagged the exit collision
        if (contactListener.playerHitExit)
        {
            playerReachedExit = true;
            contactListener.playerHitExit = false; // Reset listener flag
        }

        // If player reached exit, change difficulty using the stored renderer
        if (playerReachedExit)
        {
            SDL_Log("Proceeding to next difficulty level.");
            changeDifficulty(m_renderer, currentDifficulty + 1); // Use stored m_renderer
            playerReachedExit = false;                           // Reset level flag
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
        Level::handleEvents(event, renderer); // Pass renderer down

        // Handle level-specific events
        if (event->type == SDL_KEYDOWN)
        {
            // Pass the correct renderer (from parameter) to changeDifficulty
            switch (event->key.keysym.sym)
            {
            case SDLK_F2:
                toggleSnowEffect();
                break;
            case SDLK_r:
                // Check if CTRL is being held (to differentiate from reload)
                if (event->key.keysym.mod & KMOD_CTRL)
                {
                    restartLevel(renderer); // Use passed renderer
                }
                break;
            case SDLK_1:
                // Change to difficulty 1 (easy)
                changeDifficulty(renderer, 1); // Use passed renderer
                break;
            case SDLK_2:
                // Change to difficulty 2 (medium)
                changeDifficulty(renderer, 2); // Use passed renderer
                break;
            case SDLK_3:
                // Change to difficulty 3 (hard)
                changeDifficulty(renderer, 3); // Use passed renderer
                break;
            }
        }
    }

    void restartLevel(SDL_Renderer *renderer)
    {
        // --- 1. Delete C++ Objects ---
        // Delete player object first (its destructor might access world, but body is gone with world)
        if (player)
        {
            delete player;
            player = nullptr;
        }

        // Delete block objects (safe now as their bodies are gone with the world)
        // No need to iterate exitBlocks separately if they are also in blocks
        for (Block *b : blocks)
        {
            // Check if pointer is non-null before deleting, just in case
            if (b)
                delete b;
        }
        blocks.clear();     // Clear the vector of pointers
        exitBlocks.clear(); // Also clear this, assuming it might contain pointers also in blocks

        // --- 2. Delete Old World (Destroys all associated bodies/fixtures) ---
        if (world)
        {
            delete world;
            world = nullptr; // Prevent use after delete
        }

        // --- 3. Reset State ---
        over = false;
        playerReachedExit = false;
        contactListener.playerHitExit = false;
        bodiesToDestroy.clear(); // Clear destruction queue too

        // --- 4. Recreate World ---
        world = new b2World(gravity); // Create new world
        world->SetAllowSleeping(false);
        world->SetContactListener(&contactListener); // Assign listener to new world

        // --- 5. Reload Level ---
        loadLevelWithDifficulty(renderer, currentDifficulty); // Use passed renderer

        SDL_Log("Level restarted with clean physics state, difficulty: %d", currentDifficulty);
    }
};