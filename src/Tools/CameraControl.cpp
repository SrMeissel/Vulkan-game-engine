#include "CameraControl.hpp"
#include "keyMap.h"

namespace editor {
    void moveInPlaneXZ(Window& engineWindow, ECS::Transform& cameraTransform, float dt, float moveSpeed, float lookSpeed) {
        glm::vec3 rotate{0};

        if(engineWindow.isKeyDown(Key::KEY_RIGHT)) rotate.y += 1.0f;
        if(engineWindow.isKeyDown(Key::KEY_LEFT)) rotate.y -= 1.0f;
        if(engineWindow.isKeyDown(Key::KEY_UP)) rotate.x += 1.0f;
        if(engineWindow.isKeyDown(Key::KEY_DOWN)) rotate.x -= 1.0f;


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
        if(engineWindow.isKeyDown(Key::KEY_W)) moveDir += forwardDir;
        if(engineWindow.isKeyDown(Key::KEY_S)) moveDir -= forwardDir;
        if(engineWindow.isKeyDown(Key::KEY_D)) moveDir += rightDir;
        if(engineWindow.isKeyDown(Key::KEY_A)) moveDir -= rightDir;
        if(engineWindow.isKeyDown(Key::KEY_E)) moveDir += upDir;
        if(engineWindow.isKeyDown(Key::KEY_Q)) moveDir -= upDir;

        if(glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()){
            cameraTransform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }
}