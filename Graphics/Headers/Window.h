//
// Created by Igor Frank on 24.10.19.
//

#ifndef VULKANTEST_WINDOW_H
#define VULKANTEST_WINDOW_H

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

class Window {
public:
    Window(int width, int height, GLFWframebuffersizefun framebufferResizeCallback, GLFWmousebuttonfun mouseButtonCallback = nullptr, GLFWcursorposfun mousePositionCallback = nullptr);

    virtual ~Window();

    GLFWwindow *getWindow() const;

    vk::UniqueSurfaceKHR createSurface(vk::UniqueInstance &instance);

    std::pair<int, int> getFramebufferSize();

    bool windowShouldClose();

    void setKeyCallback(GLFWkeyfun callback = nullptr);

    void setMouseMovementCallback(GLFWcursorposfun callback = nullptr);

    void setMouseButtonCallback(GLFWmousebuttonfun callback = nullptr);

    void setScrollCallback(GLFWscrollfun callback = nullptr);

private:
    GLFWwindow *window{};

    int width, height;

    void initWindow();

};


#endif //VULKANTEST_WINDOW_H
