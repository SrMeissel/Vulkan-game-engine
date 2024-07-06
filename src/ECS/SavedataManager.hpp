#pragma once

#include "Components.hpp"

#include "../../libs/tinyXML/tinyxml2.h"

#include <vector>
#include <unordered_map>

namespace ECS {
    class SaveDataManager {
        public:
            void saveData(const char* fileName){
                tinyxml2::XMLDocument doc;
                tinyxml2::XMLNode* pRoot = doc.NewElement("Collection");
                doc.InsertFirstChild(pRoot);

                for(auto& pair : savedComponents) {
                tinyxml2::XMLElement* pElement = doc.NewElement("Entity");

                    for(auto& component : pair.second) {
                        tinyxml2::XMLElement* pComponent = component->save(doc);
                        pElement->InsertEndChild(pComponent);
                    }
                    pRoot->InsertEndChild(pElement);
                }

                doc.SaveFile(fileName);
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