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
    SDL_Window *window;

    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    VkImage *swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    InitializingInfo initInfo = {
        .pWindow = &window,

        .pInstance = &instance,
        .pSurface = &surface,
        .pPhysicalDevice = &physicalDevice,
        .pDevice = &device,

        .pGraphicsQueue = &graphicsQueue,
        .pPresentQueue = &presentQueue,

        .pSwapChain = &swapChain,
        .pSwapChainImages = &swapChainImages,
        .pSwapChainImageFormat = &swapChainImageFormat,
        .pSwapChainExtent = &swapChainExtent
    };

    if (initialize(initInfo) == EXIT_SUCCESS) { printf("Initialized properly!\n"); }
    else { printf("Failed to initialize!\n"); }
    
    if (game_loop() == EXIT_SUCCESS) { printf("Game loop ran properly!\n"); }
    else { printf("Game loop failed!\n"); }

    if (cleanup(initInfo) == EXIT_SUCCESS) { printf("Cleanup ran properly!\n"); }
    else { printf("Cleanup failed!\n"); }

    return 0;
}