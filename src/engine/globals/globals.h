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


typedef struct {
    SDL_Window **pWindow;

    VkInstance *pInstance;
    VkSurfaceKHR *pSurface;
    VkPhysicalDevice *pPhysicalDevice;
    VkDevice *pDevice;

    VkQueue *pGraphicsQueue;
    VkQueue *pPresentQueue;

    VkSwapchainKHR *pSwapChain;
    VkImage **pSwapChainImages;
    VkFormat *pSwapChainImageFormat;
    VkExtent2D *pSwapChainExtent;
    VkImageView **pSwapChainImageViews;
    uint32_t swapChainImageViewsCount;

    VkRenderPass *pRenderPass;
    VkPipelineLayout *pPipelineLayout;
    VkPipeline *pGraphicsPipeline;

    VkFramebuffer **pSwapChainFramebuffers;
    uint32_t swapChainFramebuffersCount;

    VkCommandPool *pCommandPool;
    VkCommandBuffer **pCommandBuffers;
    uint32_t commandBuffersCount;
} InitializingInfo;


typedef struct { 
    char * code;
    long size;
} bytecodeInfo;
bytecodeInfo readShaderBytecode(const char * fileName);


#endif