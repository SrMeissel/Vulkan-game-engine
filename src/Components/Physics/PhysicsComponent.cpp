#include "PhysicsComponent.hpp"


#include <iostream>

namespace engine {
    PhysicsComponent::PhysicsComponent(TransformComponent& transform) : transformer{transform} {

    }

    void PhysicsComponent::updateDynamics(float dt, TransformComponent& transform) {
        translationalVelocity += translationalAcceleration * dt;
        rotationalVelocity += rotationalAcceleration * dt;

        transform.translation += translationalVelocity * dt;
        transform.rotation += rotationalVelocity * dt;
    }
}