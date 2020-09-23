//
// Created by Igor Frank on 24.10.19.
//
#include <GraphicsCore.h>
#include <Window.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Callbacks.h>
#include <chrono>
#include <thread>

using namespace std::literals::chrono_literals;

int main() {
    auto window = Window{640,480, GraphicsCore::framebufferResizeCallback};
    auto camera = Camera{glm::vec3(0.0f, 0.f, 10.0f)};
    auto callbacks = Callbacks{camera};
    window.setMouseMovementCallback(Callbacks::mouseMovementCallback);
    window.setKeyCallback(Callbacks::keyboardCallback);
    window.setMouseButtonCallback(Callbacks::mouseButtonCallback);
    window.setScrollCallback(Callbacks::scrollCallback);
    GraphicsCore graphicsCore{640, 640, window, camera};
    std::cout << "Drawing..." << std::endl;

    while (!graphicsCore.windowShouldClose()) {
        glfwPollEvents();
        graphicsCore.drawFrame();
        std::this_thread::sleep_for(0.01s);
    }
    graphicsCore.waitIdle();

}


