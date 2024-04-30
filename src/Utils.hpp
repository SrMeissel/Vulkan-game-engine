#pragma once

#include <functional>

#define TINYOBJECTLOADER_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>

namespace engine {
    //whatever teh fuck this is
    
    // from: https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    };

    //global structs

    struct Vertex{
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};

        bool operator ==(const Vertex& other) const {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }
    };
}

namespace std {
    template<>
    struct hash<engine::Vertex> {
        size_t operator()(engine::Vertex const& vertex) const {
            size_t seed = 0;
            engine::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed; 
        }
    };
}
