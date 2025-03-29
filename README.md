# El Captcha Oscuro

A sophisticated **2D platformer game** built with **SDL2** and **Box2D** physics engine, featuring advanced physics-based gameplay, multiple level types, and rich visual effects.

## 🎮 Game Overview

El Captcha Oscuro is a physics-based platformer that combines precise movement mechanics with shooting elements. The game features:

- **Advanced physics simulation** using Box2D for realistic movement and collisions
- **Multiple specialized level types** including parkour challenges, puzzles, and boss fights
- **Dynamic player mechanics** with running, jumping, dashing, and shooting capabilities
- **Sophisticated animation system** with state-based character animations
- **Environmental effects** including snow particles and crumbling platforms
- **Immersive audio** with background music and contextual sound effects
- **Tile-based level design** with various block types and properties

## 🧰 Technical Architecture

### Core Technologies

- **C++** as the primary programming language
- **SDL2** (Simple DirectMedia Layer) for cross-platform rendering and input handling
  - `SDL2_image` for texture loading and manipulation
  - `SDL2_ttf` for font rendering
  - `SDL2_mixer` for audio management
- **Box2D** physics engine for realistic physics simulation
- **Theora/Vorbis** for video and audio compression

### Engine Design

The game is built on a custom engine with the following key components:

1. **Game Loop System**
   - Managed by the `Game` class which handles initialization, updates, rendering, and cleanup
   - Maintains a consistent frame rate with separate update and render phases
   - Handles state transitions between menu, loading screens, and gameplay

2. **Level Management System**
   - Base `Level` class with specialized derived classes for different level types
   - Level loading from text files with character-based tile mapping
   - Physics world management with Box2D integration
   - Collision detection and response handling

3. **Rendering System**
   - Sprite-based rendering with texture caching
   - Animation system with frame-based animations for different states
   - Particle effects for environmental elements like snow
   - Debug rendering capabilities for physics objects

4. **Input System**
   - Event-based input handling for keyboard and mouse
   - Configurable controls for player movement and actions
   - UI interaction for menus and buttons

5. **Audio System**
   - Background music management with volume control
   - Sound effect triggering for player actions and environmental events
   - Audio resource management with preloading

6. **Physics Integration**
   - Box2D world simulation with custom gravity settings
   - Conversion between pixel coordinates and physics units (meters)
   - Custom collision filtering for different object types
   - Ray casting for ground detection and other physics queries

## 📁 Project Structure

```
CSClassGame/
│
├── assets/                  # Game assets
│   ├── backgrounds/         # Background images for levels and menus
│   ├── blocks/              # Block textures (dirt, ice, parkour, etc.)
│   ├── buttons/             # UI button textures
│   ├── enemy/               # Enemy character sprites
│   ├── fonts/               # Font files for text rendering
│   ├── gun/                 # Weapon textures
│   ├── input_box/           # UI input elements
│   ├── lamps/               # Light source textures
│   ├── music/               # Background music tracks
│   ├── player/              # Player character animations
│   │   ├── idle/            # Idle animation frames
│   │   ├── jump/            # Jump animation frames
│   │   ├── land/            # Landing animation frames
│   │   ├── sprint/          # Running animation frames
│   │   └── walk/            # Walking animation frames
│   ├── snow/                # Snow particle textures
│   ├── sounds/              # Sound effect files
│   ├── trivia/              # Trivia level assets
│   └── video/               # Video files for cutscenes
│
├── include/                 # Header files
│   ├── box2d/               # Box2D physics engine headers
│   ├── levels/              # Level class headers
│   │   ├── Credits.hpp      # Credits screen implementation
│   │   ├── Level.hpp        # Base level class
│   │   ├── LevelHardParkour.hpp # Parkour level implementation
│   │   ├── LevelLamp.hpp    # Lamp puzzle level
│   │   ├── LevelLast.hpp    # Final level/boss fight
│   │   ├── LevelOne.hpp     # First main level
│   │   ├── LevelTrivia.hpp  # Trivia challenge level
│   │   └── LevelZero.hpp    # Tutorial level
│   ├── ogg/                 # Ogg format headers
│   ├── SDL2/                # SDL2 library headers
│   ├── theora/              # Theora video headers
│   ├── vorbis/              # Vorbis audio headers
│   ├── bullet.hpp           # Projectile implementation
│   ├── buttons.hpp          # UI button system
│   ├── CONSTANTS.hpp        # Global constants and settings
│   ├── enemy.hpp            # Enemy character implementation
│   ├── game.hpp             # Main game class
│   ├── GameState.hpp        # Game state management
│   ├── mainmenu.hpp         # Main menu implementation
│   ├── player.hpp           # Player character implementation
│   ├── soundmanager.hpp     # Audio system management
│   ├── sprite.hpp           # Base sprite class
│   └── textures.hpp         # Texture loading utilities
│
├── levels/                  # Level definition files
│   ├── hard_parkour_1.txt   # Parkour challenge level 1
│   ├── hard_parkour_2.txt   # Parkour challenge level 2
│   ├── hard_parkour_3.txt   # Parkour challenge level 3
│   ├── info.txt             # Level information
│   ├── lvl1.txt             # Main level 1 layout
│   ├── lvl_last.txt         # Final level layout
│   └── maze.txt             # Maze level layout
│
├── lib/                     # External library binaries
│
├── src/                     # Source code implementation
│   ├── collision/           # Box2D collision handling
│   ├── common/              # Common utilities
│   ├── dynamics/            # Box2D dynamics implementation
│   ├── rope/                # Rope physics implementation
│   ├── GameState.cpp        # Game state implementation
│   └── theoraplay.c         # Theora video playback
│
├── main.cpp                 # Application entry point
├── Makefile                 # Build configuration
└── README.md                # Project documentation
```

