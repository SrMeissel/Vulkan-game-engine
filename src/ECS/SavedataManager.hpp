#pragma once

#include "Components.hpp"
#include "AssetManager.hpp"
#include "../../libs/tinyXML/tinyxml2.h"
#include "../Importer.hpp"
#include "../Pipeline/deviceManager.hpp"

#include <vector>
#include <unordered_map>
#include <iostream>

// https://shilohjames.wordpress.com/2014/04/27/tinyxml2-tutorial/

namespace ECS {
    class SaveDataManager {
        public:
            SaveDataManager(engine::Device& device, engine::ScriptingSystem& scriptingSystem) : device(device), scriptingSystem(scriptingSystem) {};

            void saveData(const char* fileName, std::unordered_map<Entity, std::vector<Component*>>& savedComponents){
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
            void loadData(const char* fileName, AssetSystem& assetSystem) {
                tinyxml2::XMLDocument doc;
                doc.LoadFile(fileName);

                tinyxml2::XMLElement* pRoot = doc.FirstChildElement("Collection");
                tinyxml2::XMLElement* pEntity = pRoot->FirstChildElement("Entity");

                while(pEntity) {
                    Entity entity = assetSystem.CreateEntity();
                    tinyxml2::XMLElement* pComponent = pEntity->FirstChildElement();

                    while(pComponent) {
                        const char* componentName = pComponent->Name();

                        if (strcmp(componentName, "Transform") == 0) {
                            Transform transform;

                            tinyxml2::XMLElement* pTranslation = pComponent->FirstChildElement("Translation");
                            pTranslation->QueryFloatAttribute("x", &transform.translation.x);
                            pTranslation->QueryFloatAttribute("y", &transform.translation.y);
                            pTranslation->QueryFloatAttribute("z", &transform.translation.z);

                            tinyxml2::XMLElement* pRotation = pComponent->FirstChildElement("Rotation");
                            pRotation->QueryFloatAttribute("x", &transform.rotation.x);
                            pRotation->QueryFloatAttribute("y", &transform.rotation.y);
                            pRotation->QueryFloatAttribute("z", &transform.rotation.z);

                            tinyxml2::XMLElement* pScale = pComponent->FirstChildElement("Scale");
                            pScale->QueryFloatAttribute("x", &transform.scale.x);
                            pScale->QueryFloatAttribute("y", &transform.scale.y);
                            pScale->QueryFloatAttribute("z", &transform.scale.z);

                            assetSystem.AddComponent(entity, transform);
                            
                        } else if (strcmp(componentName, "Renderable") == 0) {
                            assetSystem.AddComponent(entity, Importer::loadOBJmodel(pComponent->GetText(), device));
                        } else if (strcmp(componentName, "Script") == 0) {
                            assetSystem.AddComponent(entity, Script{pComponent->GetText(), scriptingSystem.assembly, scriptingSystem.appDomain });
                        } else {
                            std::cout << "Component not found" << std::endl;
                        }

                        pComponent = pComponent->NextSiblingElement();
                    }
                    pEntity = pEntity->NextSiblingElement();
                }
            }
        private:
            engine::Device& device;
            engine::ScriptingSystem& scriptingSystem;
    };
}