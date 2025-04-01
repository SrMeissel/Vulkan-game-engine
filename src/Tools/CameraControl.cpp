#include "CameraControl.hpp"

namespace editor {
    void moveInPlaneXZ(const Window& engineWindow, ECS::Transform& cameraTransform, float dt, float moveSpeed, float lookSpeed) {
        glm::vec3 rotate{0};

        auto window = engineWindow.getGLFWwindow();

        if(glfwGetKey(window, keyBindings::lookRight) == GLFW_PRESS) rotate.y += 1.0f;
        if(glfwGetKey(window, keyBindings::lookLeft) == GLFW_PRESS) rotate.y -= 1.0f;
        if(glfwGetKey(window, keyBindings::lookUp) == GLFW_PRESS) rotate.x += 1.0f;
        if(glfwGetKey(window, keyBindings::lookDown) == GLFW_PRESS) rotate.x -= 1.0f;


        if(glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()){
            cameraTransform.rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        cameraTransform.rotation.x = glm::clamp(cameraTransform.rotation.x, -1.5f, 1.5f);
        cameraTransform.rotation.y = glm::mod(cameraTransform.rotation.y, glm::two_pi<float>());

        float yaw = cameraTransform.rotation.y;
        float pitch = cameraTransform.rotation.x;
        const glm::vec3 forwardDir{sin(yaw), -tan(pitch), cos(yaw)};

        const glm::vec3 rightDir{forwardDir.z, 0.0f, -forwardDir.x};
        const glm::vec3 upDir{0.0f, -1.0f, 0.0f};

        glm::vec3 moveDir{0.0f};
        if(glfwGetKey(window, keyBindings::moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if(glfwGetKey(window, keyBindings::moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if(glfwGetKey(window, keyBindings::moveRight) == GLFW_PRESS) moveDir += rightDir;
        if(glfwGetKey(window, keyBindings::moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if(glfwGetKey(window, keyBindings::moveUp) == GLFW_PRESS) moveDir += upDir;
        if(glfwGetKey(window, keyBindings::moveDown) == GLFW_PRESS) moveDir -= upDir;

        if(glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()){
            cameraTransform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }
}