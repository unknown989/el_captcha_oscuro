#pragma once
// Global game state
namespace GameState
{
    extern bool running;
    extern int current_level;
    extern bool isMenu;
    extern bool isLoading;
    void quitGame();
    void setCurrentLevel(int level);
    extern bool isPlayingWalkingSound;
    extern bool isPlayingDashingSound;
}

// Code created by Mouttaki Omar(王明清)