## 🎯 Game Components

### Player System

The `Player` class (`player.hpp`) implements a sophisticated character with:

- **State-based behavior** (idle, walking, jumping, falling, sprinting, dashing)
- **Physics-based movement** with Box2D integration
- **Advanced ground detection** using ray casting
- **Animation system** with frame-based animations for each state
- **Weapon mechanics** with aiming and shooting
- **Health and ammunition** management
- **Sound effect integration** for actions like jumping, shooting, and walking

Key player features:
```cpp
// Player states
enum PlayerState {
  IDLE, WALKING, JUMPING, FALLING, SPRINT, DASHING
};

// Movement parameters
int runSpeed = 30;
int walkSpeed = 15;
int jumpForce = 60;
int dashForce = 150;
float dashVelocity = 250.0f;

// Animation system
std::map<PlayerState, Animation> animations;
```

### Level System

The base `Level` class (`levels/Level.hpp`) provides:

- **Box2D physics world** management
- **Tile-based level loading** from text files
- **Block creation and management** with different types and properties
- **Player and enemy integration**
- **Environmental effects** like snow particles
- **Collision detection** between game elements

Level types include:
- **Tutorial levels** for introducing game mechanics
- **Parkour challenges** with precise jumping sequences
- **Puzzle levels** requiring specific interactions
- **Trivia challenges** with question-answer gameplay
- **Boss fights** with unique enemy behaviors

Block types include:
- **Dirt blocks** (`D`) - Standard solid platforms
- **Maze blocks** (`m`) - Slippery ice-like surfaces
- **Parkour blocks** (`p`) - Smaller platforms for challenging jumps
- **Crumbling blocks** - Platforms that disappear after being touched
- **Exit blocks** (`e`) - Level completion triggers

### Physics System

The game uses Box2D for realistic physics with:

- **Custom gravity settings** for different level types
- **Precise collision detection** with fixture shapes
- **Material properties** like friction and restitution
- **Ray casting** for ground detection and line-of-sight checks
- **Dynamic and static bodies** for different game elements
- **Joints** for connected objects
- **Collision filtering** to control what objects interact

Physics integration example:
```cpp
// Create player physics body
b2BodyDef bodyDef;
bodyDef.type = b2_dynamicBody;
bodyDef.position.Set((x + W_SPRITESIZE / 2) / PPM, (y + W_SPRITESIZE / 2) / PPM);
bodyDef.fixedRotation = true;
body = world->CreateBody(&bodyDef);

// Create collision shape
float hitboxScale = 0.7f;
b2PolygonShape shape;
shape.SetAsBox((playerWidth / 2 * hitboxScale) / PPM, (playerHeight / 2 * hitboxScale) / PPM);

// Set up physics properties
b2FixtureDef fixtureDef;
fixtureDef.shape = &shape;
fixtureDef.density = 1.0f;
fixtureDef.friction = 0.001f;
fixtureDef.restitution = 0.05f;
```

