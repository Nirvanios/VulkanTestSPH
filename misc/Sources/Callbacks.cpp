//
// Created by Igor Frank on 23.11.19.
//

#include <Callbacks.h>
#include <iostream>
#include "glm/gtx/string_cast.hpp"

void Callbacks::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        isLeftMousePressed = true;
        glfwGetCursorPos(window, &oldXpos, &oldYpos);
    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        isLeftMousePressed = false;
}

void Callbacks::mouseMovementCallback(GLFWwindow *window, double xpos, double ypos) {
    if(isLeftMousePressed){
        auto xoffset = oldXpos - xpos;
        auto yoffset = -(oldYpos - ypos);
        oldYpos = ypos;
        oldXpos = xpos;
        camera->ProcessMouseMovement(xoffset, yoffset);
    }
}

void Callbacks::keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if((key == GLFW_KEY_W || key == GLFW_KEY_UP) && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        camera->ProcessKeyboard(Camera_Movement::FORWARD, 0.1);
    }
    else if((key == GLFW_KEY_S || key == GLFW_KEY_DOWN) && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        camera->ProcessKeyboard(Camera_Movement::BACKWARD, 0.1);
    }
    else if((key == GLFW_KEY_A || key == GLFW_KEY_LEFT) && (action == GLFW_PRESS || action == GLFW_REPEAT)){
        camera->ProcessKeyboard(Camera_Movement::LEFT, 0.1);
    }
    else if((key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        camera->ProcessKeyboard(Camera_Movement::RIGHT, 0.1);
    }
}

void Callbacks::scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

