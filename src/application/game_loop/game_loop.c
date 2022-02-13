#include "./game_loop.h"

#include <stdlib.h>
#include <stdbool.h>


bool loopRunning = true;

int game_loop() {
    while (loopRunning) {
        // Do events here
        // Do drawing here
        // etc.
    }

    return EXIT_SUCCESS;
}