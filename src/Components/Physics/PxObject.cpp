#include "PxObject.hpp"


#include <iostream>

namespace engine {
    void DynamicsComponent::updateDynamics(float dt, PhysicsObject& object) {
        translationalVelocity += translationalAcceleration * dt;
        rotationalVelocity += rotationalAcceleration * dt;

        object.transformComponent.translation += translationalVelocity * dt;
        object.transformComponent.rotation += rotationalVelocity * dt;
    }

    PhysicsObject::PhysicsObject(TransformComponent& transform, unsigned int iD) : transformComponent{transform} {
        Dynamics = std::make_shared<DynamicsComponent>();
        collisionMesh = std::make_shared<CollisionMesh>(1.0f, transform);
        gameObjectID = iD;
        
    }
}