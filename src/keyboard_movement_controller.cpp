#include "keyboard_movement_controller.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace engine {
    void keyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, ECS::Transform& cameraTransform) {
        glm::vec3 rotate{0};

        if(glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.0f;
        if(glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.0f;
        if(glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.0f;
        if(glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.0f;


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
        if(glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if(glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if(glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if(glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if(glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if(glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

        if(glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()){
            cameraTransform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }
}