#pragma once

//#include "collisionSolver.hpp"
#include "../../gameObject.hpp"

#include <glm/glm.hpp>
#include <memory>

namespace engine {
    class GameObject;
    
    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        // https://www.youtube.com/watch?v=0X_kRtyVzm4&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=14&t=289s
        glm::mat4 mat4();
        glm::mat3 normalMatrix();

    };

    struct DynamicsComponent {
            float mass = 1;
            glm::vec3 COM;

            glm::vec3 translationalVelocity{0.0f};
            glm::vec3 translationalAcceleration{0.0f};
            
            glm::vec3 rotationalVelocity{0.0f};
            glm::vec3 rotationalAcceleration{0.0f};

            void updateDynamics(float dt, TransformComponent& transform);
    };

    struct CollisionMesh {
        CollisionMesh(float r) {radius = r; }
        float radius = 0;
    };

    class PhysicsObject {
        public:
            PhysicsObject();

            std::shared_ptr<DynamicsComponent> Dynamics;
            std::shared_ptr<CollisionMesh> collisionMesh;

            //TransformComponent& transform;
            std::shared_ptr<GameObject> gameObject;
        private:
            bool isSleep = false; // dont worry about it.
    };
}