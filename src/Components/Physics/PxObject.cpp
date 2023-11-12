#include "PxObject.hpp"


#include <iostream>

namespace engine {
    void DynamicsComponent::updateDynamics(float dt, TransformComponent& transform) {
        translationalVelocity += translationalAcceleration * dt;
        rotationalVelocity += rotationalAcceleration * dt;

        transform.translation += translationalVelocity * dt;
        transform.rotation += rotationalVelocity * dt;
    }

    PhysicsObject::PhysicsObject() {
        Dynamics = std::make_shared<DynamicsComponent>();
        collisionMesh = std::make_shared<CollisionMesh>(1.0f);
    }
}