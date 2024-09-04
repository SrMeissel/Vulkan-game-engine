#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace engine { 
    class CameraManager { 
        public:

            glm::mat4 setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

            glm::mat4 setPerspectiveProjection(float fovy, float aspect, float near, float far);
            glm::mat4 setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
            glm::mat4 setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
            glm::mat4 setViewYXZ(glm::vec3 position, glm::vec3 rotation);
    };
}