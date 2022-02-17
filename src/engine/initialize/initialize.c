#include "./initialize.h"

#include "../globals/globals.h"

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <string.h>


#define DO_VALIDATION_LAYERS true
#define VALIDATION_LAYERS_COUNT 1
const char * validationLayers[VALIDATION_LAYERS_COUNT] = {
    "VK_LAYER_KHRONOS_validation"
};

#define REQUIRED_EXTENSIONS_COUNT 1
const char * requiredExtensions[REQUIRED_EXTENSIONS_COUNT] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


InitializingInfo initInfo = {};


int initWindow() {
    SDL_Init(SDL_INIT_VIDEO);
    *(initInfo.pWindow) = SDL_CreateWindow(APPLICATION_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_VULKAN);
    if (*(initInfo.pWindow) == NULL) { return EXIT_FAILURE; }

    return EXIT_SUCCESS;
}


bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (int i = 0; i < VALIDATION_LAYERS_COUNT; i++) {
        bool layerFound = false;

        for (int b = 0; b < layerCount; b++) {
            if (strcmp(validationLayers[i], availableLayers[b].layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) { return false; }
    }

    return true;
}

int createInstance() {
    // Check validation layer support
    if (DO_VALIDATION_LAYERS && !checkValidationLayerSupport()) { return EXIT_FAILURE; }

    // Define application info
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,

        .pApplicationName = APPLICATION_TITLE,
        .applicationVersion = APPLICATION_VERSION,

        .pEngineName = ENGINE_NAME,
        .engineVersion = ENGINE_VERSION,

        .apiVersion = VULKAN_API_VERSION
    };

    // In the future if we need to add more extensions, we should rewrite this section of code to account for that
    // -- Get the extensions SDL needs
    unsigned int SDLExtensionsCount = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(*(initInfo.pWindow), &SDLExtensionsCount, NULL)) { return EXIT_FAILURE; }
    const char * *SDLExtenions = malloc(SDLExtensionsCount * sizeof(const char *));
    if (!SDL_Vulkan_GetInstanceExtensions(*(initInfo.pWindow), &SDLExtensionsCount, SDLExtenions)) { return EXIT_FAILURE; }
    // --

    // Create the vulkan instance
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,

        .pApplicationInfo = &appInfo,

        .enabledExtensionCount = SDLExtensionsCount,
        .ppEnabledExtensionNames = SDLExtenions,

        #if DO_VALIDATION_LAYERS == true
            .enabledLayerCount = VALIDATION_LAYERS_COUNT,
            .ppEnabledLayerNames = validationLayers
        #else
            .enabledLayerCount = 0
        #endif
    };

    if (vkCreateInstance(&createInfo, NULL, initInfo.pInstance) != VK_SUCCESS) { return EXIT_FAILURE; }

    return EXIT_SUCCESS;
}

int createSurface() {
    if (!SDL_Vulkan_CreateSurface(*initInfo.pWindow, *initInfo.pInstance, initInfo.pSurface)) { return EXIT_FAILURE; }



    return EXIT_SUCCESS;
}

typedef struct {
    uint32_t graphicsFamily;
    bool foundGraphicsFamily;

    uint32_t presentFamily;
    bool foundPresentFamily;
} QueueFamilyIndices;

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    indices.foundGraphicsFamily = false;
    indices.foundPresentFamily = false;

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { 
            indices.graphicsFamily = i; 
            indices.foundGraphicsFamily = true;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *initInfo.pSurface, &presentSupport);
        if (presentSupport) { 
            indices.presentFamily = i; 
            indices.foundPresentFamily = true;
        }

        // If we found all the queue families we need, then we can break
        if (
            indices.foundGraphicsFamily && indices.foundPresentFamily
        ) { break; }

        i++;
    }

    return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    VkExtensionProperties availibleExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availibleExtensions);

    for (int i = 0; i < REQUIRED_EXTENSIONS_COUNT; i++) {
        bool found = false;

        for (int b = 0; b < extensionCount; b++) {
            if (strcmp(requiredExtensions[i], availibleExtensions[b].extensionName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) { return false; }
    }

    return true;
}

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    uint32_t formatCount;
    VkPresentModeKHR *presentModes;
    uint32_t presentModeCount;
} SwapChainSupportDetails;

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, *initInfo.pSurface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, *initInfo.pSurface, &formatCount, NULL);
    if (formatCount > 0) {
        details.formats = malloc(formatCount * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, *initInfo.pSurface, &formatCount, details.formats);
    }
    details.formatCount = formatCount;

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, *initInfo.pSurface, &presentModeCount, NULL);
    if (presentModeCount > 0) {
        details.presentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, *initInfo.pSurface, &presentModeCount, details.presentModes);
    }
    details.presentModeCount = presentModeCount;

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *formats, const uint32_t formatCount) {
    // This function looks at the availible formats and aims to use the format VK_FORMAT_B8G8R8A8_SRGB and colorspace VK_COLOR_SPACE_SRGB_NONLINEAR_KHR if it can
    // If it can't, it uses the first availible option
    for (int i = 0; i < formatCount; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && 
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            
            return formats[i];
        }
    }

    return formats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR *presentModes, const uint32_t presentModeCount) {
    // This function looks at the availible present modes and aims to use VK_PRESENT_MODE_MAILBOX_KHR if it can
    // If it can't, it uses VK_PRESENT_MODE_FIFO_KHR since that will always be an option
    for (int i = 0; i < presentModeCount; i++) {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) { return presentModes[i]; }
    }
    
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        SDL_Vulkan_GetDrawableSize(*initInfo.pWindow, &width, &height);

        VkExtent2D actualExtent = { 
            .width = width, 
            .height = height 
        };
        actualExtent.width = CLAMP(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = CLAMP(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }
}

