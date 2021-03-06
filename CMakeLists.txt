cmake_minimum_required(VERSION 3.17)

include("cmake/HunterGate.cmake")

HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.269.tar.gz"
        SHA1 "64024b7b95b4c86d50ae05b926814448c93a70a0"
)


project(VulkanApp)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts -fconcepts-diagnostics-depth=10")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif ()


find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)
hunter_add_package(imgui)
find_package(imgui CONFIG REQUIRED)
hunter_add_package(fmt)
find_package(fmt CONFIG REQUIRED)
hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)


add_compile_definitions(SPDLOG_FMT_EXTERNAL
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
        VULKAN_HPP_NO_NODISCARD_WARNINGS
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        )

add_executable(VulkanApp main.cpp
        Utilities.h
        vulkan/VulkanCore.cpp vulkan/VulkanCore.h
        window/GlfwWindow.cpp window/GlfwWindow.h vulkan/Instance.cpp vulkan/Instance.h vulkan/Device.cpp vulkan/Device.h vulkan/Swapchain.cpp vulkan/Swapchain.h vulkan/Pipeline.cpp vulkan/Pipeline.h vulkan/Framebuffers.cpp vulkan/Framebuffers.h window/Callbacks.h vulkan/Types.h)
target_link_libraries(VulkanApp
        ${Vulkan_LIBRARIES} glfw spdlog::spdlog imgui::imgui fmt::fmt)