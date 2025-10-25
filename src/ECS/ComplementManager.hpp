#pragma once

#include "Components.hpp"
#include "ComponentTypes.hpp"

#include "EntityManager.hpp"
#include <typeindex>
#include <unordered_map>

namespace ECS {
    class ComplementManager {
        public:

            template<typename Cmpl>
            void registerComplement(std::function<void*(Entity)> toComplement, ComponentType type) {
                complementBuilder.insert({typeid(Cmpl),toComplement});
                typeRegistry[type] = typeid(Cmpl);
            }

            template<typename Cmpl>
            void* getComplement(Entity entity){
                assert(complementBuilder.count(typeid(Cmpl)) != 0 && "Trying to get a complement that is not registered :)");
                return complementBuilder[typeid(Cmpl)](entity);
            }

            void* getComplement(Entity entity, ComponentType type) {
                return complementBuilder[typeRegistry[type]](entity);
            }

            std::unordered_map<std::type_index, std::function<void*(Entity)>> complementBuilder;
            std::vector<std::type_index> typeRegistry;

            /*
            toComplement Example:
                void(void* data, Entity entity)[assetSystem] {
                    ECS::Transform transform = assetSystem->getComponent<ECS::Transform>(Entity);
                    TransformComplement complement = transform;
                    data = &complement;
                }
            */

    };
}