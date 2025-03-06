#include "GameState.hpp"

namespace GameState {
    bool running = true;
    void quitGame(){
        running = false;
    }
    int current_level = -1;
    bool isMenu = true;

    void setCurrentLevel(int level){
        current_level = level;
    }
}