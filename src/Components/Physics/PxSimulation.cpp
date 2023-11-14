#include "PxSimulation.hpp"

#include <iostream>

namespace engine {
    void PhysicsSimulation::update(float timeStep) {
        for(auto object : objects) {
            if(object.Dynamics != nullptr) {
                object.Dynamics->updateDynamics(timeStep, object);
            }
        }
    };

    bool PhysicsSimulation::sphereRayCollision(Ray ray, CollisionMesh mesh) {
        glm::vec3 direction = (mesh.position - ray.posiiton);
        float projection = glm::dot(ray.direction, direction);
        if(projection < 0.0f ) {
            std::cout << "Tiss behind you! \n";
            return false;
        }
        glm::vec3 closestPoint = ray.posiiton + (ray.direction * projection);
        //yay, I have now found the point along the ray that is closest to the origin of the sphere!
        float distance = glm::distance(closestPoint, mesh.position);
        if(distance < mesh.radius) {
            std::cout << "COLIISION DETECTED!!!!! \n";
            return true;
        } else {
            std::cout << "DID NOT COLLIDE! \n";
            return false;
        }
    };
}