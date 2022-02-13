cmake_minimum_required(VERSION 3.16)
project(pixel_engine)
set(CMAKE_C_STANDARD 17) # Warning, SDL dies if you do version 23, at least for me for now
set(CMAKE_CXX_STANDARD 20) # I think 23 would be fine but I'm going to stick with what I know works
file(GLOB_RECURSE SOURCES "src/engine/*.c" "src/engine/*.h" "src/application/*.c" "src/application/*.h")


# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Vulkan
find_package(Vulkan REQUIRED)

# cimgui
set(cimgui third_party/cimgui)
set(imgui_impl third_party/cimgui/imgui/backends)
include_directories(${cimgui} ${cimgui}/imgui ${imgui_impl})
add_subdirectory(${cimgui})
add_library(imgui_impl STATIC ${imgui_impl}/imgui_impl_opengl3.cpp ${imgui_impl}/imgui_impl_sdl.cpp)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GL3W)
target_compile_definitions(imgui_impl PRIVATE IMGUI_IMPL_API=extern\ \"C\")

# stb_image
include_directories(third_party)


add_executable(pixel_engine ${SOURCES})

# SDL2, Vulkan, cimgui, cglm
target_link_libraries(pixel_engine ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES} cimgui imgui_impl cglm_headers)

# cglm
add_subdirectory(third_party/cglm EXCLUDE_FROM_ALL)