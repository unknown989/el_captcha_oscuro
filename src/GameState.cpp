#include "GameState.hpp"

namespace GameState
{
    bool running = true;
    void quitGame()
    {
        running = false;
    }
    int current_level = -1;
    bool isMenu = true;
    bool isLoading = false;
    bool isPlayingWalkingSound = false;
    bool isPlayingDashingSound = false;
    void setCurrentLevel(int level)
    {
        current_level = level;
        isLoading = true;
        isPlayingWalkingSound = false;
        isPlayingDashingSound = false;
    }
}