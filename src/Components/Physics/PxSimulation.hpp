#pragma once

#include "PxObject.hpp"

#include <vector>

namespace engine {
    class PhysicsSimulation {
        public:
            void update(float timeStep);

            bool sphereRayCollision(Ray ray, CollisionMesh mesh);

            //void addObject(PhysicsObject& object) {objects.push_back(std::move(object)); }
            std::vector<PhysicsObject> objects;
        private:

    };
}