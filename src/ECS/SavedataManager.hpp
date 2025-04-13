#pragma once

#include "Components.hpp"
#include "AssetManager.hpp"
#include "../systems/MaterialSystem.hpp"
#include "../../libs/tinyXML/tinyxml2.h"
#include "Importer.hpp"
#include "../Pipeline/deviceManager.hpp"

#include <vector>
#include <unordered_map>
#include <iostream>

// https://shilohjames.wordpress.com/2014/04/27/tinyxml2-tutorial/

namespace ECS {
    class SaveDataManager {
        public:
            SaveDataManager(renderer::Device& device, engine::ScriptingSystem& scriptingSystem, engine::MaterialSystem& materialSystem, engine::SkyboxSystem& skyboxSystem) : device(device), scriptingSystem(scriptingSystem), materialSystem(materialSystem), skyboxSystem{skyboxSystem} {};

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
            void loadData(std::string fileName, AssetSystem& assetSystem) {
		std::cout << "about to load file:	" << fileName.c_str() << "\n";
                tinyxml2::XMLDocument doc;
                doc.LoadFile(fileName.c_str());
		std::cout << "file loaded \n";

                tinyxml2::XMLElement* pRoot = doc.FirstChildElement("Collection");
                tinyxml2::XMLElement* pEntity = pRoot->FirstChildElement("Entity");

                while(pEntity) {
                    Entity entity = assetSystem.CreateEntity();
                    tinyxml2::XMLElement* pComponent = pEntity->FirstChildElement();

                    while(pComponent) {
			std::cout << "adding component \n";
                        const char* componentName = pComponent->Name();

                        //at some point, when I feel like it, I will move these to be virtual functions in the component class.. I'm pretty sure I can do that without slowing down normal operation.
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
                            assetSystem.AddComponent(entity, Importer::loadMesh(pComponent->GetText(), device));

                        } else if (strcmp(componentName, "Script") == 0) {
			    std::cout << "adding Script \n";
                            assetSystem.AddComponent(entity, Script{pComponent->GetText(), scriptingSystem.assembly, scriptingSystem.appDomain });
			    std::cout << "script added \n";

                        } else if (strcmp(componentName, "Material") == 0) {
                            const char* albedoPath;
                            pComponent->QueryStringAttribute("albedoPath", &albedoPath);
                            const char* normalPath;
                            pComponent->QueryStringAttribute("normalPath", &normalPath);
                            
                            assetSystem.AddComponent(entity, Importer::loadMaterial(albedoPath, normalPath, device, materialSystem.getSampler(), materialSystem.getMaterialSetLayout()));
                            
                        } else if (strcmp(componentName, "PointLight") == 0) {
                            PointLight pointLight;
                            pComponent->QueryFloatAttribute("R", &pointLight.color.r);
                            pComponent->QueryFloatAttribute("G", &pointLight.color.g);
                            pComponent->QueryFloatAttribute("B", &pointLight.color.b);
                            pComponent->QueryFloatAttribute("intensity", &pointLight.intensity);
                            pComponent->QueryFloatAttribute("radius", &pointLight.radius);
                            
                            assetSystem.AddComponent(entity, pointLight);


                        } else if (strcmp(componentName, "SkyBox") == 0) {
                            std::vector<std::string> tags;
                            tags.push_back(pComponent->Attribute("Right"));
                            tags.push_back(pComponent->Attribute("Left"));
                            tags.push_back(pComponent->Attribute("Up"));
                            tags.push_back(pComponent->Attribute("Down"));
                            tags.push_back(pComponent->Attribute("Front"));
                            tags.push_back(pComponent->Attribute("Back"));

                            SkyBox skybox = Importer::loadSkyBox(pComponent->GetText(), tags, device, skyboxSystem.getSampler(), skyboxSystem.getSetLayout());
                            assetSystem.AddComponent(entity, skybox);
                        }
                        else {
                            std::cout << "Component not found" << std::endl;
                        }

                        pComponent = pComponent->NextSiblingElement();
                    }
                    pEntity = pEntity->NextSiblingElement();
                }
            }
        private:
            renderer::Device& device;
            engine::ScriptingSystem& scriptingSystem;
            engine::MaterialSystem& materialSystem;
            engine::SkyboxSystem& skyboxSystem;
    };
}
