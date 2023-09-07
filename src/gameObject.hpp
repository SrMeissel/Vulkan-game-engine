#pragma once

#include "modelManager.hpp"

#include <memory>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>

namespace engine {

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation;

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        // https://www.youtube.com/watch?v=0X_kRtyVzm4&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=14&t=289s
        glm::mat4 mat4();
        glm::mat3 normalMatrix();

    };

    class GameObject {
        public:
            GameObject();

            using id_t = unsigned int;
            using map = std::unordered_map<id_t, GameObject>;

            static GameObject createGameObject() {
                static id_t currentId = 0;
                return GameObject{currentId++};
            }

            GameObject(const GameObject & ) = delete;
            GameObject &operator=(const GameObject &) = delete;
            GameObject(GameObject &&) = default;
            GameObject &operator=(GameObject &&) = default;

            id_t getId() {return id; }

            std::shared_ptr<Model> model{};
            glm::vec3 color{};
            TransformComponent transform{};
        
        private:
            GameObject(id_t objId) : id{objId} {}

            id_t id;
    };
}