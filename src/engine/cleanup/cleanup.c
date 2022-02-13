#include "./cleanup.h"

#include "../globals/globals.h"

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>

#include <stdlib.h>


int cleanup(InitializingInfo initInfo) {
    vkDestroySurfaceKHR(*initInfo.pInstance, *initInfo.pSurface, NULL);
    vkDestroyInstance(*initInfo.pInstance, NULL);
    vkDestroySwapchainKHR(*initInfo.pDevice, *initInfo.pSwapChain, NULL);
    vkDestroyDevice(*initInfo.pDevice, NULL);

    SDL_DestroyWindow(*initInfo.pWindow);

    return EXIT_SUCCESS;
}