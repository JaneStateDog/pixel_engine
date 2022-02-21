#include "./cleanup.h"

#include "../globals/globals.h"

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>

#include <stdlib.h>


InitializingInfo initInfo = {};


int cleanup(InitializingInfo *tInitInfo) {
    initInfo = *tInitInfo;


    vkDestroySurfaceKHR(*initInfo.pInstance, *initInfo.pSurface, NULL);

    vkDestroyInstance(*initInfo.pInstance, NULL);

    vkDestroySwapchainKHR(*initInfo.pDevice, *initInfo.pSwapChain, NULL);

    for (int i = 0; i < initInfo.swapChainFramebuffersCount; i++) {
        vkDestroyFramebuffer(*initInfo.pDevice, *initInfo.pSwapChainFramebuffers[i], NULL);
    }
    for (int i = 0; i < initInfo.swapChainImageViewsCount; i++) {
        vkDestroyImageView(*initInfo.pDevice, *initInfo.pSwapChainImageViews[i], NULL);
    }

    vkDestroyPipeline(*initInfo.pDevice, *initInfo.pGraphicsPipeline, NULL);
    vkDestroyPipelineLayout(*initInfo.pDevice, *initInfo.pPipelineLayout, NULL);
    vkDestroyRenderPass(*initInfo.pDevice, *initInfo.pRenderPass, NULL);

    vkDestroyCommandPool(*initInfo.pDevice, *initInfo.pCommandPool, NULL);

    vkDestroyDevice(*initInfo.pDevice, NULL);


    SDL_DestroyWindow(*initInfo.pWindow);


    *tInitInfo = initInfo;

    return EXIT_SUCCESS;
}