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
#include <chrono>
#include <random>

// https://shilohjames.wordpress.com/2014/04/27/tinyxml2-tutorial/

namespace ECS {

    // struct EntityID {
    //     Entity entity;
    //     uint64_t id;
    // };

    class SaveDataManager {
        public:
            SaveDataManager(renderer::Renderer& renderer, engine::ScriptingSystem& scriptingSystem, engine::MaterialSystem& materialSystem, engine::SkyboxSystem& skyboxSystem) : renderer(renderer), scriptingSystem(scriptingSystem), materialSystem(materialSystem), skyboxSystem{skyboxSystem} {};

            void saveData(const char* fileName, std::unordered_map<Entity, std::vector<Component*>>& savedComponents){
                tinyxml2::XMLDocument doc;
                tinyxml2::XMLNode* pRoot = doc.NewElement("Collection");
                doc.InsertFirstChild(pRoot);

                for(auto& pair : savedComponents) {

                    //look for existing ID
                    if(uniqueIds.find(pair.first) == uniqueIds.end()) {     
                        uniqueIds.insert({pair.first, generateEntityID()});
                    }

                    tinyxml2::XMLElement* pElement = doc.NewElement("Entity");
                    //pElement->SetAttribute("ID", uniqueIds.find(pair.first)->second); //add ID to save file

                    for(auto& component : pair.second) {
                        tinyxml2::XMLElement* pComponent = component->save(doc);
                        pElement->InsertEndChild(pComponent);
                    }
                    pRoot->InsertEndChild(pElement);
                }

                doc.SaveFile(((std::string)SOURCE_PATH + fileName).c_str());
            }
            void loadData(std::string fileName, AssetSystem& assetSystem) {
                tinyxml2::XMLDocument doc;

                //FILE* file = fopen(((std::string)SOURCE_PATH + fileName).c_str(), "rb");
                FILE* file = fopen(fileName.c_str(), "rb");
                if (file == NULL) {
                    std::cout << "file bad \n";
                    throw std::runtime_error("file bad");
                }

                if(doc.LoadFile(file) != 0) {
                    throw std::runtime_error("load sucked");
                }

                tinyxml2::XMLElement* pRoot = doc.FirstChildElement("Collection");
                tinyxml2::XMLElement* pEntity = pRoot->FirstChildElement("Entity");
                while(pEntity) {
                    Entity entity = assetSystem.CreateEntity();
                    // tinyxml2::XMLError result = pEntity->QueryUnsigned64Attribute("ID", &id);
                    // if (result == tinyxml2::XML_SUCCESS) {
                    //     uniqueIds.insert({entity, id});
                    // } else {
                    //     uniqueIds.insert({entity, generateEntityID()});
                    // }
                    std::cout << "new Entity being loaded \n";

                    tinyxml2::XMLElement* pComponent = pEntity->FirstChildElement();

                    while(pComponent) {
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

                            uint64_t id;
                            tinyxml2::XMLError result = pComponent->QueryUnsigned64Attribute("ID", &id);
                            if (result == tinyxml2::XML_SUCCESS) {
                                transform.uniqueID = id;
                            } else {
                                std::cout << "entity does not have a unique ID, yikes \n";
                                transform.uniqueID = id;
                            }

                            assetSystem.AddComponent(entity, transform);
                            
                        } else if (strcmp(componentName, "Renderable") == 0) {
                            assetSystem.AddComponent(entity, Importer::loadMesh(pComponent->GetText(), renderer.device));

                        } else if (strcmp(componentName, "Script") == 0) {
                            assetSystem.AddComponent(entity, Script{pComponent->GetText(), scriptingSystem.assembly, scriptingSystem.appDomain });

                        } else if (strcmp(componentName, "Material") == 0) {
                            std::cout << "creating Material \n";
                            const char* albedoPath;
                            pComponent->QueryStringAttribute("albedoPath", &albedoPath);
                            const char* normalPath;
                            pComponent->QueryStringAttribute("normalPath", &normalPath);
                            
                            assetSystem.AddComponent(entity, Importer::loadMaterial(albedoPath, normalPath, renderer, materialSystem.getMaterialSetLayout()));
                            std::cout << "material Made \n";
                            
                        } else if (strcmp(componentName, "PointLight") == 0) {
                            std::cout << "creating Poinlight \n";
                            PointLight pointLight;
                            pComponent->QueryFloatAttribute("R", &pointLight.color.r);
                            pComponent->QueryFloatAttribute("G", &pointLight.color.g);
                            pComponent->QueryFloatAttribute("B", &pointLight.color.b);
                            pComponent->QueryFloatAttribute("intensity", &pointLight.intensity);
                            pComponent->QueryFloatAttribute("radius", &pointLight.radius);
                            
                            assetSystem.AddComponent(entity, pointLight);
                            std::cout << "Pointlight Made \n";

                        } else if (strcmp(componentName, "SkyBox") == 0) {
                            std::cout << "creating SkyBox \n";
                            std::vector<std::string> tags;
                            tags.push_back(pComponent->Attribute("Right"));
                            tags.push_back(pComponent->Attribute("Left"));
                            tags.push_back(pComponent->Attribute("Up"));
                            tags.push_back(pComponent->Attribute("Down"));
                            tags.push_back(pComponent->Attribute("Front"));
                            tags.push_back(pComponent->Attribute("Back"));

                            SkyBox skybox = Importer::loadSkyBox(pComponent->GetText(), tags, renderer, skyboxSystem.getSetLayout());
                            assetSystem.AddComponent(entity, skybox);
                            std::cout << "SkyBox made \n";
                        }
                        else {
                            std::cout << "Component not found" << "\n";
                        }

                        pComponent = pComponent->NextSiblingElement();
                    }
                    pEntity = pEntity->NextSiblingElement();
                }

                if (fclose(file) != 0) {
                    throw std::runtime_error("close bad");
                }
            }

            //TODO: right now Im going to make the unload EVERYTHING, but I want to unload specific entities from a file
            void unloadData(std::vector<Entity> entities, AssetSystem& assetSystem) {
                for(Entity thing : entities) {
                    assetSystem.DestroyEntity(thing);
                }
            }

                    std::unordered_map<Entity, uint32_t> uniqueIds;
        private:
            renderer::Renderer& renderer;
            engine::ScriptingSystem& scriptingSystem;
            engine::MaterialSystem& materialSystem;
            engine::SkyboxSystem& skyboxSystem;
    };
}
