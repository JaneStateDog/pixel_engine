#include "./game_loop.h"
#include "../../engine/globals/globals.h"

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>

#include <stdlib.h>
#include <stdbool.h>


InitializingInfo *initInfo;

bool loopRunning = true;


int drawFrame() {
    // Using UINT64_MAX disables the timeout
    vkWaitForFences(initInfo->device, 1, &initInfo->inFlightFences[initInfo->currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(initInfo->device, initInfo->swapChain, UINT64_MAX, initInfo->imageAvailableSemaphores[initInfo->currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (initInfo->imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(initInfo->device, 1, &initInfo->imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    initInfo->imagesInFlight[imageIndex] = initInfo->inFlightFences[initInfo->currentFrame];

    VkSemaphore waitSemaphores[] = { initInfo->imageAvailableSemaphores[initInfo->currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { initInfo->renderFinishedSemaphores[initInfo->currentFrame] };
    
    VkCommandBuffer test[1] = { initInfo->commandBuffers[imageIndex] };

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,

        .commandBufferCount = 1,
        .pCommandBuffers = &test,

        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    vkResetFences(initInfo->device, 1, &initInfo->inFlightFences[initInfo->currentFrame]);

    if (vkQueueSubmit(initInfo->graphicsQueue, 1, &submitInfo, initInfo->inFlightFences[initInfo->currentFrame]) != VK_SUCCESS) { return EXIT_FAILURE; }

    VkSwapchainKHR swapChains[] = { initInfo->swapChain };
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,

        .swapchainCount = 1,
        .pSwapchains = swapChains,

        .pImageIndices = &imageIndex,

        .pResults = NULL
    };

    vkQueuePresentKHR(initInfo->presentQueue, &presentInfo);

    vkQueueWaitIdle(initInfo->presentQueue);

    // By using the modulo operator we ensure that the frame index loops around after every MAX_FRAMES_IN_FLIGHT enqueued frames
    initInfo->currentFrame = (initInfo->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return EXIT_SUCCESS;
}


int gameLoop(InitializingInfo *tInitInfo) {
    initInfo = tInitInfo;


    while (loopRunning) {
        // --- Events ---


        // -- Draw ---
        drawFrame();
    }

    vkDeviceWaitIdle(initInfo->device);

    return EXIT_SUCCESS;
}