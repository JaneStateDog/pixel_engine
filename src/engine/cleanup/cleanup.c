#include "./cleanup.h"

#include "../globals/globals.h"

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>

#include <stdlib.h>


int cleanup(InitializingInfo *tInitInfo) {
    InitializingInfo *initInfo = tInitInfo;


    vkDestroySurfaceKHR(initInfo->instance, initInfo->surface, NULL);

    vkDestroyInstance(initInfo->instance, NULL);

    vkDestroySwapchainKHR(initInfo->device, initInfo->swapChain, NULL);

    for (int i = 0; i < initInfo->swapChainFramebuffersCount; i++) {
        vkDestroyFramebuffer(initInfo->device, initInfo->swapChainFramebuffers[i], NULL);
    }
    for (int i = 0; i < initInfo->swapChainImageViewsCount; i++) {
        vkDestroyImageView(initInfo->device, initInfo->swapChainImageViews[i], NULL);
    }

    vkDestroyPipeline(initInfo->device, initInfo->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(initInfo->device, initInfo->pipelineLayout, NULL);
    vkDestroyRenderPass(initInfo->device, initInfo->renderPass, NULL);

    vkDestroyCommandPool(initInfo->device, initInfo->commandPool, NULL);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(initInfo->device, initInfo->imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(initInfo->device, initInfo->renderFinishedSemaphores[i], NULL);

        vkDestroyFence(initInfo->device, initInfo->inFlightFences[i], NULL);
    }

    vkDestroyDevice(initInfo->device, NULL);


    SDL_DestroyWindow(initInfo->window);


    return EXIT_SUCCESS;
}