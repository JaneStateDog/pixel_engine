#ifndef GLOBALS
#define GLOBALS

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>

#include <stdint.h>

#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x)) // Stolen from SDL's wiki lol


extern const char * APPLICATION_TITLE;
extern const uint32_t APPLICATION_VERSION;
extern const char * ENGINE_NAME;
extern const uint32_t ENGINE_VERSION;
extern const uint32_t VULKAN_API_VERSION;
extern const int WIN_WIDTH;
extern const int WIN_HEIGHT;

extern const int MAX_FRAMES_IN_FLIGHT;


typedef struct {
    SDL_Window *window;

    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    VkImage *swapChainImages;
    uint32_t swapChainImagesCount;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkImageView *swapChainImageViews;
    uint32_t swapChainImageViewsCount;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkFramebuffer *swapChainFramebuffers;
    uint32_t swapChainFramebuffersCount;

    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;
    uint32_t commandBuffersCount;

    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    uint32_t imageAvailableSemaphoresCount;
    uint32_t renderFinishedSemaphoresCount;

    VkFence *inFlightFences;
    VkFence *imagesInFlight;
    uint32_t inFlightFencesCount;
    uint32_t imagesInFlightCount;

    uint32_t currentFrame;
} InitializingInfo;


typedef struct { 
    char * code;
    long size;
} bytecodeInfo;
bytecodeInfo readShaderBytecode(const char * fileName);


#endif