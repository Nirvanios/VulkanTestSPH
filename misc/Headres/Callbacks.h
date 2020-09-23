//
// Created by Igor Frank on 23.11.19.
//

#ifndef VULKANTEST_CALLBACKS_H
#define VULKANTEST_CALLBACKS_H


#include <Camera.h>
#include <GLFW/glfw3.h>

class Callbacks {
private:
    inline static Camera* camera;
    inline static bool isLeftMousePressed;
    inline static double oldXpos, oldYpos;

public:
    explicit Callbacks(Camera& camera){Callbacks::camera = &camera; isLeftMousePressed = false;};

    static void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    static void mouseMovementCallback(GLFWwindow* window, double xpos, double ypos);
};


#endif //VULKANTEST_CALLBACKS_H
