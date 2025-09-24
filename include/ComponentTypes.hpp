#pragma once

#include <glm/glm.hpp>
#include <string>

using Entity = uint64_t;

struct TransformComponent {
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale;

    std::string name;
};

TransformComponent getTransformComponent(Entity);
void setTransformComponent(Entity, const TransformComponent);