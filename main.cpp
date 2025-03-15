#define SDL_MAIN_HANDLED
#include "game.hpp"

int main(int argc, char* argv[]) {

    Game game;

    game.run();
    game.clean();

    return 0;
}

// Code created by Mouttaki Omar(王明清)