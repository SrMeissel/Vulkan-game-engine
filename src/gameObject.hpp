#pragma once

#include "modelManager.hpp"
#include "Components/Physics/PxObject.hpp" 

#include <memory>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>

namespace engine {
    class PhysicsObject;

    struct PointLightComponent {
        float ligthIntensity = 1.0f; 
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

            static GameObject makePointLight(float intensity = 1.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.0f));

            GameObject(const GameObject & ) = delete;
            GameObject &operator=(const GameObject &) = delete;
            GameObject(GameObject &&) = default;
            GameObject &operator=(GameObject &&) = default;

            id_t getId() {return id; }

            glm::vec3 color{};
            TransformComponent transform{};

            //optional components;
            std::shared_ptr<Model> model{};
            int textureIndex = 0;

            std::shared_ptr<PhysicsObject> physics{};
            
            std::unique_ptr<PointLightComponent> pointLight = nullptr;
        
        private:
            GameObject(id_t objId) : id{objId} {}

            id_t id;
    };
}