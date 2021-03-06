//
// Created by Igor Frank on 17.10.20.
//

#include "spdlog/spdlog.h"
#include "Callbacks.h"

#include "GlfwWindow.h"

GlfwWindow::GlfwWindow(const std::string &windowName, const int width, const int height) {
    this->width = width;
    this->height = height;
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = std::unique_ptr<GLFWwindow, DestroyglfwWin>(glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr));
    glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);
    this->windowName = windowName;
    spdlog::debug(fmt::format("Creating window \"{}\". Size: {}x{}.", windowName, width, height));
}

const Window &GlfwWindow::getWindow() const {
    return window;
}
void GlfwWindow::cleanUp() {
    spdlog::debug("Destroying window.");
    glfwTerminate();
}
GlfwWindow::~GlfwWindow() {
    cleanUp();
}
const std::string &GlfwWindow::getWindowName() const {
    return windowName;
}
int GlfwWindow::getWidth() const {
    return width;
}
int GlfwWindow::getHeight() const {
    return height;
}

void GlfwWindow::setFramebufferCallback(){
    glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);
}
void GlfwWindow::checkMinimized() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window.get(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window.get(), &width, &height);
        glfwWaitEvents();
    }
}
