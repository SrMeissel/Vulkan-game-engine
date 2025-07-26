#pragma once

#include <vector>
#include <array>
#include <queue>
#include <bitset>
#include <cassert>
#include <unordered_map>

namespace ECS {

    using Entity = std::uint64_t;
    static const Entity MAX_ENTITIES = 5000;
    using ComponentType = std::uint8_t;
    const ComponentType MAX_COMPONENTS = 32;
    using Signature = std::bitset<MAX_COMPONENTS>;
        
    class EntityManager {
    public:
        void CreateEntity() {
            activeEntities++;
        }
        void DestroyEntity(Entity entity) {
            signatures[entity].reset();
            activeEntities--;
        }
        void SetSignature(Entity entity, Signature signature) {
            signatures[entity] = signature;
        }
        Signature GetSignature(Entity entity) {
            return signatures[entity];
        }

    private:
        std::unordered_map<Entity, Signature> signatures{};
        int activeEntities{0};
    };
}