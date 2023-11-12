#pragma once

#include "PxObject.hpp"

#include <vector>

namespace engine {
    class PhysicsSimulation {
        public:
            void update(float timeStep);

            void addObject(PhysicsObject object) {objects.push_back(object); }
        private:

        std::vector<PhysicsObject> objects;

    };
}