#pragma once

#include "Components.hpp"
#include "ComponentTypes.hpp"

#include "EntityManager.hpp"
#include <typeindex>
#include <unordered_map>

namespace ECS {
    class ComplementManager {
        public:
            ComplementManager() {
                complementBuilder.resize(MAX_COMPONENTS);
            }

            template<typename Cmpl>
            void registerComplement(std::function<void*(Entity)> toComplement, ComponentType type) {
                complementBuilder[type] = toComplement;
                complementFinder.insert({typeid(Cmpl), type});
            }

            void* getComplement(Entity entity, ComponentType type) {
                return complementBuilder[type](entity);
            }

            ComponentType findComplement(std::type_index type) {
                return complementFinder[type];
            }

            std::vector<std::function<void*(Entity)>> complementBuilder;

            //edge case, editor need specific component, Ex. Transform.
            std::unordered_map<std::type_index, ComponentType> complementFinder;

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