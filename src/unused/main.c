#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h> // Gives us NULL. We get it from other places (such as stdlib) too but I think it's good practice to directly get what we need
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan.h>


#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

#define ENABLE_VALIDATION_LAYERS true

#define GPU_ERROR_MESSAGE "Your GPU does not meet the requirements of this application! This application cannot run"


bool running = true;

int main() {
    // ---Init---
    // SDL init and window creation
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("pixel-engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_VULKAN);

    // Set up validation layers
    const char * const validationLayers[1] = {"VK_LAYER_KHRONOS_validation"};
    uint32_t validationLayerCount = sizeof(validationLayers) / sizeof(const char * const);
    
    // Get availible validation layers
    uint32_t availibleLayersCount = 0;
    vkEnumerateInstanceLayerProperties(&availibleLayersCount, NULL);
    VkLayerProperties *availibleLayers = malloc(availibleLayersCount * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&availibleLayersCount, availibleLayers);

    // Check that the validation layers we plan to use are availible
    bool allLayersAvailible = false;
    for (int i = 0; i < validationLayerCount; i++) {
        for (int b = 0; b < availibleLayersCount; b++) {
            if (strcmp(validationLayers[i], availibleLayers[b].layerName) == 0) { // strcmp returns 0 when both strings are equal and other things to show other values. The point is, strcmp doesn't output a boolean, hence why we don't use true or false
                allLayersAvailible = true;
                break;
            }
        }

        if (!allLayersAvailible) { break; }
    }

    // Set vulkan application info
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = "pixel-engine testing application",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
        .pEngineName = "pixel-engine",
        .engineVersion = VK_MAKE_VERSION(0, 0, 0),
        .apiVersion = VK_API_VERSION_1_2
    };

    // Set vulkan instance create info
    uint32_t SDLExtensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &SDLExtensionCount, NULL);
    const char **SDLExtensions = malloc(SDLExtensionCount * sizeof(const char *));
    SDL_Vulkan_GetInstanceExtensions(window, &SDLExtensionCount, SDLExtensions);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .enabledExtensionCount = SDLExtensionCount,
        .ppEnabledExtensionNames = SDLExtensions
    };

    if (allLayersAvailible) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        printf("Certain selected validation layers were not availible");
    }

    // Create vulkan instance
    VkInstance instance;
    vkCreateInstance(&createInfo, NULL, &instance);

    // Create the surface to draw on
    VkSurfaceKHR surface;
    SDL_Vulkan_CreateSurface(window, instance, &surface);

    // Get all physical devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        printf("Failed to find and GPUs with ulkan support! This application cannot run");
        return 0;
    }
    VkPhysicalDevice *devices = malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    // Pick the physical device we want to use
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32_t *deviceScores = malloc(deviceCount * sizeof(uint32_t));

    for (int i = 0; i < deviceCount; i++) {
        deviceScores[i] = 0;
        // Initalize the entries at 0 otherwise we'd be adding to junk

        // Get the device's properties
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);

        // Get the device's features
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(devices[i], &deviceFeatures);

        // Give the device a score based on if it meets our requirements
        // Discrete GPUs have a very large performance advantage, so we give a lot of points for it
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { deviceScores[i] += 1000; }
    
        // Maximum possible size of textures will affect the graphics quality
        deviceScores[i] += deviceProperties.limits.maxImageDimension2D;

        // For now we don't have anything specific we need, so this scoring system is barebones and doesn't do much
        // At least it's here so maybe in the future when it's more helpful we can use it
    }

    int highestScore = 0;
    int highestScoreHolder = 0;
    for (int i = 0; i < deviceCount; i++) {
        if (deviceScores[i] > highestScore) { 
            highestScore = deviceScores[i];
            int highestScoreHolder = i;
        }
    }

    physicalDevice = devices[highestScoreHolder];

    if (physicalDevice == VK_NULL_HANDLE) {
        printf(GPU_ERROR_MESSAGE);
        return 0;
    }

    // Find the device's queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties *queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

    uint32_t graphicsFamilyIndex = 0;
    bool foundGraphicsFamily = false; // We have this because we don't have a way to track if graphicsFamilyIndex is set or not.
    // If it's never set, it'll default to 0 which could be a different queue family that we don't want, which would break things. 
    // So, we use this bool to keep track of if it was set or not
    uint32_t presentFamilyIndex = 0;
    bool foundPresentFamily = false;

    bool foundNeededQueueFamilies = false;
    for (int i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { 
            graphicsFamilyIndex = i;
            foundGraphicsFamily = true;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport) { presentFamilyIndex = i; foundPresentFamily = true; }

        // If all needed queue families are found, then we are done here
        if (
            foundGraphicsFamily && foundPresentFamily
        ) { foundNeededQueueFamilies = true; break; }
    }

    if (!foundNeededQueueFamilies) {
        printf(GPU_ERROR_MESSAGE);
        return 0;
    }

    // Check the physical device's extension support and confirm it has what we need
    const char * const neededDeviceExtensions[1] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    uint32_t neededDeviceExtensionCount = sizeof(neededDeviceExtensions) / sizeof(const char * const);

    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionCount, NULL);
    VkExtensionProperties *deviceExtensions = malloc(deviceExtensionCount * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &deviceExtensionCount, deviceExtensions);

    bool allExtensionsAvailible = false;
    for (int i = 0; i < neededDeviceExtensionCount; i++) {
        for (int b = 0; b < deviceExtensionCount; b++) {
            if (strcmp(neededDeviceExtensions[i], deviceExtensions[b].extensionName) == 0) {
                allExtensionsAvailible = true;
                break;
            }
        }
        if (!allExtensionsAvailible) { 
            printf(GPU_ERROR_MESSAGE);
            return 0;
        }
    }

    // Create the logical device
    VkDevice logicalDevice;

    uint32_t usedQueueFamilyCount = 2;
    VkDeviceQueueCreateInfo *queueCreateInfos = malloc(usedQueueFamilyCount * sizeof(VkDeviceQueueCreateInfo));
    
    uint32_t *uniqueQueueFamilies = malloc(usedQueueFamilyCount * sizeof(uint32_t));
    uniqueQueueFamilies[0] = graphicsFamilyIndex;
    uniqueQueueFamilies[1] = presentFamilyIndex;

    float queuePriority = 1.0f; // Use the same priority for all queues for now
    for (int i = 0; i < usedQueueFamilyCount; i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = uniqueQueueFamilies[i],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {

    }; // Right now we don't need anything fancy, so we can just leave this blank and add stuff later

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfos,
        .queueCreateInfoCount = usedQueueFamilyCount,
        .pEnabledFeatures = &deviceFeatures,
        .enabledExtensionCount = neededDeviceExtensionCount,
        .ppEnabledExtensionNames = neededDeviceExtensions
    };
    // These bits of code are commented out because they are not needed
    // They would normally exist for compatibility with older implementations of Vulkan,
    // but I'm not sure if that applies in my case since I never plan to use an older version of Vulkan
    // It's possible what I read this in meant older implementations of Vulkan hardware, i.e. older graphics cards, 
    // and in that case I would want to re-enable this code to support more cards, hence why it is just commented out for now
    //if (ENABLE_VALIDATION_LAYERS) {
        //deviceCreateInfo.enabledLayerCount = validationLayerCount;
        //deviceCreateInfo.ppEnabledLayerNames = validationLayers;
    //} else { deviceCreateInfo.enabledLayerCount = 0; }

    vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &logicalDevice);

    // Get the queues
    VkQueue graphicsQueue;
    vkGetDeviceQueue(logicalDevice, graphicsFamilyIndex, 0, &graphicsQueue); 
    // The queue index is set to 0 because we're only creating a single queue for this family for now

    VkQueue presentQueue;
    vkGetDeviceQueue(logicalDevice, presentFamilyIndex, 0, &presentQueue);


    // ---Main loop---
    while (running) {
        // ---Events---
        // Get our keyboard's state
        SDL_PumpEvents(); // Make sure we've gathered all inputs first so we have the keyboard's latest state
        const Uint8 *keyState = SDL_GetKeyboardState(NULL);

        // Run all events in the event queue
        SDL_Event event;
        while (SDL_PollEvent(&event)) { switch (event.type) {

            case SDL_QUIT: 
                running = false; break;

        } }


        // ---Drawing---

    }


    // ---Cleanup---
    // Vulkan destruction
    vkDestroySurfaceKHR(instance, surface, NULL);
    // Surfaces need to be destroyed before the instance is because we need the instance to destroy the surface
    vkDestroyInstance(instance, NULL);
    vkDestroyDevice(logicalDevice, NULL);
    
    // SDL destruction
    SDL_DestroyWindow(window);

    // I know it's a cleanup and not violent like a destruction, but destruction sounds cooler

    return 0;
}