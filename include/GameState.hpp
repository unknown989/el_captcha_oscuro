#pragma once

namespace GameState {
    extern bool running;
    extern int current_level;
    extern bool isMenu;
    void quitGame();
    void setCurrentLevel(int level);
}