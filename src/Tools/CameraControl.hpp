#pragma once

#include "Pipeline/windowManager.hpp"
#include "ECS/Components.hpp"

namespace editor {

    enum keyBindings {
        moveLeft = GLFW_KEY_A,
        moveRight = GLFW_KEY_D,
        moveForward = GLFW_KEY_W,
        moveBackward = GLFW_KEY_S,
        moveUp = GLFW_KEY_E,
        moveDown = GLFW_KEY_Q,
        lookLeft = GLFW_KEY_LEFT,
        lookRight = GLFW_KEY_RIGHT,
        lookUp = GLFW_KEY_UP,
        lookDown = GLFW_KEY_DOWN
    };

    void moveInPlaneXZ(const Window& engineWindow, ECS::Transform& cameraTransform, float dt, float moveSpeed, float lookSpeed);
}