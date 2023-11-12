#include "PxSimulation.hpp"

namespace engine {
    void PhysicsSimulation::update(float timeStep) {
        for(auto object : objects) {
            if(object.Dynamics != nullptr) {
                object.Dynamics->updateDynamics(timeStep, object.gameObject->transform);
            }
        }
    };
}