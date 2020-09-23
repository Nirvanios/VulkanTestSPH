cmake_minimum_required(VERSION 3.14)
project(VulkanTest)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
message(${Vulkan_INCLUDE_DIRS})
set(VK_LAYER_PATH "/etc/vulkan/explicit_layer.d")
set(LD_LIBRARY_PATH "/usr/lib")
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
INCLUDE_DIRECTORIES(${GLFW_INCLUDE_DIRS})

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

#add_compile_options(-O3)

include_directories(Objects/Headers)
include_directories(misc/Headres)
include_directories(Graphics/Headers)
include_directories(lib)

FILE(GLOB ObjectsSources Objects/Source/*.cpp)
FILE(GLOB ObjectsHeaders Objects/Headers/*.h)

FILE(GLOB GraphicsSources Graphics/Source/*.cpp)
FILE(GLOB GraphicsHeaders Graphics/Headers/*.h)

FILE(GLOB MiscHeaders misc/Headres/*.h)
FILE(GLOB MiscSources misc/Sources/*.cpp)

FILE(GLOB ThirdPartyLibs libs/*.h)

set(SOURCE_FILES
        ${ObjectsSources}
        ${ObjectsHeaders}
        ${ThirdParyLibs}
        ${MiscHeaders}
        ${MiscSources}
        ${GraphicsHeadres}
        ${GraphicsSources})

add_compile_definitions(GLFW_INCLUDE_VULKAN
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_ENABLE_EXPERIMENTAL
        GLM_FORCE_SWIZZLE
        STB_IMAGE_IMPLEMENTATION
        TINYOBJLOADER_IMPLEMENTATION
        VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE)

add_executable(VulkanTest main.cpp ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${GLFW_STATIC_LIBRARIES} ${GLM_LIBRARIES} ${Vulkan_LIBRARIES})
add_executable(VulkanTest2 VulkanTest.cpp ${SOURCE_FILES})
target_link_libraries(VulkanTest2 ${GLFW_STATIC_LIBRARIES} ${GLM_LIBRARIES} ${Vulkan_LIBRARIES})