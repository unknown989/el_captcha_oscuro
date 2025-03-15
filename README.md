# El Captcha Oscuro

A **2D platformer game** built with **SDL2** and **Box2D** physics engine.

##  Overview

This game features:
- **Physics-based gameplay** using Box2D
- **Level-based progression** system
- **Player character with shooting mechanics**
- **Enemy interactions**
- **Background music and sound effects**
- **Tile-based level design**

## ️ Technologies Used

- **C++**
- **SDL2** (Simple DirectMedia Layer)
  - `SDL2_image` for texture loading
  - `SDL2_ttf` for text rendering
  - `SDL2_mixer` for audio
- **Box2D** physics engine

---

##  Project Structure

```
CSClassGame/
│── assets/            # Game assets (images, fonts, music)
│   ├── backgrounds/   # Background images
│   ├── blocks/        # Block images
│   ├── buttons/       # Button images
│   ├── enemy/         # Enemy images
│   ├── fonts/         # Font files
│   ├── gun/           # Gun images
│   ├── music/         # Music files
│   └── player/        # Player images
│
│── include/           # Header files
│   ├── box2d/         # Box2D headers
│   ├── levels/        # Level headers
│   ├── SDL2/          # SDL2 headers
│   ├── bullet.hpp     # Bullet class header
│   ├── buttons.hpp    # Buttons class header
│   ├── CONSTANTS.hpp  # Constants header
│   ├── enemy.hpp      # Enemy class header
│   ├── game.hpp       # Game class header
│   ├── GameState.hpp  # GameState class header
│   ├── mainmenu.hpp   # Main menu header
│   ├── player.hpp     # Player class header
│   ├── sprite.hpp     # Sprite class header
│   └── textures.hpp   # Textures header
│
│── levels/            # Level definition files
│   ├── info.txt       # Level info
│   ├── lvl1.txt       # Level 1 definition
│   └── lvl_last.txt   # Last level definition
│
│── lib/               # External libraries
│   ├── SDL2_image.lib
│   ├── SDL2_mixer.lib
│   └── SDL2_ttf.lib
│
│── src/               # Source code
│── .gitignore         # Git ignore file
│── .vscode/           # VSCode settings
│── a.exe              # Executable file
│── main.cpp           # Main source file
│── Makefile           # Makefile for building
└── README.md          # This README file
```

---

##  Key Features

###  Level System
- Levels are **loaded from text files**
- **Tile-based level design** with different block types
- Support for **multiple levels** (`lvl1.txt`, `lvl_last.txt`, ...)

### ‍️ Player Mechanics
- **Physics-based movement**
- **Shooting capability**
- **Health system**
- **Bullet management**

###  Graphics
- **Sprite-based rendering**
- **HUD** displaying player stats (health, bullets)
- **Custom font rendering**

###  Physics
- **Box2D integration** for realistic physics simulation
- **Collision detection**
- **Static and dynamic bodies**

###  Audio
- **Background music** support
- **Sound effects** system

---

## ️Building and Running

This project uses **Make** for building.

###  Build the Project:
```bash
make
```

### ️ Run the Executable:
```bash
./a.exe
```

---

##  Code Overview

### **Main File**
The entry point is `main.cpp`. It initializes the game and starts the game loop.

### **Game Class (`game.hpp`)**
Handles the main **game loop**, **rendering**, and **event handling**.

**Methods:**
- `Game::Game()`: Constructor that initializes the game.
- `void Game::run()`: Starts the game loop.
- `void Game::update()`: Updates the game state.
- `void Game::render()`: Renders the game.
- `void Game::handleEvents()`: Handles user input events.
- `void Game::clean()`: Cleans up resources.

### **Level System**
Each level is defined in a separate class.

Example:
- `LevelOne` → Defined in `LevelOne.hpp`
- `LevelLast` → Defined in `LevelLast.hpp`

**Methods:**
- `LevelOne::LevelOne(SDL_Renderer* renderer)`: Constructor that loads Level 1.
- `void LevelOne::render(SDL_Renderer* renderer)`: Renders Level 1.
- `void LevelOne::handleEvents(SDL_Event* event, SDL_Renderer* renderer)`: Handles events for Level 1.
- `LevelOne::~LevelOne()`: Destructor that cleans up resources.

### **Player Class (`player.hpp`)**
Handles **player mechanics**, including movement, shooting, and health.

**Methods:**
- `Player::Player(SDL_Renderer *renderer, b2World *world, int x, int y)`: Initializes the player.
- `void Player::handleEvents(SDL_Event *event, SDL_Renderer *renderer)`: Handles player input.
- `void Player::update()`: Updates the player state.
- `void Player::render(SDL_Renderer *renderer)`: Renders the player.
- `void Player::fireBullet(SDL_Renderer *renderer)`: Fires a bullet.
- `void Player::updateBullets()`: Updates the bullets.
- `void Player::renderBullets(SDL_Renderer *renderer)`: Renders the bullets.
- `void Player::takeDamage(int damage)`: Reduces the player's health.

### **Enemy Class (`enemy.hpp`)**
Handles **enemy interactions and behavior**.

**Methods:**
- `Enemy::Enemy(SDL_Renderer *renderer)`: Initializes the enemy.
- `void Enemy::update()`: Updates enemy state.
- `void Enemy::render(SDL_Renderer *renderer)`: Renders the enemy.
- `void Enemy::fireBullet(SDL_Renderer *renderer)`: Fires a bullet.
- `void Enemy::updateBullets()`: Updates the bullets.
- `void Enemy::takeDamage(int damage)`: Reduces enemy health.

### **Audio System**
Uses `SDL2_mixer`:
- **Background music** (`Mix_LoadMUS`, `Mix_PlayMusic`)
- **Sound effects** (`Mix_Chunk`, `Mix_PlayChannel`)

### **Font System**
Uses `SDL2_ttf`:
- **Load fonts** (`TTF_OpenFont`)
- **Render text** (`TTF_RenderText`)

### **Physics (Box2D)**
- **Physics world updates** inside the `Level` class
- **Physics bodies** assigned to the **player, blocks, bullets**

### **Sprite Class (`sprite.hpp`)**
Base class for all **drawable objects**.

**Methods:**
- `Sprite::Sprite()`: Initializes the sprite.
- `bool Sprite::loadFromFile(const char* path, SDL_Renderer* renderer)`: Loads a texture.
- `void Sprite::render(SDL_Renderer* renderer, int x, int y)`: Renders the sprite.
- `void Sprite::setPosition(int x, int y)`: Sets position.
- `void Sprite::setSize(int w, int h)`: Sets size.

### **GameState (`GameState.hpp`)**
Global **game state variables**.

**Variables:**
- `bool running`: Game is running.
- `int current_level`: Current level.
- `bool isMenu`: Game is in the menu.
- `bool isLoading`: Game is loading.

**Functions:**
- `void quitGame()`: Quits the game.
- `void setCurrentLevel(int level)`: Sets current level.
