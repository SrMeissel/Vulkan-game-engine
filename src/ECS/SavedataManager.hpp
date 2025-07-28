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

            //TODO: make sure this saves entities with their ID
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
                doc.SaveFile(((std::string)SOURCE_PATH + fileName).c_str());
            }
            void loadData(std::string fileName, AssetSystem& assetSystem) {
                tinyxml2::XMLDocument doc;
                openXML(fileName, doc);

                tinyxml2::XMLElement* pRoot = doc.FirstChildElement("Collection");
                tinyxml2::XMLElement* pEntity = pRoot->FirstChildElement("Entity");
                while(pEntity) {
                    Entity entity = 0;
                    pEntity->QueryUnsigned64Attribute("ID", &entity);
                    entity = assetSystem.CreateEntity(entity); //ensures entity is valid.

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

                            assetSystem.AddComponent(entity, transform);
                            
                        } else if (strcmp(componentName, "Renderable") == 0) {
                            assetSystem.AddComponent(entity, Importer::loadMesh(pComponent->GetText(), renderer.device));

                        } else if (strcmp(componentName, "Script") == 0) {
                            assetSystem.AddComponent(entity, Script{pComponent->GetText(), scriptingSystem.assembly, scriptingSystem.appDomain });

                        } else if (strcmp(componentName, "Material") == 0) {
                            std::cout << "Creating Material... ";
                            const char* albedoPath;
                            pComponent->QueryStringAttribute("albedoPath", &albedoPath);
                            const char* normalPath;
                            pComponent->QueryStringAttribute("normalPath", &normalPath);
                            
                            assetSystem.AddComponent(entity, Importer::loadMaterial(albedoPath, normalPath, renderer, materialSystem.getMaterialSetLayout()));
                            std::cout << "Material Made \n";
                            
                        } else if (strcmp(componentName, "PointLight") == 0) {
                            std::cout << "Creating Pointlight... ";
                            PointLight pointLight;
                            pComponent->QueryFloatAttribute("R", &pointLight.color.r);
                            pComponent->QueryFloatAttribute("G", &pointLight.color.g);
                            pComponent->QueryFloatAttribute("B", &pointLight.color.b);
                            pComponent->QueryFloatAttribute("intensity", &pointLight.intensity);
                            pComponent->QueryFloatAttribute("radius", &pointLight.radius);
                            
                            assetSystem.AddComponent(entity, pointLight);
                            std::cout << "Pointlight Made \n";

                        } else if (strcmp(componentName, "SkyBox") == 0) {
                            std::cout << "Creating SkyBox... ";
                            std::vector<std::string> tags;
                            tags.push_back(pComponent->Attribute("Right"));
                            tags.push_back(pComponent->Attribute("Left"));
                            tags.push_back(pComponent->Attribute("Up"));
                            tags.push_back(pComponent->Attribute("Down"));
                            tags.push_back(pComponent->Attribute("Front"));
                            tags.push_back(pComponent->Attribute("Back"));

                            SkyBox skybox = Importer::loadSkyBox(pComponent->GetText(), tags, renderer, skyboxSystem.getSetLayout());
                            assetSystem.AddComponent(entity, skybox);
                            std::cout << "SkyBox Made \n";
                        }
                        else {
                            std::cout << "Component not found: " << componentName << "\n";
                        }
                        pComponent = pComponent->NextSiblingElement();
                    }
                    pEntity = pEntity->NextSiblingElement();
                }
                doc.Clear();
            }

            void unloadData(std::string fileName, AssetSystem& assetSystem) {
                if(fileName.empty()) return;
                tinyxml2::XMLDocument doc;
                openXML(fileName, doc);

                tinyxml2::XMLElement* pRoot = doc.FirstChildElement("Collection");
                tinyxml2::XMLElement* pEntity = pRoot->FirstChildElement("Entity");
                while(pEntity) {
                    Entity entity = 0;
                    pEntity->QueryUnsigned64Attribute("ID", &entity);
                    assetSystem.DestroyEntity(entity);
                    pEntity = pEntity->NextSiblingElement();
                }
            }
            
        private:
            void openXML(std::string fileName,  tinyxml2::XMLDocument& doc) {
                FILE* file = fopen(fileName.c_str(), "rb");
                if (file == NULL) {
                    std::cout << "file bad \n";
                    throw std::runtime_error("file bad");
                }
                if(doc.LoadFile(file) != 0) {
                    std::cout << "load sucked \n";
                    throw std::runtime_error("load sucked");
                }
                if (fclose(file) != 0) {
                    std::cout << "close bad \n";
                    throw std::runtime_error("close bad");
                }
            }

            renderer::Renderer& renderer;
            engine::ScriptingSystem& scriptingSystem;
            engine::MaterialSystem& materialSystem;
            engine::SkyboxSystem& skyboxSystem;
    };
}
