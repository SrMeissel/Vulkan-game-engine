#pragma once

#include "bufferManager.hpp"
#include "Utils.hpp"
#include "../../libs/tinyXML/tinyxml2.h"

#include <memory>
#include <glm/glm.hpp>
#include <iostream>
#include <string>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace ECS {

    struct Component {
        virtual ~Component() = default;
        virtual tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) = 0; // <======
    };

    struct Renderable : public Component {
        std::string Path;

        std::shared_ptr<engine::Buffer> vertexBuffer;
        uint32_t vertexCount;

        bool hasIndexBuffer = false;
        std::shared_ptr<engine::Buffer> indexBuffer;
        uint32_t indexCount;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* renderable = doc.NewElement("Renderable");
            renderable->SetText(Path.c_str());
            return renderable;
        }
    };

    struct Material : public Component {
        const char* albedoPath;
        engine::AllocatedImage albedo;
        const char* normalPath;
        engine::AllocatedImage normal;

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* material = doc.NewElement("Material");
            material->SetAttribute("albedoPath", albedoPath);
            material->SetAttribute("normalPath", normalPath);
            return material;
        }
    };

    struct Transform : public Component {
        Transform() = default;
        Transform(glm::vec3 translation, glm::vec3 scale, glm::vec3 rotation)
            : translation(translation), scale(scale), rotation(rotation) {}

        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{};

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        // https://www.youtube.com/watch?v=0X_kRtyVzm4&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=14&t=289s
        glm::mat4 mat4() {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);

            return glm::mat4{
                {
                    scale.x * (c1 * c3 + s1 * s2 * s3),
                    scale.x * (c2 * s3),
                    scale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    scale.y * (c3 * s1 * s2 - c1 * s3),
                    scale.y * (c2 * c3),
                    scale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    scale.z * (c2 * s1),
                    scale.z * (-s2),
                    scale.z * (c1 * c2), // <=== position 3,3
                    0.0f,
                },
                {translation.x, translation.y, translation.z, 1.0f}};
        }
        glm::mat3 normalMatrix() {
            const float c3 = glm::cos(rotation.z);
            const float s3 = glm::sin(rotation.z);
            const float c2 = glm::cos(rotation.x);
            const float s2 = glm::sin(rotation.x);
            const float c1 = glm::cos(rotation.y);
            const float s1 = glm::sin(rotation.y);
            const glm::vec3 invScale = 1.0f / scale;

            return glm::mat3{
                {
                    invScale.x * (c1 * c3 + s1 * s2 * s3),
                    invScale.x * (c2 * s3),
                    invScale.x * (c1 * s2 * s3 - c3 * s1),
                },
                {
                    invScale.y * (c3 * s1 * s2 - c1 * s3),
                    invScale.y * (c2 * c3),
                    invScale.y * (c1 * c3 * s2 + s1 * s3),
                },
                {
                    invScale.z * (c2 * s1),
                    invScale.z * (-s2),
                    invScale.z * (c1 * c2),
                }
            };
        }

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* transform = doc.NewElement("Transform");
            
            tinyxml2::XMLElement* translation = doc.NewElement("Translation");
            translation->SetAttribute("x", this->translation.x);
            translation->SetAttribute("y", this->translation.y);
            translation->SetAttribute("z", this->translation.z);
            transform->InsertEndChild(translation);

            tinyxml2::XMLElement* rotation = doc.NewElement("Rotation");
            rotation->SetAttribute("x", this->rotation.x);
            rotation->SetAttribute("y", this->rotation.y);
            rotation->SetAttribute("z", this->rotation.z);
            transform->InsertEndChild(rotation);

            tinyxml2::XMLElement* scale = doc.NewElement("Scale");
            scale->SetAttribute("x", this->scale.x);
            scale->SetAttribute("y", this->scale.y);
            scale->SetAttribute("z", this->scale.z);
            transform->InsertEndChild(scale);

            return transform;
        }
    };

    struct Script : public Component {
        const char* className;
        MonoClass* scriptClass;
        MonoObject* scriptObject;
        MonoClass* objectClass;

        Script() {
            scriptClass = nullptr;
            scriptObject = nullptr;
            objectClass = nullptr;
        }

        // I might want to find a way to make the assembly and domain accessible differently.
        Script(const char* name, MonoAssembly* assembly, MonoDomain* appDomain) {
            className = name;
            MonoImage* image = mono_assembly_get_image(assembly);
            scriptClass = mono_class_from_name(image, "", name);
            if(scriptClass == nullptr) std::cout << "Failed to get class!" << std::endl;

            scriptObject = mono_object_new(appDomain, scriptClass);
            if(scriptObject == nullptr) std::cout << "Failed to create object!" << std::endl;

            mono_runtime_object_init(scriptObject); // constructor

            objectClass = mono_object_get_class(scriptObject);
        }

        tinyxml2::XMLElement* save(tinyxml2::XMLDocument& doc) override {
            tinyxml2::XMLElement* script = doc.NewElement("Script");
            script->SetText(className);
            return script;
        }

    };

}