int rateDeviceSuitability(VkPhysicalDevice device) {
    int score = 0;

    VkPhysicalDeviceProperties deviceProperities;
    vkGetPhysicalDeviceProperties(device, &deviceProperities);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // Right now we don't need anything special beyond a graphics card that has Vulkan support and some queue families and such, so we can mostly skip this function
    // In the future we can use this to set up a scoring system
    QueueFamilyIndices indices = findQueueFamilies(device);
    if (indices.foundGraphicsFamily) { score += 1000; }
    if (indices.foundPresentFamily) { score += 1000; }

    if (checkDeviceExtensionSupport(device)) { 
        score += 1000; 
    
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        if (swapChainSupport.formatCount > 0 && swapChainSupport.presentModeCount > 0) {
            score += 1000;
        }
    }

    return score;
}

int pickPhysicalDevice() {
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(*initInfo.pInstance, &deviceCount, NULL);
    if (deviceCount == 0) { return EXIT_FAILURE; }
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(*initInfo.pInstance, &deviceCount, devices);

    for (int i = 0; i < deviceCount; i++) {
        if (rateDeviceSuitability(devices[i]) >= 4000) {
            *initInfo.pPhysicalDevice = devices[i];
            break;
        }
    }

    if (*initInfo.pPhysicalDevice == VK_NULL_HANDLE) { return EXIT_FAILURE; }

    return EXIT_SUCCESS;
}

int createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(*initInfo.pPhysicalDevice);

    const uint32_t possibleQueueFamilies[] = {indices.graphicsFamily, indices.presentFamily};
    const uint32_t size = sizeof(possibleQueueFamilies) / sizeof(const uint32_t);

    int uniqueQueueFamilies[size];
    for (int i = 0; i < size; i++) { uniqueQueueFamilies[i] = -1; }

    uint32_t amount = 0;
    for (int i = 0; i < size; i++) { 
        for (int b = 0; b < size; b++) {
            if (uniqueQueueFamilies[b] == possibleQueueFamilies[i]) { break; } 
            else if (uniqueQueueFamilies[b] == -1) {
                uniqueQueueFamilies[b] = possibleQueueFamilies[i];
                amount++;

                break;
            }
        }
    }

    uint32_t queueCreateInfosCount = amount;
    VkDeviceQueueCreateInfo queueCreateInfos[queueCreateInfosCount];

    float queuePriority = 1.0f;
    int b = 0;
    for (int i = 0; i < size; i++) {
        if (uniqueQueueFamilies[i] != -1) {
            VkDeviceQueueCreateInfo queueCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,

                .queueFamilyIndex = uniqueQueueFamilies[i],
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            };

            queueCreateInfos[b] = queueCreateInfo;
            b++;
        }
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,

        .queueCreateInfoCount = queueCreateInfosCount,
        .pQueueCreateInfos = queueCreateInfos,

        .pEnabledFeatures = &deviceFeatures,

        .enabledExtensionCount = REQUIRED_EXTENSIONS_COUNT,
        .ppEnabledExtensionNames = requiredExtensions,

        // These few lines are not needed in the latest implementations of vulkan, so they're here for compatibility sake
        #if DO_VALIDATION_LAYERS == true
            .enabledLayerCount = VALIDATION_LAYERS_COUNT,
            .ppEnabledLayerNames = validationLayers
        #else
            .enabledLayerCount = 0
        #endif
    };

    if (vkCreateDevice(*initInfo.pPhysicalDevice, &createInfo, NULL, initInfo.pDevice) != VK_SUCCESS) { return EXIT_FAILURE; }

    vkGetDeviceQueue(*initInfo.pDevice, indices.graphicsFamily, 0, initInfo.pGraphicsQueue);
    vkGetDeviceQueue(*initInfo.pDevice, indices.presentFamily, 0, initInfo.pPresentQueue);

    return EXIT_SUCCESS;
}

int createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(*initInfo.pPhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModeCount);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // Sticking to the minimum means we may sometimes have to wait on the driver to complete
    // its internal operations before we can get another image to render to
    // Therefore it is usually recommended to request at least one more image than the minimum, so we are doing that here
    // Also confirm we didn't exceed the maximum number of images while doing so
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        
        .surface = *initInfo.pSurface,

        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,

        .presentMode = presentMode,

        .imageArrayLayers = 1, // This is always 1 unless we are developing a steroscopic 3D application, which god no I hope we are not doing ever
        // This defines how we want to use the swap chain, which for now is only for attaching colors to the screen. In the future if we want something like postprocessing VK_IMAGE_USAGE_TRANSGER_DST_BIT could be good
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

        // We can use transforms such as flipping 90 degrees clockwise (or etc.), but right now we just want to use the default no transform
        .preTransform = swapChainSupport.capabilities.currentTransform,
        // This can allow us to have the alpha channel blend with other windows in the window system, which we will almost never want to do, so we will just use this option
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        // This means we don't care about the color of pixels that are obscured behind something like a window that is in front of them
        // Unless we really need to read these pixels back, we will get the best performance by enabling clipping
        .clipped = VK_TRUE,

        // A swap chain can become invalid or unoptimized while the application is running if, for example,
        // a window is resized
        // In that case the swap chain will need to be recreated from scratch and a reference to the old one needs to be specified in this field
        // This is a complex topic I know nothing about yet, so I'm ignoring it for now lol
        .oldSwapchain = VK_NULL_HANDLE
    };

    // We need to know if we will use EXCLUSIVE mode or CONCURRENT mode for swap chain images that are being used across multiple queue families
    // For now, we will use CONCURRENT if the graphics family is NOT the same as the present family
    QueueFamilyIndices indices = findQueueFamilies(*initInfo.pPhysicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    if (vkCreateSwapchainKHR(*initInfo.pDevice, &createInfo, NULL, initInfo.pSwapChain) != VK_SUCCESS) { return EXIT_FAILURE; }

    vkGetSwapchainImagesKHR(*initInfo.pDevice, *initInfo.pSwapChain, &imageCount, NULL);
    *initInfo.pSwapChainImages = malloc(imageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(*initInfo.pDevice, *initInfo.pSwapChain, &imageCount, *initInfo.pSwapChainImages);

    *initInfo.pSwapChainImageFormat = surfaceFormat.format;
    *initInfo.pSwapChainExtent = extent;

    return EXIT_SUCCESS;
}

int createImageViews() {
    initInfo.swapChainImageViewsCount = sizeof(*initInfo.pSwapChainImages) / sizeof(VkImage);
    *initInfo.pSwapChainImageViews = malloc(initInfo.swapChainImageViewsCount * sizeof(VkImageView));

    for (int i = 0; i < initInfo.swapChainImageViewsCount; i++) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,

            .image = *initInfo.pSwapChainImages[i],

            .viewType = VK_IMAGE_TYPE_2D,
            .format = *initInfo.pSwapChainImageFormat,

            // This lets us swizzle the color channels around. For example, if we mapped all of the channels to the red channel, we'd have a monochrome texture
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,

            // This lets us describe what the image's purpose is and which part of the image should be accessed
            // Our images will be used as color targets without any mipmapping levels or multiple layers, so we use this for now
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };

        if (vkCreateImageView(*initInfo.pDevice, &createInfo, NULL, initInfo.pSwapChainImageViews[i]) != VK_SUCCESS) { return EXIT_FAILURE; }
    }

    return EXIT_SUCCESS;
}

VkShaderModule createShaderModule(bytecodeInfo info) {
    uint32_t *code;
    memcpy(&code, info.code, info.size); // I really hope this works and it doesn't become a huge issue

    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,

        .codeSize = info.size,
        .pCode = code
    };

    VkShaderModule shaderModule;
    vkCreateShaderModule(*initInfo.pDevice, &createInfo, NULL, &shaderModule);

    free(info.code);

    return shaderModule;
}

