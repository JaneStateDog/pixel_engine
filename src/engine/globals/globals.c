#include "./globals.h"

#include <vulkan/vulkan.h>

#include <stdio.h>


const char * ENGINE_NAME = "pixel_engine";
const uint32_t ENGINE_VERSION = VK_MAKE_VERSION(1, 0, 0);
const uint32_t VULKAN_API_VERSION = VK_API_VERSION_1_2;

const int MAX_FRAMES_IN_FLIGHT = 2;


bytecodeInfo readShaderBytecode(const char * fileName) {
    FILE *file = fopen(fileName, "rb");

    if (file == NULL) {
        printf("The file failed to load!");
        return (bytecodeInfo){};
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char * buffer = malloc(fileSize * sizeof(char));
    fread(buffer, sizeof(char), fileSize, file);

    if (fclose(file) != 0) {
        printf("The file failed to close!");
        return (bytecodeInfo){};
    }
    return (bytecodeInfo){ .code = buffer, .size = fileSize };
}