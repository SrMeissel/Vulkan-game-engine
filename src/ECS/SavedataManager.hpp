#pragma once

#include "AssetManager.hpp"
#include "Components.hpp"

#include <vector>
#include <unordered_map>

namespace ECS {
    class SaveDataManager {
        public:
            void saveData(){
                for(auto& pair : savedComponents) {
                    for(auto& component : pair.second) {
                        component->save();
                    }
                }
            }
            void loadData();

            // ===========================================

            void entityCreated(Entity entity) {
                std::vector<Component*> components;
                savedComponents.insert({entity, components});
            }

            void componentCreated(Entity entity, Component* component) {
                savedComponents[entity].push_back(component);
            }

            void entityDestroyed(Entity entity) {
                savedComponents.erase(entity);
            }

            void componentDestroyed(Entity entity, Component* component) {
                savedComponents[entity].erase(std::remove(savedComponents[entity].begin(), savedComponents[entity].end(), component), savedComponents[entity].end());
            }

        private:
            std::unordered_map<Entity, std::vector<Component*>> savedComponents;

    };
}