#include "../engine/initialize/initialize.h"
#include "./game_loop/game_loop.h"
#include "../engine/cleanup/cleanup.h"

#include "../engine/globals/globals.h"

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>


const char * APPLICATION_TITLE = "pixel_engine test application";
const uint32_t APPLICATION_VERSION = VK_MAKE_VERSION(1, 0, 0);
const int WIN_WIDTH = 1280;
const int WIN_HEIGHT = 720;

int main() {
    // --- Initialize ---
    InitializingInfo initInfo = {  };

    if (initialize(&initInfo) == EXIT_SUCCESS) { printf("Initialized properly!\n"); }
    else { printf("Failed to initialize!\n"); }
    

    // --- Game loop ---
    if (gameLoop(&initInfo) == EXIT_SUCCESS) { printf("Game loop ran properly!\n"); }
    else { printf("Game loop failed!\n"); }


    // --- Cleanup ---
    if (cleanup(&initInfo) == EXIT_SUCCESS) { printf("Cleanup ran properly!\n"); }
    else { printf("Cleanup failed!\n"); }


    return 0;
}