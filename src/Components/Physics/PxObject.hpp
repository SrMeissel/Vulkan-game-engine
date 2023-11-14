#pragma once

//#include "collisionSolver.hpp"
#include "../../gameObject.hpp"

#include <glm/glm.hpp>
#include <memory>

namespace engine {
    class GameObject;
    struct TransformComponent;

    struct DynamicsComponent {
            float mass = 1;
            glm::vec3 COM;

            glm::vec3 translationalVelocity{0.0f};
            glm::vec3 translationalAcceleration{0.0f};
            
            glm::vec3 rotationalVelocity{0.0f};
            glm::vec3 rotationalAcceleration{0.0f};

            void updateDynamics(float dt, PhysicsObject& object);
    };

    struct CollisionMesh {
        CollisionMesh(float r, TransformComponent& transform) {
            radius = r;
            position = transform.translation;
        }
        float radius = 0;
        glm::vec3 position{0.0f};
    };

    struct Ray {
        glm::vec3 posiiton;
        glm::vec3 direction;
    };

    class PhysicsObject {
        public:
            PhysicsObject(TransformComponent& transform, unsigned int iD);

            std::shared_ptr<DynamicsComponent> Dynamics;
            std::shared_ptr<CollisionMesh> collisionMesh;

            TransformComponent& transformComponent;
            unsigned int gameObjectID;

        private:
            bool isSleep = false; // dont worry about it.
    };
}