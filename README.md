# CSClassGame

A 2D platformer game built with SDL2 and Box2D physics engine.

## Overview
This is a 2D game that features:
- Physics-based gameplay using Box2D
- Level-based progression system
- Player character with shooting mechanics
- Enemy interactions
- Background music and sound effects
- Tile-based level design

## Technologies Used

- C++
- SDL2 (Simple DirectMedia Layer)
  - SDL2_image for texture loading
  - SDL2_ttf for text rendering
  - SDL2_mixer for audio
- Box2D physics engine

## Project Structure
CSClassGame/<br />
&nbsp;&nbsp;├── assets/           # Game assets (images, fonts, music)<br />
&nbsp;&nbsp;├── include/          # Header files<br />
&nbsp;&nbsp;├── levels/           # Level definition files<br />
&nbsp;&nbsp;├── lib/             # External libraries<br />
&nbsp;&nbsp;└── src/             # Source code<br />

## Key Features

### Level System
- Levels are loaded from text files
- Tile-based level design with different block types
- Support for multiple levels (lvl1.txt, lvl_last.txt, ...)

### Player Mechanics
- Physics-based movement
- Shooting capability
- Health system
- Bullet management

### Graphics
- Sprite-based rendering
- HUD displaying player stats (health, bullets)
- Custom font rendering

### Physics
- Box2D integration for realistic physics simulation
- Collision detection
- Static and dynamic bodies

### Audio
- Background music support
- Sound effect system

## Building and Running

This project uses Make for building. To build the project:

```bash
make
```
and run a.exe after building.