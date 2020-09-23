//
// Created by Igor Frank on 24.10.19.
//

#include <stdexcept>
#include <iostream>
#include "../Headers/Window.h"


void Window::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, "SPH", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
}

Window::Window(int width, int height, GLFWframebuffersizefun framebufferResizeCallback, GLFWmousebuttonfun mouseButtonCallback, GLFWcursorposfun mousePositionCallback) : width(width), height(height){
    initWindow();
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    if(mouseButtonCallback != nullptr)
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
    if(mousePositionCallback != nullptr)
        glfwSetCursorPosCallback(window, mousePositionCallback);
}

GLFWwindow *Window::getWindow() const {
    return window;
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

vk::UniqueSurfaceKHR Window::createSurface(vk::UniqueInstance &instance) {
    VkSurfaceKHR tmpSurface;
    VkResult err = glfwCreateWindowSurface(instance.get(), window, nullptr, &tmpSurface);
    if (err) {
        std::cout <<  "Window surface creation failed" << std::endl;
    }
    vk::ObjectDestroy<vk::Instance, vk::DispatchLoaderStatic> surfaceDeleter(instance.get());
    return vk::UniqueSurfaceKHR(tmpSurface, surfaceDeleter);
}

std::pair<int, int> Window::getFramebufferSize() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return std::make_pair(width, height);
}

bool Window::windowShouldClose() {
    return glfwWindowShouldClose(window);
}

void Window::setKeyCallback(GLFWkeyfun callback) {
    glfwSetKeyCallback(window, callback);
}

void Window::setMouseMovementCallback(GLFWcursorposfun callback) {
    glfwSetCursorPosCallback(window, callback);
}

void Window::setScrollCallback(GLFWscrollfun callback) {
    glfwSetScrollCallback(window, callback);
}

void Window::setMouseButtonCallback(GLFWmousebuttonfun callback) {
    glfwSetMouseButtonCallback(window, callback);
}