### Graphics System

The rendering system features:

- **Sprite-based rendering** with the `Sprite` base class
- **Animation system** for frame-based character animations
- **Texture caching** to optimize memory usage
- **Particle effects** for environmental elements
- **Debug visualization** for physics objects
- **HUD rendering** for player stats and game information

### Audio System

The `SoundManager` singleton provides:

- **Background music** playback with volume control
- **Sound effect** triggering with spatial positioning
- **Resource management** for audio files
- **Channel management** for multiple simultaneous sounds

Audio implementation:
```cpp
// Music loading and playback
SOUND_MANAGER.loadMusic("menu", "assets/music/Sadness to happiness.wav");
SOUND_MANAGER.loadMusic("boss", "assets/music/La Fiola 2.wav");
SOUND_MANAGER.playMusic("menu");
SOUND_MANAGER.setMusicVolume(10);

// Sound effects
SOUND_MANAGER.loadSoundEffect("click", "assets/sounds/click.wav");
SOUND_MANAGER.loadSoundEffect("jump", "assets/sounds/jump.wav");
SOUND_MANAGER.loadSoundEffect("shoot", "assets/sounds/shoot.wav");
```

### UI System

The user interface includes:

- **Main menu** with buttons for game options
- **In-game HUD** showing player health and ammunition
- **Level transition screens**
- **Credits display**
- **Button system** with hover and click effects

## 🚀 Building and Running

### Prerequisites

- C++ compiler with C++11 support
- SDL2, SDL2_image, SDL2_ttf, and SDL2_mixer libraries
- Box2D physics library

### Build Instructions

The project uses a Makefile for building:

```bash
# Clone the repository
git clone https://github.com/yourusername/CSClassGame.git
cd CSClassGame

# Build the project
make

# Run the game
./a.exe
```

## 🎮 Gameplay

### Controls

- **WASD** or **Arrow Keys**: Move the player
- **Space**: Jump
- **Shift**: Sprint
- **Left Mouse Button**: Shoot
- **Mouse Movement**: Aim weapon
- **R**: Reload weapon
- **Q**: Quit game

### Game Flow

1. **Main Menu**: Select "Play" to start the game
2. **Level Progression**: Complete each level to advance to the next
3. **Challenges**: Navigate platforms, solve puzzles, defeat enemies
4. **Final Level**: Defeat the boss to complete the game
5. **Credits**: View the game credits after completion

## 🧠 Technical Insights

### Physics Optimization

- **Sleep management** for inactive bodies
- **Broadphase collision detection** to reduce collision checks
- **Custom collision filtering** to optimize interactions
- **Fixture caching** to reduce memory allocation

### Memory Management

- **Texture caching** to avoid redundant loading
- **Smart pointers** for automatic resource cleanup
- **Object pooling** for frequently created/destroyed objects
- **Explicit cleanup** in destructors to prevent memory leaks

### Performance Considerations

- **Frame rate management** to ensure consistent gameplay
- **Render culling** for off-screen objects
- **Physics step tuning** for balance between accuracy and performance
- **Asset preloading** to minimize loading times

## 🔧 Extending the Game

### Adding New Levels

1. Create a new level text file in the `levels/` directory
2. Implement a new level class inheriting from `Level`
3. Add the level to the level selection system in `Game::update()`

### Creating New Block Types

1. Add a new character identifier in the level file format
2. Create textures for the new block type
3. Implement the block's behavior in `Level::readLevel()`

### Adding New Enemies

1. Extend the `Enemy` class or create a new enemy class
2. Implement unique behavior and rendering
3. Add the enemy to level files with a new identifier

## 🌟 Credits

Game created by Mouttaki Omar (王明清)

---

*This README provides a comprehensive overview of the El Captcha Oscuro game codebase. For specific implementation details, refer to the source code files.*
