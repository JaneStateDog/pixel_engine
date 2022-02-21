#include "./game_loop.h"
#include "../../engine/globals/globals.h"

#include <SDL2/SDL.h>

#include <stdlib.h>
#include <stdbool.h>


InitializingInfo initInfo = {};
bool loopRunning = true;


int drawFrame() {

    return EXIT_SUCCESS;
}


int gameLoop(InitializingInfo *tInitInfo) {
    initInfo = *tInitInfo;


    while (loopRunning) {
        // --- Events ---
        

        // -- Draw ---
        drawFrame();
    }


    *tInitInfo = initInfo;

    return EXIT_SUCCESS;
}