int createGraphicsPipeline() {
    VkShaderModule vertShaderModule = createShaderModule(readShaderBytecode("src/engine/shaders/compiled/vert.spv"));
    VkShaderModule fragShaderModule = createShaderModule(readShaderBytecode("src/engine/shaders/compiled/frag.spv"));

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,

        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main"
    };

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,

        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main"
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderModule
    };

    vkDestroyShaderModule(*initInfo.pDevice, vertShaderModule, NULL);
    vkDestroyShaderModule(*initInfo.pDevice, fragShaderModule, NULL);


    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,

        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = NULL,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = NULL
    };


    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,

        /*
        - VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
        - VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse
        - VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used as start vertex for the next line
        - VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
        - VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangleVK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
        - VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse
        - VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used as start vertex for the next line
        - VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
        - VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle
        */
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        // If we set this to true then it's possible to break up lines and triangles in the _STRIP topology modes by using a special index of 0xFFFF or 0xFFFFFFFF
        .primitiveRestartEnable = VK_FALSE
    };


    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,

        .width = initInfo.pSwapChainExtent->width,
        .height = initInfo.pSwapChainExtent->height,

        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .offset = { 0, 0 },
        .extent = *initInfo.pSwapChainExtent
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,

        .viewportCount = 1,
        .pViewports = &viewport,

        .scissorCount = 1,
        .pScissors = &scissor
    };


    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,

        // If set to true then the fragments that are beyond the near and far planes are clamped to them as opposed to ignoring them
        .depthClampEnable = VK_FALSE,
        // If set to true then geometry never passes through the rasterizer stage. This pretty much disables any output to the framebuffer
        .rasterizerDiscardEnable = VK_FALSE,

        /*
        - VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
        - VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
        - VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
        */
        .polygonMode = VK_POLYGON_MODE_FILL,
        
        .lineWidth = 1.0f,

        // Determines the type of face culling to use. You can disable culling, cull the front faces, cull the back faces, or both
        .cullMode = VK_CULL_MODE_BACK_BIT,
        // Specifies the vertex order for faces to be considered front-facing and can be clockwise or counterclockwise
        .frontFace = VK_FRONT_FACE_CLOCKWISE,

        // The rasteruzer can alter the depth values by adding a constant value or biasing them based on a frament's slope. This is sometimes used for shadow mapping, but we're not using it for now
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f
    };


    // Multisampling is one of the ways to preform anti-aliasing. Enabling it requires enabling a GPU feature. For now we will leave it disabled and touch it later
    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,

        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };


    // Insert VkPipelineDepthStencilStateCreateInfo stuff here later


    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,

        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,

        .blendConstants[0] = 0.0f,
        .blendConstants[1] = 0.0f,
        .blendConstants[2] = 0.0f,
        .blendConstants[3] = 0.0f
    };


    // Here we set some dynamic states. Some of the data we've put into all the structs in this function can be changed without recreating the pipeline
    // This dynamic state create info struct specifies which states we can change without recreating the pipeline
    // This will cause the configuration of these values to be ignored and we will be required to specify the data at drawing time
    // I believe for now we want to ignore this, but maybe we'll touch on it later
    /*
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,

        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates
    };
    */


    // We can use uniform values in shaders, which are globals similar to the dynamic state variables, that can be changed at drawing time to alter the behavior of our shaders without having to recreate them
    // They are commonly used to pass the transformation matrix to the vertex shader or to create texture samplers in the fragment shader
    // We need to specify these uniform values during pipeline creation
    // Even though we are not going to use them for now, it's still required that we at least create an empty pipeline layout (a pipeline layout being what specifies the uniform values)
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,

        .setLayoutCount = 0,
        .pSetLayouts = NULL,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL
    };

    if (vkCreatePipelineLayout(*initInfo.pDevice, &pipelineLayoutInfo, NULL, initInfo.pPipelineLayout) != VK_SUCCESS) { return EXIT_FAILURE; }


    //vkDestroyShaderModule(*initInfo.pDevice, vertShaderModule, NULL);
    //vkDestroyShaderModule(*initInfo.pDevice, fragShaderModule, NULL);

    return EXIT_SUCCESS;
}

int initVulkan() {
    if (createInstance() == EXIT_FAILURE) { return EXIT_FAILURE; }
    if (createSurface() == EXIT_FAILURE) { return EXIT_FAILURE; }
    if (pickPhysicalDevice() == EXIT_FAILURE) { return EXIT_FAILURE; }
    if (createLogicalDevice() == EXIT_FAILURE) { return EXIT_FAILURE; }
    if (createSwapChain() == EXIT_FAILURE) { return EXIT_FAILURE; }
    if (createImageViews() == EXIT_FAILURE) { return EXIT_FAILURE; }
    if (createGraphicsPipeline() == EXIT_FAILURE) { return EXIT_FAILURE; }

    return EXIT_SUCCESS;
}


int initialize(InitializingInfo tInitInfo) {
    initInfo = tInitInfo;

    if (initWindow() == EXIT_FAILURE) { return EXIT_FAILURE; }
    if (initVulkan() == EXIT_FAILURE) { return EXIT_FAILURE; }

    return EXIT_SUCCESS;
}